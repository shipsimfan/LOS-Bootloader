#include <file.h>

#include <video.h>

EFI_MEMORY_TYPE allocationType;

EFI_STATUS LoadVolume(EFI_HANDLE image, EFI_FILE_HANDLE* volume) {
    EFI_GUID lipGUID = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGUID = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE* IOVolume;
    EFI_LOADED_IMAGE* loadedImage;
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

    allocationType = loadedImage->ImageDataType;

    return status;
}

EFI_STATUS Open(CHAR16* filename, EFI_FILE_HANDLE volume, EFI_FILE_HANDLE* fileHandle) { return volume->Open(volume, fileHandle, filename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM); }

BOOLEAN GrowBuffer(EFI_STATUS* status, VOID** buffer, UINTN bufferSize) {
    if (!*buffer && bufferSize)
        *status = EFI_BUFFER_TOO_SMALL;

    BOOLEAN tryAgain = FALSE;
    if (*status == EFI_BUFFER_TOO_SMALL) {
        if (*buffer)
            ST->BootServices->FreePool(*buffer);

        *status = ST->BootServices->AllocatePool(allocationType, bufferSize, buffer);

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

EFI_STATUS Read(EFI_FILE_HANDLE fileHandle, VOID** buffer, UINTN* bufferSize) {
    EFI_STATUS status = EFI_SUCCESS;
    EFI_FILE_INFO* fileInfo = NULL;
    UINTN fileInfoSize = sizeof(EFI_FILE_INFO) + 200;
    EFI_GUID genericFileInfo = EFI_FILE_INFO_ID;

    while (GrowBuffer(&status, (VOID**)&fileInfo, fileInfoSize))
        status = fileHandle->GetInfo(fileHandle, &genericFileInfo, &fileInfoSize, fileInfo);

    status = ST->BootServices->AllocatePool(allocationType, fileInfo->FileSize, buffer);
    if (EFI_ERROR(status))
        return status;

    return status = fileHandle->Read(fileHandle, &fileInfo->FileSize, *buffer);
}

EFI_STATUS Close(EFI_FILE_HANDLE fileHandle) { return fileHandle->Close(fileHandle); }