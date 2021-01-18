/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    str.c

Abstract:

    String runtime functions


Revision History

--*/

#include "lib.h"

#ifndef __GNUC__
#pragma RUNTIME_CODE(RtStrCmp)
#endif
INTN RUNTIMEFUNCTION RtStrCmp(IN CONST CHAR16* s1, IN CONST CHAR16* s2)
// compare strings
{
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return *s1 - *s2;
}
