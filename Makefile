# DIRECTORIES
BIN_DIR := ./bin
OBJ_DIR := ./obj
SRC_DIR := ./src

# TARGET
BOOTLOADER := $(BIN_DIR)/BOOTX64.EFI

# SOURCE FILES
C_SRC_FILES := $(shell find $(SRC_DIR) -name '*.c')

LINK_FILE := ./elf_x86_64_efi.lds

# OBJECT FILES
C_OBJ_FILES := $(C_SRC_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# PROGRAMS
CC := x86_64-w64-mingw32-gcc
CC_FLAGS := -ffreestanding -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol -c

LD := x86_64-w64-mingw32-gcc
LD_FLAGS := -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main
LD_POST_FLAGS :=

# BASE RULES
all: dirs $(BOOTLOADER)

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BIN_DIR)/*

# COMPILATION RULES
.SECONDEXPANSION:

$(BOOTLOADER): $(C_OBJ_FILES)
	$(LD) $(LD_FLAGS) -o $@ $^ $(LD_POST_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $$(@D)/.
	$(CC) $(CC_FLAGS) -o $@ $^

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