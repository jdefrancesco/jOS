#include "./lib.h"

int main(void) 
{
    unsigned int counter = 0;
    for(;;) {
        if ((counter % 100000) == 0) {
            printf("process1 %d\n", counter);
        }
        counter++;
    }
    return 0;
}