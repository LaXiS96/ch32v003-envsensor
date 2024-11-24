TARGET   = firmware
SOURCE   = src
INCLUDE  = include
BUILD    = build
OUTPUT   = $(BUILD)/$(TARGET)
LDSCRIPT = ch32v003.ld

# PREFIX   = riscv64-unknown-elf
# PREFIX   = /d/SysGCC/bin/riscv64-unknown-elf
# PREFIX   = /e/xpack-riscv-none-elf-gcc-14.2.0-2/bin/riscv-none-elf
# PREFIX   = "/d/Downloads/MounRiver_Studio_V192_Setup/app/toolchain/RISC-V Embedded GCC12/bin/riscv-none-elf"
PREFIX   = /e/msys64/home/LaXiS/riscv-gnu-toolchain/build-ch32v003/bin/riscv32-unknown-elf

ISPTOOL  = ./minichlink.exe -w $(OUTPUT).bin flash -b
# OPENOCD  = /d/Downloads/MounRiver_Studio_V192_Setup/app/toolchain/OpenOCD/bin/openocd.exe
# ISPTOOL  = $(OPENOCD) -s $(shell dirname "$(OPENOCD)") -f wch-riscv.cfg -c "program $(OUTPUT).elf reset exit"

CC       = $(PREFIX)-gcc
OBJCOPY  = $(PREFIX)-objcopy
OBJDUMP  = $(PREFIX)-objdump
OBJSIZE  = $(PREFIX)-size

# Note: using nano.specs, libc and libm are replaced with their nano variants
# 		also note that libg is the debug build of libc
CFLAGS   = -march=rv32ec -mabi=ilp32e -g -Os -flto -ffunction-sections -fdata-sections -fno-builtin -nostdlib -specs=nano.specs
CFLAGS  += -DF_CPU=48000000
CFLAGS  += -I$(INCLUDE) -Wall -Wno-misleading-indentation
LDFLAGS  = -T$(LDSCRIPT) -Wl,--gc-sections,--build-id=none -lc -lm -lgcc

VPATH    = $(SRC)
CFILES   = $(wildcard $(SOURCE)/*.c)
OFILES   = $(addprefix $(BUILD)/,$(patsubst %.c,%.o,$(CFILES)))

all: bin asm lst map size

$(BUILD)/%.o: %.c
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

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

flash: $(OUTPUT).bin
	$(ISPTOOL)	

clean:
	@rm -rf $(BUILD)

.PHONY: all bin asm lst map size flash clean
.SUFFIXES:
