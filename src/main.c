#include <efi.h>

EFI_SYSTEM_TABLE* ST;

int Print(CHAR16* str) {
    EFI_STATUS s = ST->ConOut->OutputString(ST->ConOut, str);
    return EFI_ERROR(s) ? -1 : 0;
}

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    ST = systemTable;

    // Turn off the watchdog
    ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    Print(L"Booting . . .\r\n");

    while (1)
        ;

    return EFI_SUCCESS;
}