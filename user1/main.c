#include "./lib.h"

int main(void) 
{
    int counter = 0;
    for(;;) {
        if ((counter % 1000) == 0) {
            printf("process1 %d\n", counter);
        }
        counter++;
    }
    return 0;
}