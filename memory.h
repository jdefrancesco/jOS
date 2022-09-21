#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdint.h>
#include <stdbool.h>

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

struct page_t {
    struct page_t *next;
};

// Define our types for various page table levels.
typedef uint64_t PDE;
typedef PDE* PD;
typedef PD* PDPTR;

#define PTE_P 1
#define PTE_W 2
#define PTE_U 4
#define PTE_ENTRY 0x80
// Base address where our kernel lives (VA).
#define KERN_BASE 0xffff800000000000
// 2 MB Page size.
#define PAGE_SIZE (2 * 1024 * 1024)

// The next to Macros convert from phys to virt addresses and vice-versa.
// Need to take care of alignment which is why we shift things around.
#define PA_UP(v) ((((uint64_t)(v) + PAGE_SIZE-1) >> 21 ) << 21)
#define PA_DOWN(v) (((uint64_t)(v) >> 21) << 21)

// Convert kernel virtual address Macros.
#define P2V(p) ((uint64_t) (p) + KERN_BASE)
#define V2P(v) ((uint64_t) (v) - KERN_BASE)

// Kernel has 1GB VMA. This represents the end of that address space.
#define MAX_ADDR 0xffff800040000000

/*
 * Function Prototypes..
 */
void init_memory(void);
void * kalloc(void);
void kfree(uint64_t v);
bool map_pages(uint64_t map, uint64_t v, uint64_t e, uint64_t pa, uint32_t attribute);
void switch_vm(uint64_t map);
void init_kvm(void);

#endif
