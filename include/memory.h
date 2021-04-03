#pragma once

#include <efi.h>

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint64_t size;
    uint64_t key;
    uint64_t descSize;
    uint32_t descVersion;
    uint64_t mapAddr;
} MemoryMap;

#pragma pack(pop)

EFI_STATUS GetMemoryMap(MemoryMap* map);