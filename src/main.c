#include <elf.h>
#include <file.h>
#include <systemTable.h>
#include <video.h>

typedef void (*KernelEntry)();

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
    void* kernel = NULL;
    UINTN kernelSize = 0;
    EFI_STATUS status = LoadFile(L"kernel.elf", &kernel, &kernelSize);
    if (EFI_ERROR(status)) {
        eprintf(L"Failed to load kernel file! (%i)\r\n", status);
        goto hang;
    }

    KernelEntry entry = (KernelEntry)LoadELFExecutable(kernel);
    if (entry == 0) {
        eprintf(L"Failed to load kernel!\r\n");
        goto hang;
    }

    SYSTEM_TABLE->BootServices->FreePool(kernel);
    printf(L"Kernel loaded!\r\n");

    // Exit boot services and launch the kernel
    printf(L"Launching the kernel . . .\r\n");

    entry();

hang:
    while (1)
        asm volatile("hlt");
}