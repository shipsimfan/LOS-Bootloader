#include <video.h>

#include <systemTable.h>

#define FLAGS_HASH (1U << 4U)
#define FLAGS_UPPERCASE (1U << 5U)
#define FLAGS_CHAR (1U << 6U)
#define FLAGS_SHORT (1U << 7U)
#define FLAGS_LONG (1U << 8U)
#define FLAGS_LONG_LONG (1U << 9U)

#define PRINTF_NTOA_BUFFER_SIZE 32

void PutString(const CHAR16* str) { SYSTEM_TABLE->ConOut->OutputString(SYSTEM_TABLE->ConOut, (CHAR16*)str); }

void PutCharacter(const CHAR16 c) {
    CHAR16 str[2] = {c, 0};
    PutString(str);
}

void ntoa(UINT64 value, BOOLEAN negative, unsigned long long base, unsigned flags) {
    CHAR16 buf[PRINTF_NTOA_BUFFER_SIZE];
    UINTN len = 0;

    if (!value)
        flags &= ~FLAGS_HASH;

    do {
        const char digit = (char)(value % base);
        buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
        value /= base;
    } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));

    if (flags & FLAGS_HASH) {
        if ((base == 16) && (len < PRINTF_NTOA_BUFFER_SIZE))
            buf[len++] = 'x';
        else if ((base == 2) && (len < PRINTF_NTOA_BUFFER_SIZE))
            buf[len++] = 'b';

        if (len < PRINTF_NTOA_BUFFER_SIZE)
            buf[len++] = '0';
    }

    if (len < PRINTF_NTOA_BUFFER_SIZE && negative)
        buf[len++] = '-';

    while (len)
        PutCharacter(buf[--len]);
}

void printf(const CHAR16* format, ...) {
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

void vprintf(const CHAR16* format, va_list arg) {
    while (*format) {
        if (*format != '%') {
            PutCharacter(*format);
            format++;
            continue;
        }

        format++;

        // Parse flags
        unsigned flags = 0;
        if (*format == '#') {
            flags |= FLAGS_HASH;
            format++;
        }

        // Parse length
        switch (*format) {
        case 'l':
            flags |= FLAGS_LONG;
            format++;
            if (*format == 'l') {
                flags |= FLAGS_LONG_LONG;
                format++;
            }
            break;

        case 'h':
            flags |= FLAGS_SHORT;
            format++;
            if (*format == 'h') {
                flags |= FLAGS_CHAR;
                format++;
            }
            break;
        case 't':
            flags |= FLAGS_LONG;
            format++;
            break;

        case 'j':
            flags |= FLAGS_LONG;
            format++;
            break;

        case 'z':
            flags |= FLAGS_LONG;
            format++;
            break;

        default:
            break;
        }

        switch (*format) {
        case '%':
            PutCharacter(*format);
            format++;
            break;

        case 'd':
        case 'i':
        case 'u':
        case 'x':
        case 'X':
        case 'o':
        case 'b': {
            unsigned int base;
            if (*format == 'x' || *format == 'X')
                base = 16;
            else if (*format == 'o')
                base = 8;
            else if (*format == 'b')
                base = 2;
            else {
                base = 10;
                flags &= ~FLAGS_HASH;
            }

            if (*format == 'X')
                flags |= FLAGS_UPPERCASE;

            if (*format == 'i' || *format == 'd') {
                if (flags & FLAGS_LONG_LONG) {
                    const long long value = va_arg(arg, long long);
                    ntoa((unsigned long long)(value > 0 ? value : 0 - value), value < 0, base, flags);
                } else if (flags & FLAGS_LONG) {
                    const long value = va_arg(arg, long);
                    ntoa((unsigned long)(value > 0 ? value : 0 - value), value < 0, base, flags);
                } else {
                    const int value = (flags & FLAGS_CHAR) ? (char)va_arg(arg, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(arg, int) : va_arg(arg, int);
                    ntoa((unsigned int)(value > 0 ? value : 0 - value), value < 0, base, flags);
                }
            } else {
                if (flags & FLAGS_LONG_LONG)
                    ntoa(va_arg(arg, unsigned long long), 0, base, flags);
                else if (flags & FLAGS_LONG)
                    ntoa(va_arg(arg, unsigned long), 0, base, flags);
                else {
                    const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(arg, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(arg, int) : va_arg(arg, int);
                    ntoa(value, 0, base, flags);
                }
            }

            format++;
            break;
        }

        case 'c':
            PutCharacter((CHAR16)va_arg(arg, int));
            format++;
            break;

        case 's':
            PutString(va_arg(arg, CHAR16*));
            format++;
            break;

        default:
            break;
        }
    }
}

void eprintf(const CHAR16* format, ...) {
    va_list arg;
    va_start(arg, format);
    evprintf(format, arg);
    va_end(arg);
}

void evprintf(const CHAR16* format, va_list arg) {
    SYSTEM_TABLE->ConOut->SetAttribute(SYSTEM_TABLE->ConOut, EFI_BRIGHT | EFI_RED);
    SYSTEM_TABLE->ConOut->OutputString(SYSTEM_TABLE->ConOut, L"ERROR: ");
    SYSTEM_TABLE->ConOut->SetAttribute(SYSTEM_TABLE->ConOut, EFI_LIGHTGRAY);
    vprintf(format, arg);
}