#include "./lib.h"
#include <stdint.h>
#include <stddef.h>

int main(void) 
{
    size_t counter = 0;
    for(;;) {
        if ((counter % 100000)== 0) {
            printf("process1 %d\n", counter);
        }
        counter++;
    }
    return 0;
}