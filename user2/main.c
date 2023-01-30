#include "lib.h"

int main(void)
{
 
    char ch[2] = {0};
    while (1) {
        ch[0] = keyboard_readu();
        printf("%s", ch);
    }
    return 0;
}