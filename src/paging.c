#include <paging.h>

typedef unsigned long long uint64_t;

typedef struct {
    uint64_t entries[512];
} page_t __attribute__((aligned(4096)));

#define NUM_PAGE_TABLES 16

// Page Tables
page_t pml4;
page_t pdpt;
page_t pd;
page_t pt[NUM_PAGE_TABLES];

void PreparePaging() {
    // Prepare PML4
    pml4.entries[0] = (uint64_t)(&pdpt) + 3;
    for (int i = 1; i < 512; i++)
        pml4.entries[i] = 0;
    pml4.entries[256] = pml4.entries[0];

    // Prepare PDPT
    pdpt.entries[0] = (uint64_t)(&pd) + 3;
    for (int i = 1; i < 512; i++)
        pdpt.entries[i] = 0;

    // Prepare PD
    int i = 0;
    for (; i < NUM_PAGE_TABLES; i++)
        pd.entries[i] = (uint64_t)(&(pt[i])) + 3;
    for (; i < 512; i++)
        pd.entries[i] = 0;

    // Prepare PTs
    uint64_t addr = 3;
    for (int t = 0; t < NUM_PAGE_TABLES; t++) {
        for (i = 0; i < 512; i++) {
            pt[t].entries[i] = addr;
            addr += 0x1000;
        }
    }
}

void EnablePaging() { asm volatile("cli; movq %0, %%rax; movq %%rax, %%cr3" : : "b"((uint64_t)(&pml4)) : "eax"); }