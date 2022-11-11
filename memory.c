#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "klib.h"
#include "memory.h"
#include "print.h"
#include "debug.h"

/* 
 *  Virtual Memory Layout
 * [============]  
 * [            ]
 * [            ]
 * [KERNEL SPACE] 0xFFFF800000000000
 * [------------]
 * [ Non-Canon  ]                       Our kernel lives here initially, and then is remapped
 * [            ]                       to HIGH_MEM at VA shown in diagram to left. Note
 * [            ]                       our kernel is using 2MiB pages which it has stored in a list.
 * [------------]                       [==============]
 * [            ]                       [ Phys. Mem 1GB]
 * [ USER SPACE ]                       [              ]
 * [============]                       [==============]
 */

// Free a memory region.
static void free_region(uint64_t v, uint64_t e);
static PD find_pdpt_entry(uint64_t map, uint64_t v, int alloc, uint32_t attribute);

static struct free_mem_region_t free_mem_region[50];
static struct page_t free_memory;
static uint64_t memory_end;

extern char end;

// We need to get the available memory map from the BIOS
// We will break up kernel into 2MB phys pages.
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

    for (int32_t i = 0; i < free_region_count; i++) {
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

    printk("Total memory = %uMB\n", total_mem/1024/1024);
    // end variable from our linker script.
    memory_end = (uint64_t) free_memory.next+PAGE_SIZE;
    printk("Memory end = %x\n", memory_end);

}


// free_region frees a previously allocated memory region.
static void free_region(uint64_t v, uint64_t e)
{
    for (uint64_t start = PA_UP(v); start+PAGE_SIZE <= e; start += PAGE_SIZE) {
        if (start+PAGE_SIZE <= MAX_ADDR) {
            kfree(start);
        }
    }
}

void free_pages(uint64_t map, uint64_t vstart, uint64_t vend)
{
    unsigned int index = 0;

    ASSERT((vstart % PAGE_SIZE) == 0);
    ASSERT((vend % PAGE_SIZE) == 0);

    do {
        PD pd = find_pdpt_entry(map, vstart, 0, 0);

        if (pd != NULL) {
            index = (vstart >> 21) & 0x1FF;
            // Page should be in memory.
            if (pd[index] & PTE_P) {
                kfree(P2V(PTE_ADDR(pd[index])));
                pd[index] = 0;
            }
        }

        vstart += PAGE_SIZE;
    } while ((vstart+PAGE_SIZE) <= vend);
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

    return page_addr;
}



/* find_pml4t_entry()
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

/* find_pdpt_entry 
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

// Map pages creates page table for the kernel.
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

    // Make sure we are within bounds.
    ASSERT(v < e);
    // Make sure we are page aligned.
    ASSERT(pa % PAGE_SIZE == 0);
    ASSERT((pa+vend-vstart) <= (1024 * 1024 * 1024));

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
// NOTE: We obtain new page tables with kalloc()
uint64_t setup_kvm(void)
{
    // This page (page_map) is the new PML4 table.
    uint64_t page_map = (uint64_t) kalloc();

    if (page_map != 0) {
        memset((void*)page_map, 0x00, PAGE_SIZE);
        if (!map_pages(page_map, KERN_BASE, memory_end, V2P(KERN_BASE), PTE_P|PTE_W)) {
            free_vm(page_map);
            page_map = 0;
        }
    }
    return page_map;
}


void init_kvm(void)
{
    // Remap kernel
    uint64_t page_map = setup_kvm();
    ASSERT(page_map != 0);
    // Set CR3 with new mapping.
    switch_vm(page_map);
    printk("Memory Manager initialized...\n");
}

// Build user VM space.
bool setup_uvm(uint64_t map, uint64_t start, int size)
{
    bool status = false;
    void *page = kalloc();

    if (page != NULL) {
        memset(page, 0x00, PAGE_SIZE);
        // For now each process gets one 2MB page.
        status = map_pages(map, 0x400000, 0x400000+PAGE_SIZE, V2P(page), PTE_P|PTE_W|PTE_U);
        if (status == true) {
            memcpy(page, (void* )start, size);
        } else {
            kfree((uint64_t)page);
            free_vm(map);
        }
    }
    return status;
}

// Free PDT
static void free_pdt(uint64_t map)
{
    const uint64_t kPageDirSize = 512;
    PDPTR *map_entry = (PDPTR*)map;
    for (size_t i = 0; i < kPageDirSize; i++) {
        if ((uint64_t) map_entry[i] & PTE_P) {
            PD *pdptr = (PD*) P2V(PDE_ADDR(map_entry[i]));

            for (size_t j = 0; j < kPageDirSize; j++) {
                if ((uint64_t)pdptr[i] & PTE_P) {
                    kfree(P2V(PDE_ADDR(pdptr[j])));
                    pdptr[j] = 0;
                }
            }
        }
    }
}

static void free_pdpt(uint64_t map)
{
    PDPTR *map_entry = (PDPTR*) map;

    for (size_t i = 0; i < 512; i++) {
        if ((uint64_t) map_entry[i] & PTE_P) {
            kfree(P2V(PDE_ADDR(map_entry[i])));
            map_entry[i] = 0;
        }
    }
}

static void free_pml4t(uint64_t map) 
{
    kfree(map);
}

void free_vm(uint64_t map) 
{
    free_pages(map, 0x400000, 0x400000+PAGE_SIZE);
    free_pdt(map);
    free_pdpt(map);
    free_pml4t(map);
}