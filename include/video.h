#pragma once

#include <efi.h>

#pragma pack(push)
#pragma pack(1)

typedef struct {
    uint32_t horizontalResolution;
    uint32_t verticalResolution;
    uint32_t pixelFormat;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t pixelsPerScanline;
    uint64_t framebuffer;
    uint64_t framebufferSize;
} GraphicsMode;

#pragma pack(pop)

EFI_STATUS GetCurrentGraphicsInfo(GraphicsMode* modeInfo);
