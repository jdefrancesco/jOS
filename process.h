#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "trap.h"
#include "klib.h"

struct process_t  {
    struct list_t *next;
    int pid;
    int state;
    int wait;
    uint64_t context;
    // PML4
    uint64_t page_map;
    // Kernel stack
    uint64_t stack;
    struct trap_frame_t *tf;
};

struct process_control_t {
    struct process_t *curr_process;
    struct head_list_t ready_list;
    struct head_list_t wait_list;
    struct head_list_t kill_list;
};

struct tss_t {
    uint32_t res0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
	uint64_t res1;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t res2;
	uint16_t res3;
	uint16_t iopb;
}__attribute__((packed));

#define STACK_SIZE (2*1024*1024)
// For now only can have 10 processes heh.
#define NUM_PROC 10
#define PROC_UNUSED 0
#define PROC_INIT 1
#define PROC_RUNNING 2
#define PROC_READY 3
#define PROC_SLEEP 4
#define PROC_KILLED 5

void init_process(void);
void launch(void);
void pstart(struct trap_frame_t *tf);
void yield(void);
void swap(uint64_t *prev, uint64_t next);
void sleep(int wait);
void wake_up(int wait);
void wait(void);
void exit(void);
#endif