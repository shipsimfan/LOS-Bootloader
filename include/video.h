#pragma once

#include <efi.h>
#include <stdarg.h>

void printf(const CHAR16* format, ...);
void vprintf(const CHAR16* format, va_list arg);

void eprintf(const CHAR16* format, ...);
void evprintf(const CHAR16* format, va_list arg);