#include <kdef.h>
#include <kernel/mm.h>

#include <arch/mm.h>

#include <lib/kprint.h>

static struct mem_map mmap;

static void dump_alloc_map() {
    // kprintf("====== Slots ======\n");
    // for (size_t i = 0; i < mmap.n_pages; i++) {
    //     kprintf("[pfn: %u; order: %u; used: %u]\n", i, mmap.page_meta_arr[i].order, mmap.page_meta_arr[i].status);
    // }
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
    dump_alloc_map();
}

// TODO: split things like below in static inline functions 
// - putting to freelist
// - taking out of freelist
// - taking page out of freelist
// - page ptr to pfn
struct page_meta* alloc_page_frame(uint8_t order) {

    if (order > MAX_ORDER) {
        kprintf("[page_alloc] order > MAX_ORDER\n");
        return NULL;
    }

    uint8_t free_order = order;
    while (free_order < MAX_ORDER && !mmap.freelist[free_order]) {
        free_order++;
    }

    if (free_order >= MAX_ORDER_ALLOC || !mmap.freelist[free_order])
    {
        kprintf("[page_alloc] free_order >= MAX_ORDER, or no page of order %d available\n", order);
        return NULL; // Out of memory
    }

    // Take the slot
    struct page_meta * freepage = mmap.freelist[free_order];
    mmap.freelist[free_order] = freepage->next;
    if (mmap.freelist[free_order]) {
        mmap.freelist[free_order]->prev = NULL;
    }

    size_t pfn = freepage - mmap.page_meta_arr;

    // split till target order
    while (free_order > order) {
        free_order--;
        size_t buddy_pfn = pfn ^ (1UL << free_order);
        
        if (buddy_pfn >= mmap.n_pages) break;
        
        mmap.page_meta_arr[buddy_pfn].order = free_order;
        mmap.page_meta_arr[buddy_pfn].status = PAGE_STATUS_FREE;

        // add buddy to free list
        mmap.page_meta_arr[buddy_pfn].next = mmap.freelist[free_order]; 
        if (mmap.freelist[free_order]) {
            mmap.freelist[free_order]->prev = &mmap.page_meta_arr[buddy_pfn];
        }
        mmap.freelist[free_order] = &mmap.page_meta_arr[buddy_pfn]; 

        mmap.page_meta_arr[buddy_pfn].order = free_order;
    }

    mmap.page_meta_arr[pfn].order = order;
    mmap.page_meta_arr[pfn].status = PAGE_STATUS_USED;
    mmap.page_meta_arr[pfn].next = NULL;
    mmap.page_meta_arr[pfn].prev = NULL;
    mmap.page_meta_arr[pfn].ref_cnt = 1;

    // mark the final as used
    return &mmap.page_meta_arr[pfn];
}

void free_page_frame(struct page_meta* page) {
    if (!page) {return;}

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
        size_t buddy_pfn = pfn ^ (1UL << current_order);
        struct page_meta * buddy = &mmap.page_meta_arr[buddy_pfn];
        if (buddy->status != PAGE_STATUS_FREE || buddy->order != current_order) {
            break;
        }

        // free buddy form freelist
        if (buddy->prev)
            buddy->prev->next = buddy->next;
        if (buddy->next)
            buddy->next->prev = buddy->prev;
        if (mmap.freelist[current_order] == buddy) 
            mmap.freelist[current_order] = buddy->next;
        buddy->order = 0;
        buddy->status = PAGE_STATUS_TRAILING;
        curr_page->order = 0;
        curr_page->status = PAGE_STATUS_TRAILING;
        
        pfn = buddy_pfn & ~(1UL << current_order);
        curr_page = &mmap.page_meta_arr[pfn];
        curr_page->order = ++current_order;
    }
    
    // put the current page to free list
    curr_page->next = mmap.freelist[current_order];
    curr_page->prev = NULL; // just defensive
    curr_page->status = PAGE_STATUS_FREE;
    mmap.freelist[current_order] = curr_page;
    if (curr_page->next) {
        curr_page->next->prev = curr_page;
    }
}
