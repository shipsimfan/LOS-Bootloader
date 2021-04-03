#pragma once

#include <efi.h>

typedef UINTN Color;

void Print(const CHAR16* format, ...);
void Println(const CHAR16* format, ...);

void SetColor(Color color);