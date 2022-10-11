#include <stdint.h>

#include "trap.h"
#include "print.h"
#include "debug.h"
#include "memory.h"

void kmain(void)
{
    // Initialize IDT.
    init_idt();
    // Initialize early stage memory subsystem.
    init_memory();
    // Initialize Virt. Memory.
    init_kvm();

    printk("Welcome to jOS!\n");

    for (;;) { }
}
