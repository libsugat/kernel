#include <kdef.h>
#include <kernel/mm.h>

#include <lib/kprint.h>

static struct mem_map mmap;

static UNUSED void dump_alloc_map() {
    kprintf("====== Max order: %d \n", MAX_ORDER);
    kprintf("====== Freelist ======\n");
    for (int i = 0; i < MAX_ORDER_ALLOC; i++) {
        struct page_meta * curr = mmap.freelist[i];
        kprintf("Order %d freelist: ", i);
        while (curr) {
            size_t pfn = curr - mmap.page_meta_arr;
            kprintf("%u ", pfn);
            curr = curr->next;
        }
        kputc('\n');
    }
}

void init_mm(struct boot_info_struct * boot_info) {
    arch_preprocess_memory_map(boot_info);
    arch_init_mem_map(boot_info, &mmap);
    // dump_alloc_map();
}

void* page_meta_to_start_addr(const struct page_meta * page) {
    if (!page) return NULL;
    uintptr_t pfn = page - mmap.page_meta_arr;
    return (void *)(pfn * PAGE_SIZE);
}


#define BUDDY_PFN(pfn, order)       (pfn) ^ ((size_t)1 << (order))
#define BUDDY_PAIR_BASE(pfn, order) (pfn) & ~((size_t)1 << (order))

// TODO:
// - putting to freelist
static void put_to_freelist(struct page_meta* page, uint8_t order) {
    page->next = mmap.freelist[order];
    page->prev = NULL; // just defensive
    page->status = PAGE_STATUS_FREE;
    mmap.freelist[order] = page;
    if (page->next) {
        page->next->prev = page;
    }
}

static ALWAYS_INLINE void freelist_unlink(struct page_meta* page, uint8_t order) {
    if (page->next) {
        page->next->prev = page->prev;
    }
    if (page->prev) {
        page->prev->next = page->next;
    } else {
        // Was the head of the list
        mmap.freelist[order] = page->next;
    }
    page->next = NULL;
    page->prev = NULL;
}

// TODO:
// - page ptr to pfn

struct page_meta* alloc_page_frame(uint8_t order) {

    if (order > MAX_ORDER) {
        kprintf("[alloc_page_frame] order > MAX_ORDER\n");
        return NULL;
    }

    uint8_t free_order = order;
    struct page_meta * freepage = NULL;

    for (;free_order <= MAX_ORDER; free_order++) {
        freepage = mmap.freelist[free_order];
        if (freepage) {
            freelist_unlink(freepage, free_order);
            break;
        }
    }

    if (!freepage)
    {
        kprintf("[alloc_page_frame] free_order >= MAX_ORDER, or no page of order %d available\n", order);
        return NULL; // Out of memory
    }

    size_t pfn = freepage - mmap.page_meta_arr;

    // split till target order
    while (free_order > order) {
        free_order--;
        size_t buddy_pfn = BUDDY_PFN(pfn, free_order);
        
        if (buddy_pfn >= mmap.n_pages) break;
        
        mmap.page_meta_arr[buddy_pfn].order = free_order;
        put_to_freelist(&mmap.page_meta_arr[buddy_pfn], free_order);
    }

    // mark the final as used
    mmap.page_meta_arr[pfn].order = order;
    mmap.page_meta_arr[pfn].status = PAGE_STATUS_USED;
    mmap.page_meta_arr[pfn].next = NULL;
    mmap.page_meta_arr[pfn].prev = NULL;
    mmap.page_meta_arr[pfn].ref_cnt = 1;

    return &mmap.page_meta_arr[pfn];
}

void free_page_frame(struct page_meta* page) {
    if (!page) return;

    size_t pfn = page - mmap.page_meta_arr;

    if (pfn >= mmap.n_pages) {
        kprintf("[page_free] pfn(%u) > total pages(%u)\n", pfn, mmap.n_pages);
        return;
    }
    if (mmap.page_meta_arr[pfn].status == PAGE_STATUS_FREE) {
        kprintf("[page_free] pfn(%u) already free; Potential Double free\n", pfn);
        return;
    }

    mmap.page_meta_arr[pfn].status = PAGE_STATUS_FREE;
    struct page_meta * curr_page = &mmap.page_meta_arr[pfn];

    uint8_t current_order = curr_page->order;

    while (current_order < MAX_ORDER) {
        size_t buddy_pfn = BUDDY_PFN(pfn, current_order);

        if (buddy_pfn >= mmap.n_pages) break; // Out of bound check

        struct page_meta * buddy = &mmap.page_meta_arr[buddy_pfn];
        if (buddy->status != PAGE_STATUS_FREE || buddy->order != current_order) {
            break;
        }

        // free buddy form freelist
        freelist_unlink(buddy, current_order);
        buddy->order = 0;
        buddy->status = PAGE_STATUS_TRAILING;
        curr_page->order = 0;
        curr_page->status = PAGE_STATUS_TRAILING;
        
        pfn = BUDDY_PAIR_BASE(buddy_pfn, current_order);
        curr_page = &mmap.page_meta_arr[pfn];
        curr_page->order = ++current_order;
    }
    
    // put the current page to free list
    put_to_freelist(curr_page, current_order);
}
