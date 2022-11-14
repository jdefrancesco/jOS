#include <stdint.h>
#include <stddef.h>

#include "syscall.h"
#include "print.h"
#include "debug.h"
#include "process.h"

static SYSTEMCALL system_calls[10];

static int
sys_write(int64_t *argptr)
{
    write_screen((char *)argptr[0], (int)argptr[1], 0xe);
    return (int)argptr[1];
}

static int 
sys_sleep(int64_t* argptr)
{
    uint64_t old_ticks; 
    uint64_t ticks;
    uint64_t sleep_ticks = argptr[0];

    ticks = get_ticks();
    old_ticks = ticks;

    while (ticks - old_ticks < sleep_ticks) {
       sleep(-1);
       ticks = get_ticks();
    }
    return 0;
}

void init_system_call(void)
{
    system_calls[0] = sys_write;
}

void system_call(struct trap_frame_t *tf)
{
    int64_t i = tf->rax;
    int64_t param_count = tf->rdi;
    int64_t *argptr = (int64_t *)tf->rsi;

    // rax holds syscall we look up in table.

    if (param_count < 0 ||  i != 0) {
        tf->rax = -1;
        return;
    }

    ASSERT(system_calls[i] != NULL);
    tf->rax = system_calls[i](argptr);
}