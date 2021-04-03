# DIRECTORIES
BIN_DIR := ./bin
OBJ_DIR := ./obj
SRC_DIR := ./src

SYSROOT_DIR := ../sysroot

# TARGET
BOOTLOADER := $(BIN_DIR)/BOOTX64.EFI

# SOURCE FILES
C_SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')

LINK_FILE := ./elf_x86_64_efi.lds

# OBJECT FILES
C_OBJ_FILES := $(C_SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# PROGRAMS
CC := x86_64-w64-mingw32-gcc
CC_FLAGS := -ffreestanding -Iinclude -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol -c -g

LD := x86_64-w64-mingw32-gcc
LD_FLAGS := -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -g

# BASE RULES
all: dirs $(BOOTLOADER)
	@echo "[ BOOTLOADER ] Build Complete!"

install: all
	@mkdir -p $(SYSROOT_DIR)/EFI/BOOT
	@cp $(BOOTLOADER) $(SYSROOT_DIR)/EFI/BOOT
	@echo "[ BOOTLOADER ] Installed!"

clean:
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(BIN_DIR)/*
	@echo "[ BOOTLOADER ] Cleaned!"

# COMPILATION RULES
.SECONDEXPANSION:

$(BOOTLOADER): $(ASM_OBJ_FILES) $(C_OBJ_FILES)
	@echo "[ BOOTLOADER ] Linking $@ . . ."
	@$(LD) $(LD_FLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $$(@D)/.
	@echo "[ BOOTLOADER ] Compiling $@ . . ."
	@$(CC) $(CC_FLAGS) -o $@ $^


# DIRECTORY RULES
$(OBJ_DIR)/.:
	@mkdir -p $@

$(OBJ_DIR)%/.:
	@mkdir -p $@

dirs:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

# . RULES
.PRECIOUS: $(OBJ_DIR)/. $(OBJ_DIR)%/.
.PHONY: dirs