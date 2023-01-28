#include "keyboard.h"
#include "print.h"
#include "process.h"

#include <stdint.h>

static const unsigned char kShiftCode[] = {
    [0x2A] = SHIFT,
    [0x36] = SHIFT,
    [0xAA] = SHIFT,
    [0xB6] = SHIFT
};

static const unsigned char kLockCode[] = {
    [0x3A] = CAPS_LOCK,
};

// Scancode set 1. If key is 0 it is invalid.
static const unsigned char kKeyMap[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=', '\b', 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
    'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f',
    'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
    'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static const unsigned char kShiftKeyMap[] = {
    0, 1, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
    'I', 'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};


static struct keyboard_buffer_t key_buffer = { {0}, 0, 0, 500};
static unsigned int flag;

// These functions implement a ring buffer...
static void write_key_buffer(char ch)
{
    int front = key_buffer.front;
    int end = key_buffer.end;
    int size = key_buffer.size;

    if ((end + 1) % size == front) {
        return;
    }
    key_buffer.buffer[end++] = ch;
    key_buffer.end = end % size;
}

char read_key_buffer(void)
{
    int front = key_buffer.front;

    if (front == key_buffer.end) {
        sleep(-2);       
    }
    
    key_buffer.front = (key_buffer.front + 1) % key_buffer.size;
    return key_buffer.buffer[front];
}


// Obtain value from keyboard with inb interface.
static char keyboard_read(void) 
{
    unsigned char scan_code;
    char ch; 

    scan_code = in_byte(0x60);

    if (scan_code == 0xE0) {
        flag |= E0_SIGN;
        return 0;
    }

    if (flag & E0_SIGN) {
        flag &= ~E0_SIGN;
        return 0;
    }

    // Check for keyup... We can clear flag if we don't need it.
    if (scan_code & 0x80 ) {
        flag &= ~(kShiftCode[scan_code]);
        return 0;
    }

    // Handle shift being held in.
    flag |= kShiftCode[scan_code];
    flag ^= kLockCode[scan_code];

    if ( flag & SHIFT) {
        ch = kShiftCode[scan_code];
    } else {
        ch = kKeyMap[scan_code];
    }

    // Check for caps lock.
    if (flag & CAPS_LOCK) {
        if ('a' <= ch && ch <= 'z') {
            ch -= 32;
        } else if ('A' <= ch && ch <= 'Z') {
            ch += 32;
        }
    }

    return ch;
}

void keyboard_handler(void) 
{
    char ch = keyboard_read();
    if (ch > 0) {
        write_key_buffer(ch);
        // Wake up with -2 meaning processes waiting for kb.
        wake_up(-2);
    }
}