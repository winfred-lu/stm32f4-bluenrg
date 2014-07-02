ADIO_DIR = $(ROOT_DIR)/Projects/Audio_playback_and_record

ADIO_LDFLAGS = -T $(ADIO_DIR)/gnu/stm32f401vc_flash.ld

ADIO_CFLAGS = -DSTM32F401xC -I$(ADIO_DIR)/Inc \
	-I$(ROOT_DIR)/Drivers/BSP/STM32F401-Discovery \
	-I$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc \
	-I$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Inc \
	-I$(ROOT_DIR)/Middlewares/Third_Party/FatFs/src \
	-I$(ROOT_DIR)/Middlewares/Third_Party/FatFs/src/drivers

ADIO_OBJS = \
	$(ROOT_DIR)/Drivers/BSP/STM32F401-Discovery/stm32f401_discovery.o \
	$(ROOT_DIR)/Drivers/BSP/STM32F401-Discovery/stm32f401_discovery_accelerometer.o \
	$(ROOT_DIR)/Drivers/BSP/STM32F401-Discovery/stm32f401_discovery_audio.o \
	$(ROOT_DIR)/Drivers/BSP/Components/cs43l22/cs43l22.o \
	$(ROOT_DIR)/Drivers/BSP/Components/lsm303dlhc/lsm303dlhc.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hcd.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s_ex.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.o \
	$(ROOT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_bot.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_scsi.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_core.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ctlreq.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ioreq.o \
	$(ROOT_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_pipes.o \
	$(ROOT_DIR)/Middlewares/Third_Party/FatFs/src/diskio.o \
	$(ROOT_DIR)/Middlewares/Third_Party/FatFs/src/ff.o \
	$(ROOT_DIR)/Middlewares/Third_Party/FatFs/src/ff_gen_drv.o \
	$(ROOT_DIR)/Middlewares/Third_Party/FatFs/src/drivers/usbh_diskio.o \
	$(ADIO_DIR)/gnu/newlib_stubs.o \
	$(ADIO_DIR)/gnu/startup_stm32f401xc.o \
	$(ADIO_DIR)/Src/main.o \
	$(ADIO_DIR)/Src/stm32f4xx_it.o \
	$(ADIO_DIR)/Src/system_stm32f4xx.o \
	$(ADIO_DIR)/Src/usbh_conf.o \
	$(ADIO_DIR)/Src/waveplayer.o \
	$(ADIO_DIR)/Src/waverecorder.o

ADIO_OBJS += \
	$(ROOT_DIR)/Middlewares/ST/STM32_Audio/Addons/PDM/libPDMFilter_CM4_GCC.a

audio: LDFLAGS += $(ADIO_LDFLAGS)
audio: CFLAGS += $(ADIO_CFLAGS)
audio: $(OBJS) $(ADIO_OBJS)
audio: OBJS += $(ADIO_OBJS)
audio: audio.bin
