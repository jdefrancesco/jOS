#include "trap.h"

#define VECTOR_COUNT 256
static struct idt_ptr idt_pointer;
static struct idt_entry vectors[VECTOR_COUNT];

// init_idt_entry initalizes the Interrupt Descriptor Table
static void
init_idt_entry(struct idt_entry *entry, uint64_t addr, uint8_t attrib)
{
    entry->low = (uint64_t)addr;
    entry->selector = 8;
    entry->attr = attrib;
    entry->mid = (uint16_t) (addr>>16);
    entry->high = (uint32_t) (addr>>32);
}

void init_idt(void)
{
    init_idt_entry(&vectors[0],  (uint64_t) vector0, 0x8e);
    init_idt_entry(&vectors[1],  (uint64_t) vector1, 0x8e);
    init_idt_entry(&vectors[2],  (uint64_t) vector2, 0x8e);
    init_idt_entry(&vectors[3],  (uint64_t) vector3, 0x8e);
    init_idt_entry(&vectors[4],  (uint64_t) vector4, 0x8e);
    init_idt_entry(&vectors[5],  (uint64_t) vector5, 0x8e);
    init_idt_entry(&vectors[6],  (uint64_t) vector6, 0x8e);
    init_idt_entry(&vectors[7],  (uint64_t) vector7, 0x8e);
    init_idt_entry(&vectors[8],  (uint64_t) vector8, 0x8e);
    init_idt_entry(&vectors[10], (uint64_t) vector10, 0x8e);
    init_idt_entry(&vectors[11], (uint64_t) vector11, 0x8e);
    init_idt_entry(&vectors[12], (uint64_t) vector12, 0x8e);
    init_idt_entry(&vectors[13], (uint64_t) vector13, 0x8e);
    init_idt_entry(&vectors[14], (uint64_t) vector14, 0x8e);
    init_idt_entry(&vectors[16], (uint64_t) vector16, 0x8e);
    init_idt_entry(&vectors[17], (uint64_t) vector17, 0x8e);
    init_idt_entry(&vectors[18], (uint64_t) vector18, 0x8e);
    init_idt_entry(&vectors[19], (uint64_t) vector19, 0x8e);
    init_idt_entry(&vectors[32], (uint64_t) vector32, 0x8e);
    init_idt_entry(&vectors[39], (uint64_t) vector39, 0x8e);

    idt_pointer.limit = sizeof(vectors) - 1;
    idt_pointer.addr = (uint64_t) vectors;
    load_idt(&idt_pointer);

}

// Handler of interrupts
void handler(struct trap_frame *tf)
{
    unsigned char isr_value;
    switch (tf->trapno) {
        // timer
        case 32:
            eoi();
            break;

        // Spur. interrupts or not.
        case 39:
            isr_value = read_isr();
            if ((isr_value & (1 << 7)) != 0) {
                eoi();
            }
            break;

        default:
            while (1) { }
    }
}
