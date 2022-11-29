#include "./lib.h"
#include <stdint.h>

int main(void) 
{

    printf("process2panic-problem\n");
    // Lets cause a problem
    char *p = (char *)0xffff800000200200;
    *p = 1;
    sleepu(100);

    return 0;
}