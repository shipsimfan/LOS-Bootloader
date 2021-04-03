#include <memory.h>

#include <systemTable.h>

EFI_STATUS GetMemoryMap(MemoryMap* map) {
    EFI_STATUS status = SYSTEM_TABLE->BootServices->GetMemoryMap(&(map->size), (EFI_MEMORY_DESCRIPTOR*)map->mapAddr, &(map->key), &(map->descSize), &(map->descVersion));
    if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL)
        return status;

    status = SYSTEM_TABLE->BootServices->AllocatePool(EfiLoaderData, map->size, (void**)&(map->mapAddr));
    if (EFI_ERROR(status))
        return status;

    return SYSTEM_TABLE->BootServices->GetMemoryMap(&(map->size), (EFI_MEMORY_DESCRIPTOR*)map->mapAddr, &(map->key), &(map->descSize), &(map->descVersion));
}