#include <video.h>

#include <systemTable.h>

EFI_GUID gopGUID = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

EFI_STATUS GetCurrentGraphicsInfo(GraphicsMode* modeInfo) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;

    EFI_STATUS status = SYSTEM_TABLE->BootServices->LocateProtocol(&gopGUID, NULL, (void**)&gop);
    if (EFI_ERROR(status))
        return status;

    modeInfo->horizontalResolution = gop->Mode->Info->HorizontalResolution;
    modeInfo->verticalResolution = gop->Mode->Info->VerticalResolution;
    modeInfo->pixelFormat = gop->Mode->Info->PixelFormat;
    modeInfo->redMask = gop->Mode->Info->PixelInformation.RedMask;
    modeInfo->blueMask = gop->Mode->Info->PixelInformation.BlueMask;
    modeInfo->greenMask = gop->Mode->Info->PixelInformation.GreenMask;
    modeInfo->pixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;
    modeInfo->framebuffer = gop->Mode->FrameBufferBase;
    modeInfo->framebufferSize = gop->Mode->FrameBufferSize;

    return status;
}