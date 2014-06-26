SNSR_DIR = $(ROOT_DIR)/Projects/f401_sensor

SNSR_LDFLAGS = -T $(SNSR_DIR)/stm32f401vc_flash.ld

SNSR_CFLAGS = -DSTM32F401xC -I$(SNSR_DIR) \
	-I$(ROOT_DIR)/Drivers/BSP/Components/lsm303dlhc \
	-I$(ROOT_DIR)/Drivers/BSP/STM32F401-Discovery

SNSR_OBJS = \
	$(ROOT_DIR)/Drivers/BSP/STM32F401-Discovery/stm32f401_discovery.o \
	$(ROOT_DIR)/Drivers/BSP/STM32F401-Discovery/stm32f401_discovery_accelerometer.o \
	$(ROOT_DIR)/Drivers/BSP/Components/lsm303dlhc/lsm303dlhc.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.o \
	$(SNSR_DIR)/main.o \
	$(SNSR_DIR)/newlib_stubs.o \
	$(SNSR_DIR)/startup_stm32f401xc.o \
	$(SNSR_DIR)/system_stm32f4xx.o

# Bluetooth low energy objects
BLE_DIR = $(ROOT_DIR)/Bluetooth_LE/SimpleBlueNRG_HCI
SNSR_CFLAGS += -I$(BLE_DIR)/includes
SNSR_OBJS += \
	$(BLE_DIR)/hci/hci.o \
	$(BLE_DIR)/hci/controller/bluenrg_hci.o \
	$(BLE_DIR)/utils/list.o \
	$(SNSR_DIR)/gatt_db.o


ifeq ($(WITH_USART) $(WITH_VCP), 1 1)
$(error Both USART and VCP are enabled, please choose only one of them)
endif

# USART support
ifeq ($(WITH_USART), 1)
SNSR_CFLAGS += -DWITH_USART
SNSR_OBJS += \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.o
endif

# USB virtual com port support
ifeq ($(WITH_VCP), 1)
SNSR_CFLAGS += -DWITH_VCP \
	-I$(ROOT_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
	-I$(ROOT_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
SNSR_OBJS += \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.o \
	$(SNSR_DIR)/usbd_conf.o \
	$(SNSR_DIR)/usbd_cdc_interface.o \
	$(SNSR_DIR)/usbd_desc.o
endif

sensor: LDFLAGS += $(SNSR_LDFLAGS)
sensor: CFLAGS += $(SNSR_CFLAGS)
sensor: $(OBJS) $(SNSR_OBJS)
sensor: OBJS += $(SNSR_OBJS)
sensor: sensor.bin
