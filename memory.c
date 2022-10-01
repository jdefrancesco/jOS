#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "klib.h"
#include "memory.h"
#include "print.h"
#include "debug.h"

// Free a memory region.
static void free_region(uint64_t v, uint64_t e);

static struct free_mem_region_t free_mem_region[50];
static struct page_t free_memory;
static uint64_t memory_end;
uint64_t page_map;
extern char end;

// We need to get the available memory from the BIOS
void init_memory(void)
{
    // Store size of free memory we can use.
    uint64_t total_mem = 0;

    int32_t count  = *(int32_t *)0x9000;
    struct e820_t *mem_map =  (struct e820_t *)0x9008;

    // Store count of free memory regions.
    int free_region_count = 0;

    // Make sure number of memory regions is less than fifty.
    ASSERT(count <= 50);

    for (int32_t i = 0; i < count; i++) {
        if (mem_map[i].type == 1) {
            free_mem_region[free_region_count].address = mem_map[i].address;
            free_mem_region[free_region_count].length = mem_map[i].length;
            total_mem += mem_map[i].length;
            free_region_count++;
        }

        printk("%x  %uKB %u\n", mem_map[i].address, mem_map[i].length/1024, (uint64_t)mem_map[i].type);
    }

    for (size_t i = 0; i < free_region_count; i++) {
        // We need vstart and vend to use VA
        uint64_t vstart = P2V(free_mem_region[i].address);
        uint64_t vend = vstart + free_mem_region[i].length;

        // We need address of end since its provided by linker script it's not a variable, it's treated as symbol.
        if (vstart > (uint64_t)&end) {
            free_region(vstart, vend);
        } else if (vend > (uint64_t)&end) {
            free_region((uint64_t)&end, vend);
        }
    }

    memory_end = (uint64_t) free_memory.next+PAGE_SIZE;
    printk("%x\n", memory_end);

}


// free_region frees a previously allocated memory region.
static void free_region(uint64_t v, uint64_t e)
{
    for (size_t start = PA_UP(v); start+PAGE_SIZE <= e; start += PAGE_SIZE) {
        if (start+PAGE_SIZE <= MAX_ADDR) {
            kfree(start);
        }
    }
}



// Free memory adding it back to our pool.
void kfree(uint64_t v)
{
    // Make sure page aligned.
    ASSERT((v % PAGE_SIZE) == 0);
    // Virt address is within kernel.
    ASSERT(v >= (uint64_t)&end);
    // Check one GiB mem limit.
    ASSERT(v+PAGE_SIZE <= MAX_ADDR);

    struct page_t *page_addr = (struct page_t *)v;
    page_addr->next = free_memory.next;
    free_memory.next = page_addr;
}

// Allocate memory from our free memory region pool.
void * kalloc(void)
{
    struct page_t *page_addr = free_memory.next;
    if (page_addr != NULL) {
        ASSERT((uint64_t)page_addr % PAGE_SIZE == 0);
        ASSERT((uint64_t)page_addr >= (uint64_t)&end);
        ASSERT((uint64_t)page_addr+PAGE_SIZE <= MAX_ADDR);
        // Remove page from list.
        free_memory.next = page_addr->next;
    }

    printk("page_addr = %x", page_addr);
    return page_addr;
}




/*
 *
 */
static PDPTR 
find_pml4t_entry(uint64_t map, uint64_t v, int alloc, uint32_t attribute)
{
    PDPTR *map_entry = (PDPTR *) map;
    PDPTR pdptr = NULL;
    unsigned int index = (v >> 39) & 0x1FF;

    if ((uint64_t)map_entry[index] & PTE_P) {
        pdptr = (PDPTR) P2V(PDE_ADDR(map_entry[index]));
    } else if (alloc == 1) {
        pdptr = (PDPTR) kalloc();
        if (pdptr != NULL) {
            memset(pdptr, 0, PAGE_SIZE);
            map_entry[index] = (PDPTR) (V2P(pdptr) | attribute);
        }
    }

    return pdptr;

}

/*
 *
 *
 */
static PD 
find_pdpt_entry(uint64_t map, uint64_t v, int alloc, uint32_t attribute) 
{
    PDPTR pdptr = NULL;
    PD pd = NULL;
    unsigned int index = (v >> 30) & 0x1FF;

    pdptr = find_pml4t_entry(map, v, alloc, attribute);
    if (pdptr == NULL)
        return NULL;

    if ((uint64_t)pdptr[index] & PTE_P) {
        pd = (PD) P2V(PDE_ADDR(pdptr[index]));
    } else if (alloc == 1) {
        pd = (PD) kalloc();
        if (pd != NULL) {
            memset(pd, 0, PAGE_SIZE);
            pdptr[index] = (PD) (V2P(pd) | attribute);
        }
    }
    return pd;
}

bool map_pages(uint64_t map, 
    uint64_t v, 
    uint64_t e, 
    uint64_t pa, 
    uint32_t attribute)
{
    uint64_t vstart = PA_DOWN(v);
    uint64_t vend = PA_UP(e);
    PD pd = NULL;
    unsigned int index;

    ASSERT(v < e);
    ASSERT(pa % PAGE_SIZE == 0);
    ASSERT(pa + vend - vstart <= 1024 * 1024 * 1024);

    do {
        pd = find_pdpt_entry(map, vstart, 1, attribute);
        if (pd == NULL) {
            return false;
        }

        index = (vstart >> 21) & 0x1FF;
        ASSERT(((uint64_t)pd[index] & PTE_P) == 0);

        pd[index] = (PDE) (pa | attribute | PTE_ENTRY);
        vstart += PAGE_SIZE;
        pa += PAGE_SIZE;

    } while ((vstart + PAGE_SIZE) <= vend);

    return true;
}

void switch_vm(uint64_t map)
{
    // CR3 Needs physical address.
    load_cr3(V2P(map));
}

// Function used to remap our kernel using 2MiB pages.
static void setup_kvm(void)
{
    // This page is the new PML4 table.
    page_map = (uint64_t) kalloc();
    ASSERT(page_map != 0);

    memset((void*)page_map, 0x00, PAGE_SIZE);
    bool status = map_pages(page_map, KERN_BASE, memory_end, V2P(KERN_BASE), PTE_P|PTE_W);
    ASSERT(status == true);
}


void init_kvm(void)
{
    // Remap kernel
    setup_kvm();
    // Set CR3
    switch_vm(page_map);
    printk("Memory Manager initialized...");
}
