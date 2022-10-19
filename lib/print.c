
// print.c is part of the jOS kernel project.
//
// This file implements basic string formatting and conversion tasks.
// The code is a bit sloppy, I will make it more elegant as I progress.
//
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>


// the row, col = 0, 0
// NOTE: AFTER we turn on memory manager, we need to use the Virtual Address of 0xb8000

#define DEC_DIGITS "0123456789"
#define HEX_DIGITS "0123456789ABCDEF"


#define LINE_SIZE 160
#define MAX_BUFF_SIZE 1024
#define DIGITS_BUFF_SIZE 25

extern int writeu(char *buffer, size_t buffer_size);

// Unsigned decimal to string formatting function.
static size_t udec_to_string(char *buffer, size_t pos, uint64_t digits)
{
    // const char * const kDecDigits = DEC_DIGITS;
    char digits_map[10] = DEC_DIGITS;
    char digits_buffer[DIGITS_BUFF_SIZE] = {0};
    int sz = 0;

    do {
        digits_buffer[sz++] = digits_map[digits % 10];
        digits /= 10;
    } while ((digits != 0) && (sz < DIGITS_BUFF_SIZE));

    // Reverse, digits would have been backwards
    for(int i = (sz - 1); i >= 0; i--) {
        buffer[pos++] = digits_buffer[i];
    }

    return sz;
}


static size_t dec_to_string(char *buffer, size_t pos, uint64_t digits)
{
    size_t sz = 0;

    if (digits < 0) {
        digits = -digits;
        buffer[pos++] = '-';
        sz = 1;
    }

    sz += udec_to_string(buffer, pos, (uint64_t)digits);
    return sz;
}


/// read_string will read a string into a buffer and return number
//  of characters read into buffer.
static size_t read_string(char *buffer, size_t pos, const char *str)
{
    size_t i = 0;
    for ( ; str[i] != '\0'; i++) {
        buffer[pos++] = str[i];
    }

    return i;
}

// Dump string from hex.
static int hex_to_string(char *buffer, size_t pos, uint64_t digits)
{
    char digits_buffer[DIGITS_BUFF_SIZE];
    const char * const kHexTable = HEX_DIGITS;
    size_t sz = 0;

    do {
        digits_buffer[sz++] = kHexTable[digits % 16];
        digits /= 16;
    } while ((digits != 0) && (sz < DIGITS_BUFF_SIZE));

    // Reverse, digits would have been backwards
    for(int i = (sz - 1); i >= 0; i--) {
        buffer[pos++] = digits_buffer[i];
    }

    buffer[pos++] = 'h';

    return sz+1;

}

// Kernels printf function. Will add more format specifiers as we move along.
int printf(const char *format, ...)
{

    char buffer[MAX_BUFF_SIZE] = {0};
    size_t buff_size = 0;
    int64_t integer = 0;
    char *string = 0;
    va_list args;

    va_start(args, format);

    for (size_t i = 0; format[i] != '\0'; i++) {
        if (format[i] != '%') {
            buffer[buff_size++] = format[i];
        } else {
            switch (format[++i]) {
                case 'x':
                    integer = va_arg(args, int64_t);
                    buff_size += hex_to_string(buffer, buff_size, (uint64_t)integer);
                    break;

                case 'u':
                    integer = va_arg(args, int64_t);
                    buff_size += udec_to_string(buffer, buff_size, (uint64_t)integer);
                    break;

                case 'd':
                    integer = va_arg(args, int64_t);
                    buff_size += dec_to_string(buffer, buff_size, integer);
                    break;

                case 's':
                    string =  va_arg(args, char *);
                    buff_size += read_string(buffer, buff_size, string);
                    break;

                default:
                    buffer[buff_size++] = '%';
                    i--;
            }
        }
    }

    // 0xf -- print in white
    // write_screen(buffer, buff_size, &screen_buffer, 0xf);
    buff_size = writeu(buffer, buff_size);
    va_end(args);

    return buff_size;

}
