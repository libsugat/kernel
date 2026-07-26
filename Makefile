# Terminal options
ARCH ?= ia32

# Toolchain Configuration
CC = gcc
# CC = clang -target i686-unknown-none-elf
LD = ld

CC_FLAGS  := -Wall -Wextra -ffreestanding -nostdlib -fpie -fno-stack-protector
ASFLAGS   := $(CC_FLAGS)
LDFLAGS   :=

CC_FLAGS += -std=c11
CC_FLAGS += -Iarch/$(ARCH)/include -Iinclude

# Architecture-specific flags
include arch/$(ARCH)/config.mk

# Directories & Targets
BUILD_DIR           := build
TARGET              := $(BUILD_DIR)/kernel.elf

C_SRCS   :=
ASM_SRCS :=


# ------------ Sources -----------
include arch/$(ARCH)/Makefile # architecture specific sources
include lib/Makefile
include kernel/Makefile

C_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SRCS))
ASM_OBJS := $(patsubst %.S, $(BUILD_DIR)/%.o, $(ASM_SRCS))
OBJS := $(C_OBJS) $(ASM_OBJS)

# --- Rules ---
all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@

# =========== Compilation ===========
$(BUILD_DIR)/%.o: %.c
	@echo "[Compiling] : $< -> $@"
	@mkdir -p $(dir $@)
	@$(CC) $(CC_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	@echo "[Assembling]: $< -> $@"
	@mkdir -p $(dir $@)
	@$(CC) $(ASFLAGS) -c $< -o $@

# =========== Linking ==============
$(TARGET): $(OBJS) arch/$(ARCH)/kernel.ld
	@echo "[Linking]   : $^ -> $@"
	@$(LD) $(LDFLAGS) -T arch/$(ARCH)/kernel.ld $(OBJS) -o $@

# ============ Housekeeping & phony targets ================
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
