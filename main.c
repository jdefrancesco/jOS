#include <stdint.h>

#include "trap.h"
#include "print.h"
#include "debug.h"
#include "memory.h"
#include "process.h"
#include "syscall.h"

void kmain(void)
{
    // Initialize IDT.
    init_idt();
    // Initialize early stage memory subsystem.
    init_memory();
    // Initialize Virt. Memory.
    init_kvm();
    // Initialize our system call interface.
    init_system_call();
    init_process();
    launch();
}
