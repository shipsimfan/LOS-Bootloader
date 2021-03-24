#include <file.h>

#include <systemTable.h>

EFI_GUID LIP_GUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID FS_GUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID GENERIC_FILE_GUID = EFI_FILE_INFO_ID;

EFI_MEMORY_TYPE allocationType;
EFI_FILE_HANDLE bootVolume = NULL;

EFI_STATUS LoadBootVolume() {
    EFI_LOADED_IMAGE* loadedImage;
    EFI_STATUS status = SYSTEM_TABLE->BootServices->HandleProtocol(IMAGE_HANDLE, &LIP_GUID, (void**)&loadedImage);
    if (EFI_ERROR(status))
        return status;

    EFI_FILE_IO_INTERFACE* ioVolume;
    status = SYSTEM_TABLE->BootServices->HandleProtocol(loadedImage->DeviceHandle, &FS_GUID, (void**)&ioVolume);
    if (EFI_ERROR(status))
        return status;

    status = ioVolume->OpenVolume(ioVolume, &bootVolume);
    if (EFI_ERROR(status))
        return status;

    allocationType = loadedImage->ImageDataType;

    return status;
}

EFI_STATUS Open(const CHAR16* name, EFI_FILE_HANDLE* fileHandle) { return bootVolume->Open(bootVolume, fileHandle, (CHAR16*)name, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM); }

EFI_STATUS Close(EFI_FILE_HANDLE handle) { return handle->Close(handle); }

BOOLEAN GrowBuffer(EFI_STATUS* status, VOID** buffer, UINTN bufferSize) {
    if (!*buffer && bufferSize)
        *status = EFI_BUFFER_TOO_SMALL;

    BOOLEAN tryAgain = FALSE;
    if (*status == EFI_BUFFER_TOO_SMALL) {
        if (*buffer)
            SYSTEM_TABLE->BootServices->FreePool(*buffer);

        *status = SYSTEM_TABLE->BootServices->AllocatePool(allocationType, bufferSize, buffer);

        if (*buffer)
            tryAgain = TRUE;
        else
            *status = EFI_OUT_OF_RESOURCES;
    }

    if (!tryAgain && EFI_ERROR(*status) && *buffer) {
        SYSTEM_TABLE->BootServices->FreePool(*buffer);
        *buffer = NULL;
    }

    return tryAgain;
}

EFI_STATUS Read(EFI_FILE_HANDLE handle, void** buffer, UINTN* size) {
    EFI_STATUS status = EFI_SUCCESS;
    EFI_FILE_INFO* fileInfo = NULL;
    UINTN fileInfoSize = sizeof(EFI_FILE_INFO) + 200;
    EFI_GUID genericFileInfo = EFI_FILE_INFO_ID;

    while (GrowBuffer(&status, (VOID**)&fileInfo, fileInfoSize))
        status = handle->GetInfo(handle, &genericFileInfo, &fileInfoSize, fileInfo);

    status = SYSTEM_TABLE->BootServices->AllocatePool(allocationType, fileInfo->FileSize, buffer);
    if (EFI_ERROR(status))
        return status;

    *size = fileInfo->FileSize;

    return handle->Read(handle, size, *buffer);
}

EFI_STATUS LoadFile(const CHAR16* name, void** buffer, UINTN* size) {
    EFI_STATUS status;

    if (bootVolume == NULL) {
        // Open boot volume
        status = LoadBootVolume();
        if (EFI_ERROR(status))
            return status;
    }

    // Open the file
    EFI_FILE_HANDLE fileHandle;
    status = Open(name, &fileHandle);
    if (EFI_ERROR(status))
        return status;

    // Load the file
    status = Read(fileHandle, buffer, size);
    if (EFI_ERROR(status))
        return status;

    // Close the file
    return Close(fileHandle);
}