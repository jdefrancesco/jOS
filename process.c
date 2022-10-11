#include <stddef.h>

#include "process.h"
#include "trap.h"
#include "memory.h"
#include "print.h"
#include "klib.h"
#include "debug.h"

extern struct tss_t tss; 
static struct process_t process_table[NUM_PROC];
static int pid_num = 1;
void main(void);

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

static void set_process_entry(struct process_t *proc)
{
    uint64_t stack_top;

    proc->state = PROC_INIT;
    proc->pid = pid_num++;

    // Allocate kernel stack
    proc->stack = (uint64_t)kalloc();
    ASSERT(proc->stack != 0);

    memset((void*)proc->stack, 0, PAGE_SIZE);   
    stack_top = proc->stack + STACK_SIZE; // Top of stack, grows downward.

    // Get base address of trap_frame
    proc->tf = (struct trap_frame_t*)(stack_top - sizeof(struct trap_frame_t)); 
    
    proc->tf->cs = 0x10|3;
    proc->tf->rip = 0x400000;
    proc->tf->ss = 0x18|3;
    proc->tf->rsp = 0x400000 + PAGE_SIZE;
    proc->tf->rflags = 0x202;
    
    // New kvm for this process.
    proc->page_map = setup_kvm();
    ASSERT(proc->page_map != 0);
    ASSERT(setup_uvm(proc->page_map, (uint64_t)main, PAGE_SIZE));
}

void init_process(void)
{  
    struct process_t *proc = find_unused_process();
    ASSERT(proc == &process_table[0]);

    set_process_entry(proc);
}

void launch(void)
{
    set_tss(&process_table[0]);
    switch_vm(process_table[0].page_map);
    // Go to ring3
    pstart(process_table[0].tf);
}

// A simple test programm to run in ring3
void main(void)
{
    char *p = (char*)0xffff800000200020;
    *p = 1;
}