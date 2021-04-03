#include <console.h>
#include <elf.h>
#include <error.h>
#include <file.h>
#include <systemTable.h>
#include <video.h>

#define PRINT_OK()                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     \
    SetColor(EFI_GREEN);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    Println(L"OK");                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    \
    SetColor(EFI_LIGHTGRAY);

typedef void (*KernelEntry)(GraphicsMode* graphicsInfo) __attribute__((sysv_abi));
GraphicsMode graphicsMode;

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

    // Load the kernel
    Print(L"Loading the kernel . . . ");
    void* kernel = NULL;
    UINTN kernelSize = 0;
    EFI_STATUS status = LoadFile(L"kernel.elf", &kernel, &kernelSize);
    if (EFI_ERROR(status))
        FatalError(L"\r\nFailed to load kernel file: %s", StatusString(status));

    KernelEntry entry;
    status = LoadELFExecutable(kernel, (UINT64*)&entry);
    if (EFI_ERROR(status))
        FatalError(L"\r\nFailed to load kernel: %s", StatusString(status));

    SYSTEM_TABLE->BootServices->FreePool(kernel);
    PRINT_OK();

    // Get the graphics mode info
    Print(L"Getting video mode information . . . ");
    status = GetCurrentGraphicsInfo(&graphicsMode);
    if (EFI_ERROR(status))
        FatalError(L"\r\nFailed to get video mode information: %s", StatusString(status));
    PRINT_OK();

    // Exit boot services and launch the kernel
    Println(L"Launching the kernel . . .");

    entry(&graphicsMode);

    while (1)
        asm volatile("hlt");
}