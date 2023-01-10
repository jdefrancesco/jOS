#include <stdint.h>

#include "keyboard.h"
#include "print.h"
#include "process.h"

static unsigned char shift_code[256] = {
    [0x2A] = SHIFT,
    [0x36] = SHIFT,
    [0xAA] = SHIFT,
    [0xB6] = SHIFT
};

static unsigned char lock_code[256] = {
    [0x3A] = CAPS_LOCK,
};

static unsigned char key_map[256] = {

};