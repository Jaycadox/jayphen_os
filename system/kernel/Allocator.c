#pragma once
#include "../elf_libc.c"

#define PAGE_SIZE 4096

struct Allocator {
    struct MemoryLayout *MemoryLayout;
    usize                TotalPages;
};
static struct Allocator gAllocator = {0};

struct FreeRun {
    usize           NumberOfPages;
    struct FreeRun *Next;
};

static struct FreeRun *FreeList = {0};

usize PageIndexToMemoryAddress(usize PageIndex) {
    PageIndex += 1; // Don't allow access to the first page
    if (PageIndex >= gAllocator.TotalPages) {
        Panic("Page index (%zu) exceeds total number of pages (%zu)", PageIndex, gAllocator.TotalPages);
    }

    usize RegionIndex = 0;

    for (;;) {
        struct MemoryRegion *Region = &gAllocator.MemoryLayout->ConventionalRegions[RegionIndex];
        if (PageIndex < Region->NumPages) {
            return Region->Start + (PageIndex * PAGE_SIZE);
        }

        PageIndex -= Region->NumPages;
        ++RegionIndex;
    }
}

usize MemoryAddressToPageIndex(usize Address) {
    usize PageIndex   = 0;
    usize RegionIndex = 0;

    for (;;) {
        struct MemoryRegion *Region = &gAllocator.MemoryLayout->ConventionalRegions[RegionIndex];

        usize RegionStart = Region->Start;
        usize RegionEnd   = RegionStart + (Region->NumPages * PAGE_SIZE);

        if (Address >= RegionStart && Address < RegionEnd) {
            usize Offset = Address - RegionStart;
            return (PageIndex + (Offset / PAGE_SIZE)) - 1; // Don't allow access to the first page
        }

        PageIndex += Region->NumPages;
        RegionIndex++;
    }
}

void InitializeAllocator(struct MemoryLayout *MemoryLayout) {
    gAllocator.MemoryLayout = MemoryLayout;
    gAllocator.TotalPages   = 0;

    for (u32 i = 0; i < MemoryLayout->NumConventionalRegions; ++i) {
        gAllocator.TotalPages += MemoryLayout->ConventionalRegions[i].NumPages;
    }
    gAllocator.TotalPages -= 1; // Don't allow access to the first page

    PrintLinef("System memory: %zuMB (%zu pages from %zu regions)",
               (gAllocator.TotalPages * 4096) / 1048576,
               gAllocator.TotalPages,
               MemoryLayout->NumConventionalRegions);
    FreeList                = (struct FreeRun *) PageIndexToMemoryAddress(0);
    FreeList->NumberOfPages = gAllocator.TotalPages;
    FreeList->Next          = NULL;
}

void *AllocatePages(usize NumberOfPages) {
    NumberOfPages += 1; // Extra room for header page

    struct FreeRun **Link     = &FreeList;
    struct FreeRun  *Best     = NULL;
    struct FreeRun **BestLink = NULL;
    bool             Found    = false;

    for (; *Link; Link = &(*Link)->Next) {
        if ((*Link)->NumberOfPages >= NumberOfPages && (!Best || ((*Link)->NumberOfPages < Best->NumberOfPages))) {
            Best     = *Link;
            BestLink = Link;
            Found    = true;
        }
    }

    if (!Found) {
        Panic("Out of memory (attempted to allocate %zu pages)", NumberOfPages);
        return NULL;
    }

    *BestLink = Best->Next;

    if (Best->NumberOfPages > NumberOfPages) {
        // The run of pages allocated is too large, so we should split it
        struct FreeRun *LeftOver = (struct FreeRun *) (PageIndexToMemoryAddress(MemoryAddressToPageIndex((usize) Best) + NumberOfPages));
        LeftOver->NumberOfPages  = Best->NumberOfPages - NumberOfPages;

        struct FreeRun **Ins = &FreeList;
        while (*Ins && *Ins < LeftOver) {
            Ins = &(*Ins)->Next;
        }
        LeftOver->Next = *Ins;
        *Ins           = LeftOver;
    }
    Best->NumberOfPages = NumberOfPages;

    return ((char *) Best) + (1 * PAGE_SIZE);
}

void FreePages(void *PagesStart) {
    char *Start = PagesStart;
    Start -= (1 * PAGE_SIZE); // Previous page contains the header
    if ((usize) Start % PAGE_SIZE != 0) {
        Panic("FreePages(%p) not called on a page boundary.", Start);
    }

    struct FreeRun *Header      = (struct FreeRun *) Start;
    usize           HeaderIndex = MemoryAddressToPageIndex((usize) Header);

    struct FreeRun *Before  = NULL;
    struct FreeRun *Current = FreeList;
    while (Current && MemoryAddressToPageIndex((usize) Current) < HeaderIndex) {
        Before  = Current;
        Current = Current->Next;
    }
    struct FreeRun *After = Current;

    if (Before) {
        usize BeforeIndex = MemoryAddressToPageIndex((usize) Before);
        if (BeforeIndex + Before->NumberOfPages == HeaderIndex) {
            Before->NumberOfPages += Header->NumberOfPages;
            Header = Before;
        } else {
            Header->Next = After;
            Before->Next = Header;
        }
    } else {
        Header->Next = After;
        FreeList     = Header;
    }

    if (After) {
        HeaderIndex      = MemoryAddressToPageIndex((usize) Header);
        usize AfterIndex = MemoryAddressToPageIndex((usize) After);

        if (HeaderIndex + Header->NumberOfPages == AfterIndex) {
            Header->NumberOfPages += After->NumberOfPages;
            Header->Next = After->Next;
        }
    }
}
