#include "debug.h"
#include "print.h"

void error_check(char *file, uint64_t line) 
{
    printk("[FATAL] Assertion Failed [%s:%u]", file, line);
    for(;;) {}
}