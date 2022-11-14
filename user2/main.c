#include "./lib.h"
#include <stdint.h>
#include <stddef.h>

int main(void) 
{
    size_t counter = 0;
    for(;;) {
        printf("process2\n");
        sleepu(100);
    }
    return 0;
}