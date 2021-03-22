#pragma once

#include <efi.h>

EFI_STATUS LoadFile(const CHAR16* path, void** buffer, UINTN* size);