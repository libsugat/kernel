#ifndef ACPI_FADT_H
#define ACPI_FADT_H

#ifndef __ASSEMBLER__

#include <kdef.h>

#include <kernel/types.h>

#include <acpi/common.h>

struct fadt {
    struct acpi_header header;
    
    // ACPI 1.0 32-bit Fields
    uint32_t firmware_ctrl;      // Physical address of FACS
    uint32_t dsdt;               // Physical address of DSDT
    uint8_t  __reserved1;        // Unused in ACPI 1.0 (Model)
    uint8_t  preferred_pm_profile;
    uint16_t sci_interrupt;      // System Control Interrupt (IRQ vector)
    uint32_t smi_command_port;   // System Management Interrupt port
    uint8_t  acpi_enable;        // Value to write to smi_command_port to enable ACPI
    uint8_t  acpi_disable;       // Value to write to smi_command_port to disable ACPI
    uint8_t  s4bios_req;         // Value to write to SMI port to enter S4BIOS
    uint8_t  pstate_control;     // Value for CPU performance state control
    uint32_t pm1a_event_block;   // I/O port address for PM1a Event Regs
    uint32_t pm1b_event_block;   // I/O port address for PM1b Event Regs
    uint32_t pm1a_control_block; // I/O port address for PM1a Control Regs
    uint32_t pm1b_control_block; // I/O port address for PM1b Control Regs
    uint32_t pm2_control_block;  // I/O port address for PM2 Control Regs
    uint32_t pm_timer_block;     // I/O port address for PM Timer
    uint32_t gpe0_block;         // I/O port address for GPE0
    uint32_t gpe1_block;         // I/O port address for GPE1
    uint8_t  pm1_event_length;
    uint8_t  pm1_control_length;
    uint8_t  pm2_control_length;
    uint8_t  pm_timer_length;
    uint8_t  gpe0_length;
    uint8_t  gpe1_length;
    uint8_t  gpe1_base;
    uint8_t  cstate_control;     // C-state control
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alarm;          // RTC alarm offset
    uint8_t  month_alarm;        // RTC alarm offset
    uint8_t  century;            // RTC century register offset
    
    // IA-PC Boot Architecture Flags
    uint16_t boot_architecture_flags; // <--- BIT 1 IS YOUR PS/2 8042 CHECK!
    
    uint8_t  __reserved2;
    uint32_t flags;              // Fixed Feature Flags

    // ACPI 2.0+ 64-bit Extended Fields (GAS blocks)
    struct acpi_generic_address reset_reg;
    uint8_t  reset_value;
    uint16_t arm_boot_architecture_flags;
    uint8_t  minor_version;
    uint64_t x_firmware_ctrl;    // 64-bit physical address of FACS
    uint64_t x_dsdt;             // 64-bit physical address of DSDT
    
    struct acpi_generic_address x_pm1a_event_block;
    struct acpi_generic_address x_pm1b_event_block;
    struct acpi_generic_address x_pm1a_control_block;
    struct acpi_generic_address x_pm1b_control_block;
    struct acpi_generic_address x_pm2_control_block;
    struct acpi_generic_address x_pm_timer_block;
    struct acpi_generic_address x_gpe0_block;
    struct acpi_generic_address x_gpe1_block;
    
    struct acpi_generic_address sleep_control_reg;
    struct acpi_generic_address sleep_status_reg;
    uint64_t hypervisor_vendor_identity;
} PACKED;

#endif // __ASSEMBLER__
#endif // ACPI_FADT_H
