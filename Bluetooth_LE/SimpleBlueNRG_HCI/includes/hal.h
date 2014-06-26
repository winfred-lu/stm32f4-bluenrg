/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : hal.h
* Author             : AMS - HEA&RF BU
* Version            : V1.0.0
* Date               : 19-July-2012
* Description        : Header file which defines Hardware abstraction layer APIs
*                       used by the BLE stack. It defines the set of functions
*                       which needs to be ported to the target platform.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
#ifndef __HAL_H__
#define __HAL_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <hal_types.h>
#include <ble_status.h>

#if defined  (STM32L1XX_MD) || (STM32L1XX_XL) 
#include "SDK_EVAL_Config.h"
#else
#include "SDK_EVAL_Spi_Driver.h"
#endif
/******************************************************************************
 * Macros
 *****************************************************************************/
/* Little Endian buffer to Controller Byte order conversion */
#define LE_TO_HOST_16(ptr)  (uint16_t) ( ((uint16_t) \
                                           *((tHalUint8 *)ptr)) | \
                                          ((uint16_t) \
                                           *((tHalUint8 *)ptr + 1) << 8 ) )

#define LE_TO_HOST_32(ptr)   (tHalUint32) ( ((tHalUint32) \
                                           *((tHalUint8 *)ptr)) | \
                                           ((tHalUint32) \
                                            *((tHalUint8 *)ptr + 1) << 8)  | \
                                           ((tHalUint32) \
                                            *((tHalUint8 *)ptr + 2) << 16) | \
                                           ((tHalUint32) \
                                            *((tHalUint8 *)ptr + 3) << 24) )
											
/* Store Value into a buffer in Little Endian Format */
#define HOST_TO_LE_16(buf, val)    ( ((buf)[0] =  (tHalUint8) (val)    ) , \
                                   ((buf)[1] =  (tHalUint8) (val>>8) ) )

#define HOST_TO_LE_32(buf, val)    ( ((buf)[0] =  (tHalUint8) (val)     ) , \
                                   ((buf)[1] =  (tHalUint8) (val>>8)  ) , \
                                   ((buf)[2] =  (tHalUint8) (val>>16) ) , \
                                   ((buf)[3] =  (tHalUint8) (val>>24) ) ) 
 
/******************************************************************************
 * Types
 *****************************************************************************/

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * Writes data to a serial interface.
 *
 * @param[in]  data1    1st buffer
 * @param[in]  data2    2nd buffer
 * @param[in]  n_bytes1 number of bytes in 1st buffer
 * @param[in]  n_bytes2 number of bytes in 2nd buffer
 */
void Hal_Write_Serial(const void* data1, const void* data2, tHalInt32 n_bytes1, tHalInt32 n_bytes2);

/**
 * Enable interrupts from HCI controller.
 */
void Enable_SPI_IRQ(void);

/**
 * Disable interrupts from BLE controller.
 */
void Disable_SPI_IRQ(void);


#endif /* __HAL_H__ */
