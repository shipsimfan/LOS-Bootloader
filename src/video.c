#include <video.h>

extern EFI_SYSTEM_TABLE* ST;

const CHAR16* digits = L"0123456789ABCDEF";

EFI_STATUS GetCurrentVideoModeInfo(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE** info) {
    EFI_GUID gopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

    EFI_STATUS status = ST->BootServices->LocateProtocol(&gopGUID, NULL, (void**)&gop);
    if (EFI_ERROR(status))
        return status;

    *info = gop->Mode;

    return status;
}

int Print(CHAR16* str) {
    EFI_STATUS s = ST->ConOut->OutputString(ST->ConOut, str);
    return EFI_ERROR(s) ? -1 : 0;
}

int PrintHex(UINT64 val) {
    CHAR16 num[19];
    num[0] = '0';
    num[1] = 'x';

    for (int i = 17; i > 1; i--) {
        num[i] = digits[val & 0xF];
        val = val >> 4;
    }

    num[18] = 0;

    Print(num);
}