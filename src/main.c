#include <elf.h>
#include <file.h>
#include <video.h>

EFI_SYSTEM_TABLE* ST;

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint64_t size;
    uint64_t key;
    uint64_t descSize;
    uint32_t descVersion;
    uint32_t reserved;
    uint64_t mapAddr;
} MemoryMap_t;

typedef struct {
    uint32_t horizontalResolution;
    uint32_t verticalResolution;
    uint32_t pixelFormat;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t reserved;
    uint32_t pixelsPerScanline;
    uint64_t frameBufferBase;
    uint64_t frameBufferSize;
} GraphicsMode_t;

#pragma pack(pop)

typedef void (*KernelStartFunc)(MemoryMap_t*, GraphicsMode_t*, void*) __attribute__((sysv_abi));

MemoryMap_t mmap;
GraphicsMode_t gmode;

EFI_STATUS GetMemoryMap(MemoryMap_t* map) {
    EFI_STATUS status = ST->BootServices->GetMemoryMap(&(map->size), (EFI_MEMORY_DESCRIPTOR*)map->mapAddr, &(map->key), &(map->descSize), &(map->descVersion));
    if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL)
        return status;

    status = ST->BootServices->AllocatePool(EfiLoaderData, map->size, (void**)&(map->mapAddr));
    if (EFI_ERROR(status))
        return status;

    return status = ST->BootServices->GetMemoryMap(&(map->size), (EFI_MEMORY_DESCRIPTOR*)map->mapAddr, &(map->key), &(map->descSize), &(map->descVersion));
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

int MemoryCompare(const void* str1, const void* str2, uint64_t count) {
    register const unsigned char* s1 = (const unsigned char*)str1;
    register const unsigned char* s2 = (const unsigned char*)str2;

    while (count-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }

    return 0;
}

EFI_STATUS
efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
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

    gmode.horizontalResolution = mode->Info->HorizontalResolution;
    gmode.verticalResolution = mode->Info->VerticalResolution;
    gmode.pixelFormat = mode->Info->PixelFormat;
    gmode.redMask = mode->Info->PixelInformation.RedMask;
    gmode.blueMask = mode->Info->PixelInformation.BlueMask;
    gmode.greenMask = mode->Info->PixelInformation.GreenMask;
    gmode.pixelsPerScanline = mode->Info->PixelsPerScanLine;
    gmode.frameBufferBase = mode->FrameBufferBase;
    gmode.frameBufferSize = mode->FrameBufferSize;

    // Get ACPI location
    EFI_GUID acpi20GUID = {0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};
    EFI_CONFIGURATION_TABLE* ect = ST->ConfigurationTable;
    void* rdsp = NULL;
    for (int i = 0; i < ST->NumberOfTableEntries; i++) {
        if (MemoryCompare(&ect->VendorGuid, &acpi20GUID, sizeof(acpi20GUID))) {
            rdsp = ect->VendorTable;
            break;
        }
    }

    if (rdsp == NULL) {
        Print(L"Unable to find ACPI 2.0 table\r\n");
        return -1;
    }

    // Get the memory map
    Print(L"Getting the memory map . . . ");
    status = GetMemoryMap(&mmap);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get memory map\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    Print(L"Exiting boot services . . .");

    ST->BootServices->ExitBootServices(imageHandle, mmap.key);

    kernelStart(&mmap, &gmode, rdsp);

    while (1)
        ;

    return EFI_SUCCESS;
}