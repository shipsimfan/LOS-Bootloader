#include <elf.h>

#include <console.h>
#include <error.h>
#include <systemTable.h>

EFI_STATUS LoadELFExecutable(void* file, UINT64* entry) {
    Elf64_Ehdr* hdr = file;

    // Check ELF MAG
    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || hdr->e_ident[EI_MAG1] != ELFMAG1 || hdr->e_ident[EI_MAG2] != ELFMAG2 || hdr->e_ident[EI_MAG3] != ELFMAG3)
        return ELF_INCORRECT_MAG;

    // Verify class
    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
        return ELF_INVALID_CLASS;

    // Verify data order
    if (hdr->e_ident[EI_DATA] != ELFDATA2LSB)
        return ELF_INVALID_DATA;

    // Verify version
    if (hdr->e_ident[EI_VERSION] == 0 || hdr->e_version == 0)
        return ELF_INVALID_VERSION;

    // Verify type
    if (hdr->e_type != ET_EXEC)
        return ELF_INVALID_TYPE;

    // Verify machine
    if (hdr->e_machine != EM_AMD64)
        return ELF_INVALID_MACHINE;

    // Load the executable
    Elf64_Phdr* phdr = (Elf64_Phdr*)((Elf64_Off)hdr + hdr->e_phoff);
    for (int i = 0; i < hdr->e_phnum; i++, phdr = (Elf64_Phdr*)((Elf64_Xword)phdr + hdr->e_phentsize)) {
        if (phdr->p_type == PT_LOAD) {
            EFI_STATUS status = SYSTEM_TABLE->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, EFI_SIZE_TO_PAGES(phdr->p_memsz), (EFI_PHYSICAL_ADDRESS*)phdr->p_paddr);
            if (EFI_ERROR(status))
                return status;

            if (phdr->p_filesz > 0)
                SYSTEM_TABLE->BootServices->CopyMem((void*)phdr->p_paddr, (void*)((Elf64_Xword)hdr + phdr->p_offset), phdr->p_filesz);

            Elf64_Xword diff = phdr->p_memsz - phdr->p_filesz;
            uint8_t* start = (uint8_t*)(phdr->p_paddr + phdr->p_filesz);
            for (Elf64_Xword i = 0; i < diff; i++)
                start[i] = 0;
        }
    }

    *entry = hdr->e_entry;

    return EFI_SUCCESS;
}
