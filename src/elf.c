#include <efi.h>
#include <elf.h>

extern EFI_SYSTEM_TABLE* ST;

extern int Print(CHAR16* str);

int VerifyELFHeader(Elf64_Ehdr* hdr, unsigned char class, unsigned char data, Elf64_Half type, Elf64_Half machine) {
    // Check ELF MAG
    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || hdr->e_ident[EI_MAG1] != ELFMAG1 || hdr->e_ident[EI_MAG2] != ELFMAG2 || hdr->e_ident[EI_MAG3] != ELFMAG3)
        return -1;

    // Verify class
    if (hdr->e_ident[EI_CLASS] != class)
        return -2;

    // Verify data order
    if (hdr->e_ident[EI_DATA] != data)
        return -3;

    // Verify version
    if (hdr->e_ident[EI_VERSION] == 0)
        return -4;

    // Verify type
    if (hdr->e_type != type)
        return -5;

    // Verify machine
    if (hdr->e_machine != machine)
        return -6;

    // Verify version again
    if (hdr->e_version == 0)
        return -7;

    return 0;
}

int LoadELFExecutable(Elf64_Ehdr* hdr) {
    int status = VerifyELFHeader(hdr, ELFCLASS64, ELFDATA2LSB, ET_EXEC, EM_AMD64);
    if (status < 0)
        return status;

    // Load the executable
    Elf64_Phdr* phdr = (Elf64_Phdr*)((Elf64_Off)hdr + hdr->e_phoff);
    for (int i = 0; i < hdr->e_phnum; i++, phdr = (Elf64_Phdr*)((Elf64_Xword)phdr + hdr->e_phentsize)) {
        if (phdr->p_type == PT_LOAD) {
            status = ST->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, EFI_SIZE_TO_PAGES(phdr->p_memsz), (EFI_PHYSICAL_ADDRESS*)phdr->p_paddr);
            if (EFI_ERROR(status))
                return -1;

            if (phdr->p_filesz > 0)
                ST->BootServices->CopyMem((void*)phdr->p_paddr, (void*)((Elf64_Xword)hdr + phdr->p_offset), phdr->p_filesz);
        }
    }
}