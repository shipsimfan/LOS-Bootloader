#pragma once

#include <efi.h>

int Print(CHAR16* str);
int PrintHex(UINT64 val);

EFI_STATUS GetCurrentVideoModeInfo(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE** info);