TARGET   = firmware
SOURCE   = src
INCLUDE  = include
BUILD    = build
LDSCRIPT = ch32v003.ld
PREFIX   = riscv64-unknown-elf
NEWLIB   = /usr/include/newlib
ISPTOOL  = ./minichlink.exe -w $(BUILD)/$(TARGET).bin flash -b

CC       = $(PREFIX)-gcc
OBJCOPY  = $(PREFIX)-objcopy
OBJDUMP  = $(PREFIX)-objdump
OBJSIZE  = $(PREFIX)-size

CFLAGS   = -g -Os -flto -ffunction-sections -fdata-sections -fno-builtin -nostdlib
CFLAGS  += -march=rv32ec -mabi=ilp32e -I$(NEWLIB) -I$(INCLUDE) -I$(SOURCE) -I. -Wall
LDFLAGS  = -T$(LDSCRIPT) -lgcc -Wl,--gc-sections,--build-id=none

VPATH    = $(SRC)
CFILES   = $(wildcard $(SOURCE)/*.c)
OFILES   = $(addprefix $(BUILD)/,$(patsubst %.c,%.o,$(CFILES)))

OUTPUT   = $(BUILD)/$(TARGET)

all: bin asm lst map size

$(BUILD)/%.o: %.c
	@echo "Compiling $<"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT).elf: $(OFILES)
	@echo "Linking $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

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
