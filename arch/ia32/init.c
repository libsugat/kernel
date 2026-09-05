#include<arch/init.h>

#include<arch/apic.h>

extern void parse_acpi(struct rsdp *found_rsdp);

void arch_hardware_init(struct boot_info_struct * boot_info) {
    parse_acpi(boot_info->rsdp_ptr_struct);
}
