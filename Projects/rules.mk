ROOT_DIR = .

# Toolchain configurations
CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

# Basic configurations
CFLAGS = -g -std=c99
CFLAGS += -Wall

# Cortex-M4 implements the ARMv7E-M architecture
CFLAGS += -mcpu=cortex-m4 -march=armv7e-m -mtune=cortex-m4
CFLAGS += -mlittle-endian -mthumb

# C libraries
define get_library_path
    $(shell dirname $(shell $(CC) $(CFLAGS) -print-file-name=$(1)))
endef
LDFLAGS += -L $(call get_library_path,libc.a)
LDFLAGS += -L $(call get_library_path,libgcc.a)

# Definitions for library usage
CFLAGS += -DUSE_STDPERIPH_DRIVER

# CMSIS and STM32F4 libraries
CFLAGS += \
	-I$(ROOT_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
	-I$(ROOT_DIR)/Drivers/CMSIS/Include \
	-I$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc

%.bin: %.elf
ifeq ($(V), 1)
	$(OBJCOPY) -O binary $< $@
	$(OBJDUMP) -h -S -D $< > $(basename $^).lst
	$(SIZE) $^
else
	@$(OBJCOPY) -O binary $< $@
	@$(OBJDUMP) -h -S -D $< > $(basename $^).lst
	@echo "SIZE  $<"
	@$(SIZE) $<
endif

.PRECIOUS: %.elf

%.elf: $(OBJS)
ifeq ($(V), 1)
	$(LD) -o $@ $(OBJS) $(LDFLAGS)
else
	@echo "LD  $@"
	@$(LD) -o $@ $(OBJS) $(LDFLAGS)
endif

%.o: %.c
ifeq ($(V), 1)
	$(CC) $(CFLAGS) -c $< -o $@
else
	@echo "CC  $<"
	@$(CC) $(CFLAGS) -c $< -o $@
endif

%.o: %.s
ifeq ($(V), 1)
	$(CC) $(CFLAGS) -c $< -o $@
else
	@echo "AS  $<"
	@$(CC) $(CFLAGS) -c $< -o $@
endif

clean:
ifeq ($(V), 1)
	rm -f *.elf *.bin *.lst
	find . -name \*.o -delete
else
	@rm -f *.elf *.bin *.lst
	@find . -name \*.o -delete
	@echo Objects deleted.
endif

flash:
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x_stlink.cfg \
	-c "init" \
	-c "reset init" \
	-c "flash probe 0" \
	-c "flash info 0" \
	-c "flash write_image erase $(shell ls *.bin) 0x8000000" \
	-c "reset run" -c shutdown

.PHONY: clean
