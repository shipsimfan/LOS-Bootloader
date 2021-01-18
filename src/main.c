#include <elf.h>
#include <file.h>
#include <paging.h>
#include <video.h>

EFI_SYSTEM_TABLE* ST;

typedef void (*KernelStartFunc)(uint64_t, uint64_t) __attribute__((sysv_abi));

typedef struct {
    UINTN size;
    UINTN key;
    UINTN descSize;
    UINT32 descVersion;
    VOID* map;
} MemoryMap;

EFI_STATUS GetMemoryMap(MemoryMap* map) {
    EFI_STATUS status = ST->BootServices->GetMemoryMap(&(map->size), map->map, &(map->key), &(map->descSize), &(map->descVersion));
    if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL)
        return status;

    status = ST->BootServices->AllocatePool(EfiLoaderData, map->size, &(map->map));
    if (EFI_ERROR(status))
        return status;

    return status = ST->BootServices->GetMemoryMap(&(map->size), map->map, &(map->key), &(map->descSize), &(map->descVersion));
}

EFI_STATUS LoadKernel(EFI_HANDLE imageHandle, void** kernelFile, UINTN* kernelFileSize) {
    // Load the boot volume
    Print(L"Opening volume . . . ");
    EFI_FILE_HANDLE volume;
    EFI_STATUS status = LoadVolume(imageHandle, &volume);
    if (EFI_ERROR(status)) {
        Print(L"Failed to open the volume\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    // Open the kernel
    Print(L"Opening kernel file . . . ");
    EFI_FILE_HANDLE kernelHandle;
    status = Open(L"kernel.elf", volume, &kernelHandle);
    if (EFI_ERROR(status)) {
        Print(L"Failed to open the kernel file\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    // Load the kernel
    Print(L"Reading kernel file . . . ");
    status = Read(kernelHandle, kernelFile, kernelFileSize);
    if (EFI_ERROR(status)) {
        Print(L"Failed to read the kernel file\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    // Close the kernel handle
    return Close(kernelHandle);
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    // Save the system table
    ST = systemTable;

    // Turn off the watchdog
    ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    // Load the kernel
    void* kernelFile;
    UINTN kernelFileSize;
    EFI_STATUS status = LoadKernel(imageHandle, &kernelFile, &kernelFileSize);
    if (EFI_ERROR(status))
        return status;

    // Load the ELF sections
    Elf64_Ehdr* kernelHeader = (Elf64_Ehdr*)kernelFile;
    Print(L"Loading kernel executable . . . ");
    int s = LoadELFExecutable(kernelHeader);
    if (s < 0) {
        Print(L"Failed to load executable\r\n");
        return s;
    } else
        Print(L"OK\r\n");

    // Save the entry point
    KernelStartFunc kernelStart = (KernelStartFunc)kernelHeader->e_entry;

    // Free the file
    ST->BootServices->FreePool(kernelFile);

    // Get Video information
    Print(L"Getting the current video mode . . . ");
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE* mode;
    status = GetCurrentVideoModeInfo(&mode);
    if (EFI_ERROR(status)) {
        Print(L"Failed to get current video mode\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    // Prepare paging
    PreparePaging();

    // Get the memory map
    Print(L"Getting the memory map . . . ");
    MemoryMap mmap;
    status = GetMemoryMap(&mmap);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get memory map\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    Print(L"Exiting boot services . . .");

    ST->BootServices->ExitBootServices(imageHandle, mmap.key);

    // EnablePaging(); //TODO: Get paging working

    kernelStart(mode->FrameBufferBase, mode->FrameBufferSize);

    while (1)
        ;

    return EFI_SUCCESS;
}