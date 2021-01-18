#include <efi.h>

EFI_SYSTEM_TABLE* ST;
EFI_FILE_HANDLE bootVolume;

int Print(CHAR16* str) {
    EFI_STATUS s = ST->ConOut->OutputString(ST->ConOut, str);
    return EFI_ERROR(s) ? -1 : 0;
}

EFI_STATUS GetVolume(EFI_HANDLE image, EFI_FILE_HANDLE* volume) {
    EFI_LOADED_IMAGE* loaded_image = NULL;
    EFI_GUID lipGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE* IOVolume;
    EFI_STATUS status;

    status = ST->BootServices->HandleProtocol(image, &lipGUID, (void**)&loaded_image);
    if (EFI_ERROR(status)) {
        Print(L"Unable to get image handle!\r\n");
        return status;
    }

    status = ST->BootServices->HandleProtocol(loaded_image->DeviceHandle, &fsGUID, (VOID*)&IOVolume);
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

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    EFI_STATUS status;

    ST = systemTable;

    // Turn off the watchdog
    ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    Print(L"Opening boot volume . . . ");
    status = GetVolume(imageHandle, &bootVolume);
    if (EFI_ERROR(status)) {
        Print(L"Failed to open boot volume!\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    Print(L"Opening /kernel.elf . . . ");
    EFI_FILE_HANDLE kernelHandle;
    status = Open(L"kernel.elf", &kernelHandle);
    if (EFI_ERROR(status)) {
        Print(L"Failed to open kernel.elf\r\n");
        return status;
    } else
        Print(L"OK\r\n");

    while (1)
        ;

    return EFI_SUCCESS;
}