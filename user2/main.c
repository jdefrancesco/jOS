// First console application to begin interacting with the OS.
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "lib.h"
#include "console.h"

#define MAX_BUFF 80

static void 
cmd_get_total_mem(void)
{
    size_t total = 0;
    total = get_total_memoryu();
    printf("Total memory: %dMB\n", total);
}

static int 
parse_cmd(char *buffer, size_t buff_size)
{
    int32_t cmd = -1;

    // TODO: Need to implement string functions!!
    if (buff_size == 8 && (!memcmp("totalmem", buffer, 8))) {
        cmd = 0;
    }

    return cmd;
}

static void 
execute_cmd(int32_t cmd)
{ 
    cmd_func_t cmd_list[1] = {cmd_get_total_mem};
    if (cmd == 0) {       
        cmd_list[0]();
    }
}

static int 
read_cmd(char *buffer)
{
    char ch[2] = { 0 };
    size_t buff_size = 0;

    while (1) {
        ch[0] = keyboard_readu();
        
        if (ch[0] == '\n' || buff_size >= 80) {
            printf("%s", ch);
            break;
        }
        else if (ch[0] == '\b') {    
            if (buff_size > 0) {
                buff_size--;
                printf("%s", ch);    
            }           
        }          
        else {     
            buffer[buff_size++] = ch[0]; 
            printf("%s", ch);        
        }
    }

    return buff_size;
}

int main(void)
{
    char buffer[MAX_BUFF];
    size_t buff_size = 0;
    int32_t cmd = 0;

    static const char * const kShellPrompt ="[jsh ]$";
    
    while(true) {
        printf("%s ", kShellPrompt);
        buff_size = read_cmd(buffer);
        if (buff_size == 0)
            continue;

        cmd = parse_cmd(buffer, buff_size);
        if (cmd < 0) {
            printf("[ERROR] Command not found\n");
        } else {
            execute_cmd(cmd);
        }
    }

    return 0;
}