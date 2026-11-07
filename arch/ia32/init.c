#include<arch/init.h>

#include<arch/apic.h>
#include<arch/gdt.h>

#include <lib/kprint.h>

struct gdt_segment_descriptor gdt_table[] = {
    {0, 0, 0, 0, 0, 0}, // Null descriptor
    {
        // Kernel Code segment
        .limit_low = 0xFFFF,
        .base_low = 0x00,
        .base_mid = 0x00,
        .access_byte = 0x9A,
        .flags_limit = 0b11001111,
        .base_high = 0x00
    },
    {
        // Kernel Data segment
        .limit_low = 0xFFFF,
        .base_low = 0x00,
        .base_mid = 0x00,
        .access_byte = 0x92,
        .flags_limit = 0b11001111,
        .base_high = 0x00
    },
    {
        // User Code segment
        .limit_low = 0xFFFF,
        .base_low = 0x00,
        .base_mid = 0x00,
        .access_byte = 0xFA,
        .flags_limit = 0b11001111,
        .base_high = 0x00
    },
    {
        // User Data segment
        .limit_low = 0xFFFF,
        .base_low = 0x00,
        .base_mid = 0x00,
        .access_byte = 0xF2,
        .flags_limit = 0b11001111,
        .base_high = 0x00
    },
    // TODO: Add segment for TSS
};

struct gdtr gdt_reg = {
    .limit = sizeof(gdt_table) - 1,
    .base = (uint32_t)&gdt_table
};

void cpu_init_early(void) {
    __asm__ __volatile__(
        /* Load the new GDT */
        "lgdt %0\n\t"

        /* Reload data segments using %1 */
        "movw %1, %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "movw %%ax, %%ss\n\t"

        /* Reload CS (0x08) with a far return */
        "pushl $0x08\n\t"
        "pushl $1f\n\t"
        "lret\n\t"
        "1:\n\t"
        :
        : "m" (gdt_reg), "i" ((uint16_t)0x10)
        : "memory"
    );
}

extern void parse_acpi(struct rsdp *found_rsdp);
void arch_hardware_init(struct boot_info_struct * boot_info) {
    parse_acpi(boot_info->rsdp_ptr_struct);
}
