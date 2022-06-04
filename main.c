#include <stdint.h>


#include "trap.h"
#include "print.h"
#include "debug.h"

void kmain(void)
{
    char *boot_string = "jOS - Welcome....";
    int64_t value = 0x123456789ABCD;

    // Initialize IDT
    init_idt();

    ASSERT(0);
}
