#include "lib.h"

int main(void)
{

   int64_t counter = 0;

    while (1) {
        if (counter % 100000000 == 0)
            //printf("process3 %d\n",counter);
        counter++;
    }
    return 0;
    
}