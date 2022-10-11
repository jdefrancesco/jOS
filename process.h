#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "trap.h"

struct process_t  {
    int pid;
    int state;
    // PML4
    uint64_t page_map;
    // Kernel stack
    uint64_t stack;
    struct trap_frame_t *tf;
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

void init_process(void);
void launch(void);
void pstart(struct trap_frame_t *tf);

#endif