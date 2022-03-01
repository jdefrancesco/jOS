#include <stdint.h>
#include <stdlib.h>

#include "trap.h"
#include "print.h"

void kmain(void)
{
    char *boot_string = "jOS - Welcome....";
    int64_t value = 0x123456789ABCD;

    // Initialize IDT
    init_idt();

    printk("%s\n", boot_string);
    printk("test value = %x", value);
}
