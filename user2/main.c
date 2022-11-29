#include "./lib.h"
#include <stdint.h>

int main(void) 
{

    // Lets cause a problem
    char *p = (char *)0xffff800000200200;
    *p = 1;
    printf("process2panic\n");
    sleepu(100);

    return 0;
}