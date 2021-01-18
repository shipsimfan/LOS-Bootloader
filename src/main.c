#include <efi.h>
#include <elf.h>

EFI_SYSTEM_TABLE* ST;
EFI_FILE_HANDLE bootVolume;
EFI_MEMORY_TYPE poolAllocationType;
EFI_LOADED_IMAGE* loadedImage = NULL;

typedef void (*KernelStartFunc)(uint64_t, uint64_t) __attribute__((sysv_abi));

int Print(CHAR16* str) {
    EFI_STATUS s = ST->ConOut->OutputString(ST->ConOut, str);
    return EFI_ERROR(s) ? -1 : 0;
}

EFI_STATUS GetVolume(EFI_HANDLE image, EFI_FILE_HANDLE* volume) {
    EFI_GUID lipGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE* IOVolume;
    EFI_STATUS status;

    status = ST->BootServices->HandleProtocol(image, &lipGUID, (void**)&loadedImage);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get image handle!\r\n");
        return status;
    }

    status = ST->BootServices->HandleProtocol(loadedImage->DeviceHandle, &fsGUID, (VOID*)&IOVolume);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get device handle!\r\n");
        return status;
    }

    status = IOVolume->OpenVolume(IOVolume, volume);
    if (EFI_ERROR(status))
        Print(L"Unable to get volume!\r\n");

    return status;
}

EFI_STATUS Open(CHAR16* filename, EFI_FILE_HANDLE* fileHandle) { return bootVolume->Open(bootVolume, fileHandle, filename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM); }

EFI_STATUS Close(EFI_FILE_HANDLE fileHandle) { return fileHandle->Close(fileHandle); }

BOOLEAN GrowBuffer(EFI_STATUS* status, VOID** buffer, UINTN bufferSize) {
    BOOLEAN tryAgain;
    if (!*buffer && bufferSize)
        *status = EFI_BUFFER_TOO_SMALL;

    tryAgain = FALSE;
    if (*status == EFI_BUFFER_TOO_SMALL) {
        if (*buffer)
            ST->BootServices->FreePool(*buffer);

        *status = ST->BootServices->AllocatePool(poolAllocationType, bufferSize, buffer);

        if (*buffer)
            tryAgain = TRUE;
        else
            *status = EFI_OUT_OF_RESOURCES;
    }

    if (!tryAgain && EFI_ERROR(*status) && *buffer) {
        ST->BootServices->FreePool(*buffer);
        *buffer = NULL;
    }

    return tryAgain;
}

