#include "./lib.h"

int main(void) 
{
    for(;;) {
        printf("process2\n");
        sleepu(10000);
    }
    return 0;
}