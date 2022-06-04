#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdint.h>

struct e820_t {
    uint64_t address;
    uint64_t length;
    uint32_t type;
}__attribute__((packed));

// As some memory regions aren't available to us, we track
// such info with this data structure.
struct free_mem_region_t {
    uint64_t address;
    uint64_t length;
};

void init_memory(void);
#endif 