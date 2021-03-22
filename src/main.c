#include <file.h>
#include <systemTable.h>
#include <video.h>

EFI_SYSTEM_TABLE* SYSTEM_TABLE;
EFI_HANDLE IMAGE_HANDLE;

EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    // Disable watchdog timer
    systemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

    // Save system table and image handle
    SYSTEM_TABLE = systemTable;
    IMAGE_HANDLE = imageHandle;

    // Clear the screen
    systemTable->ConOut->ClearScreen(systemTable->ConOut);

    printf(L"Loading the kernel . . .\r\n");

    // Load the kernel
    void* kernel;
    UINTN kernelSize;
    EFI_STATUS status = LoadFile(L"kernl.elf", &kernel, &kernelSize);
    if (EFI_ERROR(status)) {
        eprintf(L"Failed to load kernel! (%i)\r\n", status);
        goto hang;
    }

    printf(L"Kernel Loaded!\r\n");

hang:
    while (1)
        asm volatile("hlt");
}