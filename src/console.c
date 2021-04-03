#include <console.h>
#include <error.h>

#include <efi.h>
#include <stdarg.h>
#include <systemTable.h>

#define FLAGS_HASH (1U << 4U)
#define FLAGS_UPPERCASE (1U << 5U)
#define FLAGS_CHAR (1U << 6U)
#define FLAGS_SHORT (1U << 7U)
#define FLAGS_LONG (1U << 8U)
#define FLAGS_LONG_LONG (1U << 9U)

#define PRINTF_NTOA_BUFFER_SIZE 32

#define PRINTV(format, arg)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            \
    va_list arg;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       \
    va_start(arg, format);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             \
    Printv(format, arg);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    va_end(arg)

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

void Printv(const CHAR16* format, va_list arg) {
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

void Print(const CHAR16* format, ...) { PRINTV(format, arg); }

void Println(const CHAR16* format, ...) {
    PRINTV(format, arg);
    PutString(L"\r\n");
}

void FatalError(const CHAR16* format, ...) {
    SetColor(EFI_LIGHTRED);
    PutString(L"Fatal Error: ");
    SetColor(EFI_LIGHTGRAY);

    PRINTV(format, arg);

    while (1)
        asm volatile("hlt");
}

void SetColor(Color color) { SYSTEM_TABLE->ConOut->SetAttribute(SYSTEM_TABLE->ConOut, color); }

const CHAR16* StatusString(EFI_STATUS status) {
    switch (status) {
    case EFI_SUCCESS:
        return L"Success";

    case EFI_LOAD_ERROR:
        return L"Load Error";

    case EFI_INVALID_PARAMETER:
        return L"Invalid Parameter";

    case EFI_UNSUPPORTED:
        return L"Unsupported";

    case EFI_BAD_BUFFER_SIZE:
        return L"Bad Buffer Size";

    case EFI_BUFFER_TOO_SMALL:
        return L"Buffer too Small";

    case EFI_NOT_READY:
        return L"Not Ready";

    case EFI_WRITE_PROTECTED:
        return L"Write Protected";

    case EFI_OUT_OF_RESOURCES:
        return L"Out of Resources";

    case EFI_VOLUME_CORRUPTED:
        return L"Volume Corrupted";

    case EFI_VOLUME_FULL:
        return L"Volume Full";

    case EFI_NO_MEDIA:
        return L"No Media";

    case EFI_MEDIA_CHANGED:
        return L"Media Changed";

    case EFI_NOT_FOUND:
        return L"Not Found";

    case EFI_ACCESS_DENIED:
        return L"Access Denied";

    case EFI_NO_RESPONSE:
        return L"No Response";

    case EFI_NO_MAPPING:
        return L"No Mapping";

    case EFI_TIMEOUT:
        return L"Timeout";

    case EFI_NOT_STARTED:
        return L"Not Started";

    case EFI_ALREADY_STARTED:
        return L"Already Started";

    case EFI_ABORTED:
        return L"Aborted";

    case EFI_ICMP_ERROR:
        return L"ICMP Error";

    case EFI_TFTP_ERROR:
        return L"TFTP Error";

    case EFI_PROTOCOL_ERROR:
        return L"Protocol Error";

    case EFI_INCOMPATIBLE_VERSION:
        return L"Incompatible Version";

    case EFI_SECURITY_VIOLATION:
        return L"Security Violation";

    case EFI_CRC_ERROR:
        return L"CRC Error";

    case EFI_END_OF_MEDIA:
        return L"End of Media";

    case EFI_END_OF_FILE:
        return L"End of File";

    case EFI_INVALID_LANGUAGE:
        return L"Invalid Language";

    case EFI_COMPROMISED_DATA:
        return L"Compromised Data";

    case ELF_INCORRECT_MAG:
        return L"Incorrect MAG";

    case ELF_INVALID_CLASS:
        return L"Invalid Class";

    case ELF_INVALID_DATA:
        return L"Invalid Data";

    case ELF_INVALID_VERSION:
        return L"Invalid Version";

    case ELF_INVALID_TYPE:
        return L"Invalid Type";

    case ELF_INVALID_MACHINE:
        return L"Invalid Machine";

    default:
        return L"Unknown Error";
    }
}