#include <stddef.h>

#include "klib.h"
#include "process.h"
#include "trap.h"
#include "memory.h"
#include "print.h"
#include "debug.h"

extern struct tss_t tss; 
static struct process_t process_table[NUM_PROC];
static int pid_num = 1;
static struct process_control_t pc;

static void set_tss(struct process_t *proc)
{
    tss.rsp0 = proc->stack + STACK_SIZE;    
}

static struct process_t* find_unused_process(void) 
{
    struct process_t *process = NULL;

    for (int i = 0; i < NUM_PROC; i++) {
        if (process_table[i].state == PROC_UNUSED) {
            process = &process_table[i];
            break;
        }
    }

    return process;
}

static void set_process_entry(struct process_t *proc, uint64_t addr)
{
    uint64_t stack_top;

    proc->state = PROC_INIT;
    proc->pid = pid_num++;

    // Allocate kernel stack
    proc->stack = (uint64_t)kalloc();
    ASSERT(proc->stack != 0);

    memset((void*)proc->stack, 0, PAGE_SIZE);   
    stack_top = proc->stack + STACK_SIZE; // Top of stack, grows downward.

    proc->context = stack_top - sizeof(struct trap_frame_t) - 7*8;
    // Add 48 to RSP
    *(uint64_t *)(proc->context + 6*8) = (uint64_t) trap_return;

    // Get base address of trap_frame
    proc->tf = (struct trap_frame_t*)(stack_top - sizeof(struct trap_frame_t)); 
    
    proc->tf->cs = 0x10|3;
    proc->tf->rip = 0x400000;
    proc->tf->ss = 0x18|3;
    proc->tf->rsp = 0x400000 + PAGE_SIZE;
    // Int flag is set when get to user mode.
    proc->tf->rflags = 0x202;
    
    // New kvm for this process.
    proc->page_map = setup_kvm();
    ASSERT(proc->page_map != 0);
    // Going to load our user.bin main.
    ASSERT(setup_uvm(proc->page_map, P2V(addr), 5120));
    proc->state = PROC_READY;
    
}

static struct process_control_t * get_pc(void)
{
    return &pc;
}



static void switch_process(struct process_t *prev, struct process_t *current) 
{
    
    set_tss(current);
    switch_vm(current->page_map);
    // Will change kernel stack for corresponding process.
    // If we are proc1, after swap we are in proc2
    swap(&prev->context, current->context);
}
static void schedule(void) 
{
    struct process_t *prev_proc;
    struct process_t *current_proc;
    struct process_control_t *process_control;
    struct head_list_t *list;

    process_control = get_pc();
    prev_proc = process_control->curr_process;
    list = &process_control->ready_list;
    ASSERT(!is_list_empty(list));
    
    // process_t is set to running.  
    current_proc = (struct process_t*)remove_list_head(list);
    current_proc->state = PROC_RUNNING;   
    process_control->curr_process = current_proc;

    switch_process(prev_proc, current_proc);   
}

void init_process(void)
{  

    struct process_control_t *process_control;
    struct process_t *process;
    struct head_list_t *list;
    uint64_t addr[2] = {0x20000, 0x30000};
    
    process_control = get_pc();
    list = &process_control->ready_list;
    printk("%x\n", process_control);

    for (size_t i = 0; i < 2; i++) {
        process = find_unused_process();
        set_process_entry(process, addr[i]);
        append_list_tail(list, (struct list_t *)process);
    }
}

void launch(void)
{
    struct process_control_t *process_control;
    struct process_t *process;

    // Set our new process to run.
    process_control = get_pc();
    process = (struct process_t *) remove_list_head(&process_control->ready_list);
    process->state = PROC_RUNNING;
    process_control->curr_process = process;

    set_tss(process);
    switch_vm(process->page_map);
    // Go to ring3
    pstart(process->tf);
}

void yield(void) 
{
    struct process_control_t *proc_control;
    struct process_t *process;
    struct head_list_t *list;

    // get_pc returns address of pc var.
    proc_control = get_pc();
    list = &proc_control->ready_list;

    // If list is empty we have no more procs to run.
    if (is_list_empty(list)) {
        return;
    }

    process = proc_control->curr_process;
    process->state = PROC_READY;
    append_list_tail(list, (struct list_t *)process);
    schedule();
}


void sleep(int wait)
{
    struct process_control_t *process_control;
    struct process_t *process;
    
    process_control = get_pc();
    process = process_control->curr_process;
    process->state = PROC_SLEEP;
    process->wait = wait;

    append_list_tail(&process_control->wait_list, (struct list_t*)process);
    schedule();
}

void wake_up(int wait)
{
    struct process_control_t *process_control;
    struct process_t *process;
    struct head_list_t *ready_list;
    struct head_list_t *wait_list;

    process_control = get_pc();
    ready_list = &process_control->ready_list;
    wait_list = &process_control->wait_list;
    process = (struct process_t*)remove_list(wait_list, wait);

    while (process != NULL) {       
        process->state = PROC_READY;
        append_list_tail(ready_list, (struct list_t*)process);
        process = (struct process_t*)remove_list(wait_list, wait);
    }
}