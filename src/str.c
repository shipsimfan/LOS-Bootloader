/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    str.c

Abstract:




Revision History

--*/

#include "lib.h"

INTN StrCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2)
// compare strings
{
    return RtStrCmp(s1, s2);
}

INTN StrnCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2, IN UINTN len)
// compare strings
{
    while (*s1 && len) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
        len -= 1;
    }

    return len ? *s1 - *s2 : 0;
}

INTN EFIAPI LibStubStriCmp(IN EFI_UNICODE_COLLATION_INTERFACE* This EFI_UNUSED, IN CHAR16* s1, IN CHAR16* s2) { return StrCmp(s1, s2); }

VOID EFIAPI LibStubStrLwrUpr(IN EFI_UNICODE_COLLATION_INTERFACE* This EFI_UNUSED, IN CHAR16* Str EFI_UNUSED) {}

BOOLEAN
MetaMatch(IN CHAR16* String, IN CHAR16* Pattern) {
    CHAR16 c, p, l;

    for (;;) {
        p = *Pattern;
        Pattern += 1;

        switch (p) {
        case 0:
            // End of pattern.  If end of string, TRUE match
            return *String ? FALSE : TRUE;

        case '*':
            // Match zero or more chars
            while (*String) {
                if (MetaMatch(String, Pattern)) {
                    return TRUE;
                }
                String += 1;
            }
            return MetaMatch(String, Pattern);

        case '?':
            // Match any one char
            if (!*String) {
                return FALSE;
            }
            String += 1;
            break;

        case '[':
            // Match char set
            c = *String;
            if (!c) {
                return FALSE; // syntax problem
            }

            l = 0;
            while ((p = *Pattern++)) {
                if (p == ']') {
                    return FALSE;
                }

                if (p == '-') {   // if range of chars,
                    p = *Pattern; // get high range
                    if (p == 0 || p == ']') {
                        return FALSE; // syntax problem
                    }

                    if (c >= l && c <= p) { // if in range,
                        break;              // it's a match
                    }
                }

                l = p;
                if (c == p) { // if char matches
                    break;    // move on
                }
            }

            // skip to end of match char set
            while (p && p != ']') {
                p = *Pattern;
                Pattern += 1;
            }

            String += 1;
            break;

        default:
            c = *String;
            if (c != p) {
                return FALSE;
            }

            String += 1;
            break;
        }
    }
}

BOOLEAN EFIAPI LibStubMetaiMatch(IN EFI_UNICODE_COLLATION_INTERFACE* This EFI_UNUSED, IN CHAR16* String, IN CHAR16* Pattern) { return MetaMatch(String, Pattern); }
