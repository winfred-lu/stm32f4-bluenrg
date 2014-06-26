/******************** (C) COPYRIGHT 2014 STMicroelectronics ********************
* File Name          : btl.h
* Author             : AMS,AAS division
* Version            : V1.0.0
* Date               : 04-February-2014
* Description        : OTA Btl header file 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BTL_H
#define __BTL_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "hal_types.h"
#include "hci.h"
#include "bluenrg_hci_internal.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

/** On successful update of new image, related flash start and end address will be stored 
 *  in last two words of EEPROM memory.
 *  EEPROM start address information might be used by the Reset Manager and by
 *  the application itself as criteria to calculate
 *  free mem space information to set the 1ST OTA BTL characteristic 
 */
#define NEW_APP_MEM_INFO (uint32_t)(0x08080FF8)

#ifndef VECTOR_TABLE_BASE_DFU_OFFSET 
#define VECTOR_TABLE_BASE_DFU_OFFSET            (0x3000)  /* It is always supported on BlueNRG Eval platforms  */
#endif 
#define VECTOR_TABLE_BASE_RESET_MANAGER_OFFSET  (0x800)   /* Reset manager size: 2K */
#define VECTOR_TABLE_BASE_HIGHER_APP_OFFSET     (0xB000)
#define VECTOR_TABLE_BASE_ADDRESS_OTA_BASIC_APP (0x2A00)


#ifdef ST_OTA_BTL /* OTA Bootloader Service support is enabled */

 #ifdef HIGHER_APP 
    /* application is based at higher FLASH address */
    #define VECTOR_TABLE_BASE_ADDRESS (VECTOR_TABLE_BASE_DFU_OFFSET + VECTOR_TABLE_BASE_HIGHER_APP_OFFSET)

 #else 
    /* application is based at lower FLASH address */
    #define VECTOR_TABLE_BASE_ADDRESS (VECTOR_TABLE_BASE_DFU_OFFSET + VECTOR_TABLE_BASE_RESET_MANAGER_OFFSET)

 #endif /* end HIGHER_APP */

#else
#ifdef ST_OTA_BASIC_APPLICATION /* OTA Service Manager is used */
    /* application is based at lower FLASH address */
    #define VECTOR_TABLE_BASE_ADDRESS (VECTOR_TABLE_BASE_DFU_OFFSET + VECTOR_TABLE_BASE_RESET_MANAGER_OFFSET + VECTOR_TABLE_BASE_ADDRESS_OTA_BASIC_APP)
#else /* No OTA Service is supported; No OTA Service Manager is used */
#define VECTOR_TABLE_BASE_ADDRESS (VECTOR_TABLE_BASE_DFU_OFFSET)
#endif /* ST_OTA_BASIC_APPLICATION */

#endif /* ST_OTA_BTL */

/* end ST_OTA_BTL */

/* Exported Variables  --------------------------------------------------------*/
extern uint8_t bootloadingOngoing;
extern uint8_t BTLServiceUUID4Scan[];

/* Exported Functions  --------------------------------------------------------*/
void OTA_Write_Request_CB(evt_gatt_attr_modified *am);
tBleStatus Add_Btl_Service(void);

void Switch_To_OTA_Service_Manager_Application(uint32_t imageBase);

#endif /* __BTL_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/