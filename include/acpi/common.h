#ifndef ACPI_COMMON_H
#define ACPI_COMMON_H

#ifndef __ASSEMBLER__

#include <kdef.h>

#include <kernel/types.h>

struct acpi_header {
    union {
        char     str[4];         /* "RSDT", "APIC", etc */
        uint32_t val; 
    } signature;
    uint32_t length;             /* Total table size including header */
    uint8_t  revision;           /* ACPI specification revision */
    uint8_t  checksum;           /* Entire table must sum to 0 */
    char     oem_id[6];          /* OEM ID string */
    char     oem_table_id[8];    /* OEM Table ID string */
    uint32_t oem_revision;       /* OEM Revision number */
    uint32_t creator_id;         /* Creator Vendor ID */
    uint32_t creator_revision;   /* Creator Revision number */
} PACKED;

struct acpi_generic_address {
    uint8_t  address_space_id;  // 0 = System Memory, 1 = System I/O
    uint8_t  register_bit_width;
    uint8_t  register_bit_offset;
    uint8_t  access_size;
    uint64_t address;            // 64-bit physical address / port
} PACKED;


#endif // __ASSEMBLER__
       
/**
 * ACPI Table Signature Constants (32-bit Little-Endian)
 * Used to identify top-level system description tables by matching their 4-character ASCII signatures.
 */
#define ACPI_SIG_RSDT 0x54445352 // RSDT
#define ACPI_SIG_XSDT 0x54445358 // XSDT

#define ACPI_SIG_MADT 0x43495041 // APIC
#define ACPI_SIG_FADT 0x50434146 // FACP
#define ACPI_SIG_HPET 0x54455048 // HPET
#define ACPI_SIG_WAET 0x54454157 // WAET
#define ACPI_SIG_MCFG 0x4746434D // MCFG

#define ACPI_SIG_DSDT 0x54445344 // DSDT


#endif // ACPI_COMMON_H
