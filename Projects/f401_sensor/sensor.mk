SNSR_DIR = $(ROOT_DIR)/Projects/f401_sensor

SNSR_LDFLAGS = -T $(SNSR_DIR)/stm32f401vc_flash.ld

SNSR_CFLAGS = -DSTM32F401xC -I$(SNSR_DIR)

SNSR_OBJS = \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.o \
	$(SNSR_DIR)/startup_stm32f401xc.o \
	$(SNSR_DIR)/system_stm32f4xx.o \
	$(SNSR_DIR)/main.o


sensor: LDFLAGS += $(SNSR_LDFLAGS)
sensor: CFLAGS += $(SNSR_CFLAGS)
sensor: $(OBJS) $(SNSR_OBJS)
sensor: OBJS += $(SNSR_OBJS)
sensor: sensor.bin
