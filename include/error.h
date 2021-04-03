#pragma once

#include <efi.h>

#define ELF_INCORRECT_MAG EFIERR(256)
#define ELF_INVALID_CLASS EFIERR(257)
#define ELF_INVALID_DATA EFIERR(258)
#define ELF_INVALID_VERSION EFIERR(259)
#define ELF_INVALID_TYPE EFIERR(260)
#define ELF_INVALID_MACHINE EFIERR(261)

const CHAR16* StatusString(EFI_STATUS status);

void FatalError(const CHAR16* format, ...);