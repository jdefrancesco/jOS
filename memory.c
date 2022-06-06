#include <stdint.h>
#include <stddef.h>

#include "memory.h"
#include "print.h"
#include "debug.h"

static struct free_mem_region_t free_mem_region[50];

// We need to get the available memory from the BIOS
void init_memory(void) 
{
    uint64_t total_mem = 0;
    volatile int32_t count  = *(int32_t *)0x9000;
    struct e820_t *mem_map =  (struct e820_t *)0x9008;
    int free_region_count = 0;

    ASSERT(count <= 50);

    for (int i = 0; i < count; i++) {
        if (mem_map[i].type == 1) {
            free_mem_region[free_region_count].address = mem_map[i].address;
            free_mem_region[free_region_count].length = mem_map[i].length;
            total_mem += mem_map[i].length;
            free_region_count++;
        }

        printk("%x  %uKB %u\n", mem_map[i].address, mem_map[i].length/1024, (uint64_t)mem_map[i].type);
    } 

    // Give total memory in MiB
    printk("Total memory is %uMB\n", total_mem/1024/1024);
}