EFI_FILE_INFO* GetFileInfo(EFI_FILE_HANDLE fileHandle) {
    EFI_STATUS status = EFI_SUCCESS;
    EFI_FILE_INFO* buffer = NULL;
    UINTN bufferSize = sizeof(EFI_FILE_INFO) + 200;
    EFI_GUID genericFileInfo = EFI_FILE_INFO_ID;

    while (GrowBuffer(&status, (VOID**)&buffer, bufferSize))
        status = fileHandle->GetInfo(fileHandle, &genericFileInfo, &bufferSize, buffer);

    return buffer;
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    EFI_STATUS status;

    ST = systemTable;

    // Turn off the watchdog
    ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    // Load the boot volume
    Print(L"Opening boot volume . . . ");
    status = GetVolume(imageHandle, &bootVolume);
    if (EFI_ERROR(status)) {
        Print(L"Failed to open boot volume!\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    poolAllocationType = loadedImage->ImageDataType;

    // Open the kernel
    Print(L"Opening /kernel.elf . . . ");
    EFI_FILE_HANDLE kernelHandle;
    status = Open(L"kernel.elf", &kernelHandle);
    if (EFI_ERROR(status)) {
        Print(L"Failed to open kernel.elf\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    // Load the kernel
    Print(L"Reading kernel.elf . . . ");
    EFI_FILE_INFO* info = GetFileInfo(kernelHandle);

    UINT8* kernelFile;
    status = ST->BootServices->AllocatePool(poolAllocationType, info->FileSize, (VOID**)&kernelFile);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate pool for reading kernel\r\n");
        return status;
    }

    status = kernelHandle->Read(kernelHandle, &info->FileSize, kernelFile);
    if (EFI_ERROR(status)) {
        Print(L"Failed to read kernel.elf\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    // Close the kernel handle
    kernelHandle->Close(kernelHandle);

    // Parse the ELF headers
    Elf64_Ehdr* kernelHeader = (Elf64_Ehdr*)kernelFile;

    // Check ELF MAG
    if (kernelHeader->e_ident[EI_MAG0] != ELFMAG0 || kernelHeader->e_ident[EI_MAG1] != ELFMAG1 || kernelHeader->e_ident[EI_MAG2] != ELFMAG2 || kernelHeader->e_ident[EI_MAG3] != ELFMAG3) {
        Print(L"kernel.elf has an incorrect ELF MAG\r\n");
        return -1;
    }

    // Verify class
    if (kernelHeader->e_ident[EI_CLASS] != ELFCLASS64) {
        Print(L"kernel.elf is not a 64-bit file\r\n");
        return -1;
    }

    // Verify data order
    if (kernelHeader->e_ident[EI_DATA] != ELFDATA2LSB) {
        Print(L"kernel.elf is not lsb\r\n");
        return -1;
    }

    // Verify version
    if (kernelHeader->e_ident[EI_VERSION] == 0) {
        Print(L"kernel.elf has an incorrect ELF version\r\n");
        return -1;
    }

    // Verify type
    if (kernelHeader->e_type != ET_EXEC) {
        Print(L"kernel.elf is not an executable\r\n");
        return -1;
    }

    // Verify machine
    if (kernelHeader->e_machine != EM_AMD64) {
        Print(L"kernel.elf was not built for AMD64\r\n");
        return -1;
    }

    // Verify version again
    if (kernelHeader->e_version == 0) {
        Print(L"kernel.elf has an incorrect ELF version\r\n");
        return -1;
    }

    Print(L"kernel.elf is a valid ELF file\r\n");

    // Load the kernel
    Elf64_Phdr* phdr = (Elf64_Phdr*)((Elf64_Off)kernelHeader + kernelHeader->e_phoff);
    for (int i = 0; i < kernelHeader->e_phnum; i++, phdr = (Elf64_Phdr*)((Elf64_Xword)phdr + kernelHeader->e_phentsize)) {
        if (phdr->p_type == PT_LOAD) {
            status = ST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, EFI_SIZE_TO_PAGES(phdr->p_memsz), (EFI_PHYSICAL_ADDRESS*)phdr->p_paddr);
            if (EFI_ERROR(status)) {
                Print(L"Failed to allocate pages!\r\n");
                return -1;
            }

            if (phdr->p_filesz > 0)
                ST->BootServices->CopyMem((void*)phdr->p_paddr, (void*)((Elf64_Xword)kernelHeader + phdr->p_offset), phdr->p_filesz);

            Print(L"Section loaded\r\n");
        }
    }

    KernelStartFunc kernelStart = (KernelStartFunc)kernelHeader->e_entry;

    ST->BootServices->FreePool(kernelFile);

    Print(L"Kernel Loaded!\r\n");

    // Get Video information
    EFI_GUID gopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

    status = ST->BootServices->LocateProtocol(&gopGUID, NULL, (void**)&gop);
    if (EFI_ERROR(status)) {
        Print(L"Unable to locate GOP!\r\n");
        return -1;
    }

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* gopInfo;
    UINTN infoSize, numModes, nativeMode;

    status = gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &infoSize, &gopInfo);
    if (status == EFI_NOT_STARTED)
        status = gop->SetMode(gop, 0);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get video mode info");
        return -1;
    }

    // Get the memory map
    UINTN memoryMapSize;
    UINTN memoryMapKey;
    UINTN descriptorSize;
    UINT32 descriptorVersion;
    VOID* memoryMap;

    status = ST->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);
    if (EFI_ERROR(status) && status != EFI_BUFFER_TOO_SMALL) {
        Print(L"Unable to get initial memory map\r\n");
        return -1;
    }

    status = ST->BootServices->AllocatePool(EfiLoaderData, memoryMapSize, &memoryMap);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get memory map pool\r\n");
        return -1;
    }

    status = ST->BootServices->GetMemoryMap(&memoryMapSize, memoryMap, &memoryMapKey, &descriptorSize, &descriptorVersion);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get memory map\r\n");
        return -1;
    }

    Print(L"Test");

    ST->BootServices->ExitBootServices(imageHandle, memoryMapKey);

    kernelStart(gop->Mode->FrameBufferBase, gop->Mode->FrameBufferSize);

    while (1)
        ;

    return EFI_SUCCESS;
}