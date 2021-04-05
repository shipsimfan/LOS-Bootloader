#include <acpi.h>

#include <stddef.h>
#include <systemTable.h>

EFI_GUID acpi20GUID = {0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}};

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = s1;
    const uint8_t* p2 = s2;
    for (size_t i = 0; i < n; i++)
        if (p1[i] != p2[i])
            return p1[i] - p2[i];

    return 0;
}

EFI_STATUS GetRDSP(void** rdsp) {
    *rdsp = NULL;
    EFI_CONFIGURATION_TABLE* ect = SYSTEM_TABLE->ConfigurationTable;
    for (int i = 0; i < SYSTEM_TABLE->NumberOfTableEntries; i++) {
        if (memcmp(&ect->VendorGuid, &acpi20GUID, sizeof(acpi20GUID)) == 0)
            *rdsp = ect->VendorTable;

        ect++;
    }

    if (*rdsp == NULL)
        return EFI_NOT_FOUND;

    return EFI_SUCCESS;
}