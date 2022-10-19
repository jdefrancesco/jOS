#ifndef _PRINT_H
#define _PRINT_H

#include <stddef.h>
#include <stdint.h>

#define LINE_SIZE 160
#define MAX_BUFF_SIZE 1024
#define DIGITS_BUFF_SIZE 25

struct screen_buffer {
    char *buff;
    int column;
    int row;
};

int printk(const char *format, ...);
void write_screen(const char *buffer, size_t size, char color);

#endif
