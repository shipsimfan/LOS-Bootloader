#include <efi.h>

EFI_SYSTEM_TABLE* ST;
EFI_FILE_HANDLE bootVolume;
EFI_MEMORY_TYPE poolAllocationType;
EFI_LOADED_IMAGE* loadedImage = NULL;

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

    UINT8* buffer;
    status = ST->BootServices->AllocatePool(poolAllocationType, info->FileSize, (VOID**)&buffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to allocate pool for reading kernel\r\n");
        return status;
    }

    status = kernelHandle->Read(kernelHandle, &info->FileSize, buffer);
    if (EFI_ERROR(status)) {
        Print(L"Failed to read kernel.elf\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    // Close the kernel handle
    kernelHandle->Close(kernelHandle);

    // Parse the ELF headers

    while (1)
        ;

    return EFI_SUCCESS;
}