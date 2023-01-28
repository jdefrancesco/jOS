#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>

struct keyboard_buffer_t {
    char buffer[500];
    int front;
    int end;
    int size;
};

#define E0_SIGN (1 << 0)
#define SHIFT (1 << 1)
#define CAPS_LOCK (1 << 2)

char read_key_buffer(void);
unsigned char in_byte(uint16_t port);
void keyboard_handler(void);

#endif