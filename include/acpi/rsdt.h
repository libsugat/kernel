#ifndef ACPI_RSDT_H
#define ACPI_RSDT_H

#ifndef __ASSEMBLER__

#include <kdef.h>

#include <kernel/types.h>

#include <acpi/common.h>

/* Root System Description Table (ACPI 1.0 - 32-bit Pointers) */
struct rsdt {
    struct acpi_header header;
    /* 
     * Flexible array member containing 32-bit physical addresses 
     * of other ACPI tables (MADT, MCFG, HPET, FADT, etc.).
     * Number of entries = (header.length - sizeof(acpi_header)) / sizeof(uint32_t)
     */
    uint32_t sdt_pointers[];
} PACKED;

struct rsdp {
    char signature[8];
    uint8_t shecksum;
    char oemid[6];
    uint8_t revision;
    struct rsdt *rsdt;
} PACKED;

struct xsdp {
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    struct rsdt *rsdt;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} PACKED;

#endif // __ASSEMBLER__
#endif // ACPI_RSDT_H
