#pragma once

#include <efi.h>

extern EFI_SYSTEM_TABLE* ST;

EFI_STATUS LoadVolume(EFI_HANDLE image, EFI_FILE_HANDLE* volume);
EFI_STATUS Open(CHAR16* filename, EFI_FILE_HANDLE volume, EFI_FILE_HANDLE* fileHandle);
EFI_STATUS Read(EFI_FILE_HANDLE fileHandle, VOID** buffer, UINTN* bufferSize);
EFI_STATUS Close(EFI_FILE_HANDLE fileHandle);