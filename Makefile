TARGET   = firmware
CONFIG   = config.h
SOURCE   = src
INCLUDE  = include
BUILD    = build
OUTPUT   = $(BUILD)/$(TARGET)
LDSCRIPT = ch32v003.ld

# PREFIX   = riscv64-unknown-elf
# PREFIX   = /d/SysGCC/bin/riscv64-unknown-elf
# PREFIX   = /e/xpack-riscv-none-elf-gcc-14.2.0-2/bin/riscv-none-elf
# PREFIX   = "/d/Downloads/MounRiver_Studio_V192_Setup/app/toolchain/RISC-V Embedded GCC12/bin/riscv-none-elf"
PREFIX   = $(MSYS2_ROOT)/ucrt64/bin/riscv32-unknown-elf

# ISPTOOL  = ./minichlink.exe -w $(OUTPUT).bin flash -b
OPENOCD  = /d/Downloads/MounRiver_Studio_V192_Setup/app/toolchain/OpenOCD/bin/openocd.exe -f /d/Downloads/MounRiver_Studio_V192_Setup/app/toolchain/OpenOCD/bin/wch-riscv.cfg
ISPTOOL  = $(OPENOCD) -c "program $(OUTPUT).elf reset exit"

CC       = $(PREFIX)-gcc
OBJCOPY  = $(PREFIX)-objcopy
OBJDUMP  = $(PREFIX)-objdump
OBJSIZE  = $(PREFIX)-size
NM       = $(PREFIX)-nm

# Note: using nano.specs, libc and libm are replaced with their nano variants
# 		also note that libg is the debug build of libc
CFLAGS   = -march=rv32ec -mabi=ilp32e -g -Os -nostdlib -specs=nano.specs
CFLAGS  += -flto -ffunction-sections -fdata-sections -fno-builtin
CFLAGS  += -include $(CONFIG) -I$(INCLUDE) -Wall -Wno-misleading-indentation
DEPFLAGS = -MMD -MP
LDFLAGS  = -T$(LDSCRIPT) -Wl,--gc-sections -lc -lm -lgcc

CFILES   = $(wildcard $(SOURCE)/*.c)
OFILES   = $(addprefix $(BUILD)/,$(patsubst %.c,%.o,$(CFILES)))
DEPFILES:= $(OFILES:%.o=%.d)

all: bin asm lst map size

$(DEPFILES):
-include $(DEPFILES)

$(BUILD)/%.o: %.c $(BUILD)/%.d
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(OUTPUT).elf: $(OFILES)
	@echo "Linking $@"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

bin: $(OUTPUT).bin
%.bin: %.elf
	@$(OBJCOPY) -O binary $< $@

asm: $(OUTPUT).asm
%.asm: %.elf
	@$(OBJDUMP) -d $< > $@

lst: $(OUTPUT).lst
%.lst: %.elf
	@$(OBJDUMP) -S $< > $@

map: $(OUTPUT).map
%.map: %.elf
	@$(OBJDUMP) -t $< > $@

size: $(OUTPUT).elf
	@$(OBJSIZE) -d $<

symbols: $(OUTPUT).elf
	@$(NM) -S --size-sort $<

debug: $(OUTPUT).elf
	$(OPENOCD)

flash: $(OUTPUT).bin
	$(ISPTOOL)

clean:
	@rm -rf $(BUILD)

.PHONY: all bin asm lst map size symbols debug flash clean
.SUFFIXES:
