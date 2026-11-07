#ifndef KERNEL_MM_H
#define KERNEL_MM_H

#ifndef __ASSEMBLER__

#include <kdef.h>
#include <kernel/types.h>
#include <arch/mm.h>

struct boot_info_struct;

enum page_status {
    PAGE_STATUS_FREE = 0,
    PAGE_STATUS_USED = 1,
    PAGE_STATUS_TRAILING = 0xFF,
    PAGE_STATUS_RESERVED = 0xFE
};

struct ALIGNED(16) page_meta {
    union {
        struct {
            struct page_meta *next, *prev;
        };
        struct {
            uint16_t ref_cnt;
        };
    };
    uint8_t status;
    uint8_t order;
};

#define MAX_ORDER 10
#define MAX_ORDER_ALLOC (MAX_ORDER + 1)

struct mem_map {
    size_t n_pages;
    struct page_meta *page_meta_arr;
    struct page_meta *freelist[MAX_ORDER_ALLOC];
};

STATIC_ASSERT(sizeof(struct page_meta) <= 64, "struct page_meta >= 64; struct size limit crossed");

void init_mm(struct boot_info_struct *);
struct page_meta* alloc_page_frame(uint8_t order);
void free_page_frame(struct page_meta *);

#endif // __ASSEMBLER__
#endif // KERNEL_MM_H 
