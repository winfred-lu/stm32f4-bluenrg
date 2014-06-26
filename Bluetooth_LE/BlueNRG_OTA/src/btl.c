/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : btl.c
* Author             : AMS - AAS division
* Version            : V1.0.1
* Date               : 10-February-2014
* Description        : BlueNRG OTA service implementation
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "hw_config.h"
#include "hal_types.h"
#include "hci.h"
#include "bluenrg_hci.h"
#include "gp_timer.h"
#include "hal.h"
#include "osal.h"
#include "gatt_server.h"
#include "hci_internal.h"
#include "bluenrg_hci_internal.h"
#include "gap.h"
#include "sm.h"
#include <stdio.h>

#include "stm32l1xx_flash.h"
#include "clock.h"

#include "SDK_EVAL_Config.h"

#include "btl.h"

/** Define variables for user choice:
 *  For essential amount of status information during OTA bootloader session 
 *  let ST_OTA_BTL_MINIMAL_ECHO be defined (strongly suggested)
 *  To add more verbosity on OTA bootloader output let ST_OTA_BTL_ECHO be defined
 */
#define ST_OTA_BTL_MINIMAL_ECHO
//#define ST_OTA_BTL_ECHO 

/* The following defines **MUST NOT** be modified for proper operation of the current OTA BTL release */
#define PAGE_SIZE 256
#define BUF_SIZE 128
#define NOTIFICATION_WINDOW 1

/* defines for FLASH flags init */
#define  FLASH_SR_OPTVERRUSR                 ((uint32_t)0x00001000)        /* Option User validity error */ 
#define FLASH_FLAG_OPTVERRUSR                FLASH_SR_OPTVERRUSR           /* FLASH Option User Validity error flag  */

/* Half page flash writes will be protected as atomic */
#define ATOMIC(x) {asm volatile ("cpsid i"); {x};   asm volatile ("cpsie i");}

/* OTA bootloades notification error codes */
#define OTA_SUCCESS            0x0000
#define OTA_FLASH_VERIFY_ERROR 0x003C
#define OTA_FLASH_WRITE_ERROR  0x00FF
#define OTA_SEQUENCE_ERROR     0x00F0
#define OTA_CHECKSUM_ERROR     0x000F

typedef  void (*pFunction)(void);

/* Characteristic handles */
tHalUint16 btlServHandle, btlImageCharHandle, btlNewImageCharHandle, btlNewImageTUContentCharHandle, btlExpectedImageTUSeqNumberCharHandle;

/* OTA service & characteristic UUID 128:

OTA service: 8a97f7c0-8506-11e3-baa7-0800200c9a66
Image Characteristic: 122e8cc0-8508-11e3-baa7-0800200c9a66
New Image Characteristic: 210f99f0-8508-11e3-baa7-0800200c9a66
New Image TU Content Characteristic:2691aa80-8508-11e3-baa7-0800200c9a66
ExpectedImageTUSeqNumber Characteristic: 2bdc5760-8508-11e3-baa7-0800200c9a66
*/

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)

#define COPY_BTL_SERVICE_UUID(uuid_struct)                   COPY_UUID_128(uuid_struct,0x8a,0x97,0xf7,0xc0,0x85,0x06,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_IMAGE_CHAR_UUID(uuid_struct)                    COPY_UUID_128(uuid_struct,0x12,0x2e,0x8c,0xc0,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_NEW_IMAGE_CHAR_UUID(uuid_struct)                COPY_UUID_128(uuid_struct,0x21,0x0f,0x99,0xf0,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_NEW_IMAGE_TU_CONTENT_CHAR_UUID(uuid_struct)     COPY_UUID_128(uuid_struct,0x26,0x91,0xaa,0x80,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)
#define COPY_EXPECTED_IMAGE_TU_SEQNUM_CHAR_UUID(uuid_struct) COPY_UUID_128(uuid_struct,0x2b,0xdc,0x57,0x60,0x85,0x08,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66)

/* Free flash space advertised by the application (data for 1ST OTA BTL characteristic initialization) */

#ifdef ST_OTA_BASIC_APPLICATION_FIXED_BASE_ADDRESS /* it is used only from OTA_ServiceManager application */
/* DFU is enabled by default. Reset Manager size: 2K (0x800) */
/* Free space x lower application starts from DFU base: 0x3000 + Reset Manager size: 0x800  +  OTA_ServiceManager_Size: 0x2A00  = 0x6200 */
tHalUint8 currentImageInfos[8] = {0x08,0x00,0x62,0x00,0x08,0x01,0xFF,0xFF};

#else

#ifdef HIGHER_APP
/* DFU is enabled by default. Reset Manager size: 2K (0x800) */
/* Free space x lower application starts from DFU base: 0x3000 + Reset Manager size: 0x800 */
tHalUint8 currentImageInfos[8] = {0x08,0x00,0x38,0x00,0x08,0x00,0xE0,0x00}; 
#else /* lower application */
/* DFU is enabled by default */
/* Free space for higher application starts from  DFU base: 0x3000 + Higher Application default base: 0xB000 */
tHalUint8 currentImageInfos[8] = {0x08,0x00,0xE0,0x00,0x08,0x01,0xFF,0xFF};
#endif

#endif 

struct
{
    uint16_t  replyCounter;
    uint16_t  errCode;    
} notification;

uint8_t imageBuffer[BUF_SIZE];

uint8_t BTLServiceUUID4Scan[18]= {0x11,0x06,0x8a,0x97,0xf7,0xc0,0x85,0x06,0x11,0xe3,0xba,0xa7,0x08,0x00,0x20,0x0c,0x9a,0x66}; 

/* Let the application know whether we are in the middle of a bootloading session through a global status variable */
uint8_t bootloadingOngoing = 0;
  
/**
 * @brief  EEPROM erase utility made available to ensure EEPROM initialization at system setup
 * @param  None.
 * @retval status.
 */
uint8_t resetDataEEPROM(void)
{
   FLASH_Status flashStatus;
   
   DATA_EEPROM_Unlock();
   flashStatus = DATA_EEPROM_EraseWord(NEW_APP_MEM_INFO);
   flashStatus |= DATA_EEPROM_EraseWord(NEW_APP_MEM_INFO + 4);
   DATA_EEPROM_Lock();
   
   if (flashStatus == FLASH_COMPLETE)
     return 1;
   else
     return 0;
}/* end resetDataEEPROM() */


/**
 * @brief  Enables flash interface
 * @param  None.
 * @retval None.
 */
void enableFlitf(void)
{
    /* Unlock the Program memory */
  FLASH_Unlock();

  /* Clear all FLASH flags */  
  FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR
                  | FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR | FLASH_FLAG_OPTVERRUSR);
}/* end enableFlitf() */

/**
 * @brief  Disables flash interface.
 * @param  None.
 * @retval None.
 */
void disableFlitf(void)
{
  FLASH_Lock();
}/* end disableFlitf() */


/**
 * @brief  Verifies flash content.
 * @param  currentWriteAddress: beginning of written address
 *         pbuffer: target buffer address
 *         size: buffer size
 * @retval Status.
 */
FLASH_Status FLASH_Verify(uint32_t currentWriteAddress,uint32_t * pbuffer,uint8_t size)
{
  uint8_t * psource = (uint8_t*) (currentWriteAddress);
  uint8_t * pdest   = (uint8_t*) pbuffer;
  
  for(;(size>0) && (*(psource++) == *(pdest++)) ;size--);
  
  if (size>0)
    return FLASH_ERROR_PROGRAM;
  else
    return FLASH_COMPLETE;
}


/**
 * @brief  Jump to the OTA Service Manager Application
 * @param  imageBase: base address where to jump on reset
 * @retval None.
 */

void Switch_To_OTA_Service_Manager_Application(uint32_t imageBase)
{
  FLASH_Status flashStatus;
  
  DATA_EEPROM_Unlock();
  flashStatus = DATA_EEPROM_ProgramWord(NEW_APP_MEM_INFO, imageBase);               
  DATA_EEPROM_Lock();
  if (flashStatus == FLASH_COMPLETE)
  {
    /* We are jumping towards new app */       
    /* Reset manager will take care of running the new application*/
    NVIC_SystemReset();
  }
}

/**
 * @brief  Add the 'Bootloader' service.
 * @param  None.
 * @retval Status.
 */
tBleStatus Add_Btl_Service(void)
{
    tBleStatus ret;
    uint8_t uuid[16];
    
    COPY_BTL_SERVICE_UUID(uuid);
    /* OTA BOOTLOADER SERVICE ALLOCATION */
    ret = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 10, &btlServHandle); /* TBR: original = 41 moved to 10 */
    if (ret != BLE_STATUS_SUCCESS) goto fail;    
    
    COPY_IMAGE_CHAR_UUID(uuid);
    /* 1ST OTA SERVICE CHARAC ALLOCATION: FREE FLASH RANGE INFOS */
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, uuid, 8, CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT,
                             16, 0, &btlImageCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO    
    printf("\r\n");
    printf("\r\n *** btlImageCharHandle : 0x%04X ***\r\n", btlImageCharHandle);
    printf("\r\n");
#endif
    
    COPY_NEW_IMAGE_CHAR_UUID(uuid);
    /* 2ND OTA SERVICE CHARAC ALLOCATION: BASE AND SIZE OF NEW IMAGE */
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, uuid, 9, CHAR_PROP_READ|CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT | GATT_SERVER_ATTR_WRITE,
                             16, 0, &btlNewImageCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO     
    printf("\r\n");
    printf("\r\n *** btlNewImageCharHandle : 0x%04X ***\r\n", btlNewImageCharHandle);
    printf("\r\n");
#endif
    
    COPY_NEW_IMAGE_TU_CONTENT_CHAR_UUID(uuid);
    /* 3RD OTA SERVICE CHARAC ALLOCATION: IMAGE CONTENT  */
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_READ|CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT | GATT_SERVER_ATTR_WRITE,
                             16, 0, &btlNewImageTUContentCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO     
    printf("\r\n");
    printf("\r\n *** btlNewImageTUContentCharHandle : 0x%04X ***\r\n", btlNewImageTUContentCharHandle);
    printf("\r\n");
#endif    

    COPY_EXPECTED_IMAGE_TU_SEQNUM_CHAR_UUID(uuid);
    /* 4TH OTA SERVICE CHARAC ALLOCATION: IMAGE BLOCK FOR NOTIFICATION, INCLUDES NEXT EXPECTED IMAGE NUMBER AND ERROR CONDITIONS */
    ret =  aci_gatt_add_char(btlServHandle, UUID_TYPE_128, uuid, 4, CHAR_PROP_NOTIFY|CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT,
                             16, 0, &btlExpectedImageTUSeqNumberCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef ST_OTA_BTL_ECHO     
    printf("\r\n");
    printf("\r\n *** btlExpectedImageTUSeqNumberCharHandle : 0x%04X ***\r\n", btlExpectedImageTUSeqNumberCharHandle);
    printf("\r\n");
#endif
	
    /* We provide defaults current image information characteristic (1ST CHAR)*/
    ret = aci_gatt_update_char_value(btlServHandle, btlImageCharHandle, 0, 8,currentImageInfos);
    if (ret != BLE_STATUS_SUCCESS)
    {
      printf("Error while updating characteristic.\n");
      return BLE_STATUS_ERROR;
    }
    
    return BLE_STATUS_SUCCESS;
           
fail:
    return BLE_STATUS_ERROR ;
    
}/* end Add_Btl_Service() */

/**
 * @brief  OTA Bootloading service callback.
 * @param  evt_gatt_attr_modified
 * @retval None
 */
void OTA_Write_Request_CB(evt_gatt_attr_modified *am)
{
    static uint32_t imageSize = 0; 
    static uint16_t bufPointer = 0;
    static uint16_t totalBytesWritten = 0;
    static uint8_t checksum = 0;
    static uint32_t imageBase = 0;           
    static uint32_t currentWriteAddress = 0;
    static uint16_t numPages = 0;
    static uint16_t expectedSeqNum = 0;
    uint16_t receivedSeqNum;
    FLASH_Status flashStatus;
    FLASH_Status verifyStatus;
    tBleStatus ret;
    uint16_t k;

    notification.replyCounter = 0;
    notification.errCode = 0;

    if (am->attr_handle == (btlNewImageCharHandle + 1)){
      /* Incoming write charachteristic to allow master to specify the base address and size
       * of the firmaware image it intends to send. 
       * Get base_address and image size. 
       */
      imageSize = (uint32_t)(am->att_data[4] << 24) + (uint32_t)(am->att_data[3] << 16) + (uint32_t)(am->att_data[2] << 8) + am->att_data[1];
      imageBase = (uint32_t)(am->att_data[8] << 24) + (uint32_t)(am->att_data[7] << 16) + (uint32_t)(am->att_data[6] << 8) + am->att_data[5];
      currentWriteAddress = imageBase;
      numPages = imageSize/PAGE_SIZE;
      bufPointer = 0;
      totalBytesWritten = 0;
      expectedSeqNum = 0;
#ifdef ST_OTA_BTL_MINIMAL_ECHO
      printf("Image base = 0x%08X ; Image size = 0x%08X \r\n",imageBase,imageSize);
#endif      
    } else  if (am->attr_handle == (btlExpectedImageTUSeqNumberCharHandle + 2)){

     /* Here we are handling write characteristic descriptor, switch on notifications.
      * At this point it performs required pages erase according to the previously provided image 
      * size and provide notification
      */
      if (imageSize%PAGE_SIZE != 0)
        numPages++;
      expectedSeqNum = 0;
      enableFlitf();
      flashStatus = FLASH_BUSY; /* this holds only to treat the numPages==0 case */
      for(k=0;k<numPages;k++){
          static uint32_t eraseAddress;
          eraseAddress = imageBase + (k * PAGE_SIZE);
          flashStatus = FLASH_ErasePage(eraseAddress);
#ifdef ST_OTA_BTL_ECHO
          printf("Erasing page %d at base address 0x%08X \r\n",k,eraseAddress);
#endif          
          if (flashStatus != FLASH_COMPLETE)
            break;
       }
       disableFlitf();
       if (flashStatus == FLASH_COMPLETE){
#ifdef ST_OTA_BTL_MINIMAL_ECHO
          printf("Page erase successful, sending notification\r\n");
#endif
          notification.replyCounter = 0;
          notification.errCode = 0;
          ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(tHalUint8*)&notification);
          if (ret != BLE_STATUS_SUCCESS) {
            printf("Error while updating ACC characteristic.\n");
          } else {            
          /* light up led on the BlueNRG platform to advertise beginning of OTA bootloading session */
#ifdef HIGHER_APP
            SdkEvalLedOn(LED1);
#else
            SdkEvalLedOn(LED2);
#endif 
            /* warn beginning of bootloading session through gloabal variable */
            bootloadingOngoing = 1;
          }
       }
#ifdef ST_OTA_BTL_MINIMAL_ECHO
       else{
          printf("Page erase failure \r\n");
       }
#endif
    } else if (am->attr_handle == (btlNewImageTUContentCharHandle + 1)){
       /* Here we read updated characteristic content filled by "write with no response command' coming from the master */
       if (bufPointer < BUF_SIZE){
          /* Data will be received by the OTA slave 16 byte wise (due to charachteristic image content = 16 bytes image + 4 of headers)
           * Drop new image data into buffer
           */
          for(k=bufPointer; k<(bufPointer + am->data_length - 4); k++){
             if (k<imageSize)
               imageBuffer[k] = am->att_data[(k - bufPointer) + 1];
             else
               /* zero pad unutilized residual*/
               imageBuffer[k] = 0;
              
             checksum ^= imageBuffer[k];
          }
          /* include header data into checksum processing as well */
          checksum ^= (am->att_data[am->data_length - 3] ^ am->att_data[am->data_length - 2] ^ am->att_data[am->data_length - 1]);          
          bufPointer = k;
#ifdef ST_OTA_BTL_ECHO
          printf("NEW IMAGE PACKET:\r\n");
#endif
          /* In the section of code below: notify for received packet integrity (cheksum), sequence number correctness
           * and eventually write flash (which will get notified as well)
          */
          
          /* check checksum */ 
          if (checksum == am->att_data[0]){
             /* checksum ok */
#ifdef ST_OTA_BTL_ECHO
             printf("Integrity check passed \r\n");
#endif      
             /* sequence number check */
             receivedSeqNum = ((am->att_data[19]<<8) + am->att_data[18]);
             if (expectedSeqNum == receivedSeqNum) { 
                /* sequence number check ok, increment expected sequence number and prepare for next block notification */
                expectedSeqNum++;
#ifdef ST_OTA_BTL_ECHO
                printf("Sequence check passed for packet number %d \r\n",receivedSeqNum);
#endif              
                if ((((receivedSeqNum+1) % NOTIFICATION_WINDOW) == 0) || (((receivedSeqNum+1)*16) >= imageSize)){
                   /* Here is where we manage notifications related to correct sequence number and write/verify
                    * results if conditions get us through the next nested 'if' section (FLASH write section)
                    */
                   /* replyCounter defaults to expectedSeqNum unless flash write fails */
                   notification.replyCounter = expectedSeqNum;
                   notification.errCode = 0x0000;
                  
                   if (((bufPointer % BUF_SIZE) == 0) || (((imageSize - totalBytesWritten) < BUF_SIZE)&& (bufPointer >= (imageSize - totalBytesWritten)))){
                      /* drop buffer into flash if it's the right time */
                      currentWriteAddress = imageBase + totalBytesWritten;
#ifdef ST_OTA_BTL_ECHO
                      printf("Attempting to write *%d* byte image buffer into flash\r\n",bufPointer);
#endif 
                      k=0;
                      verifyStatus = FLASH_COMPLETE;
                      enableFlitf();
                      while (k<bufPointer){
                         uint8_t byteIncrement = 128;
                         if ((bufPointer - k) >= 128){
                            ATOMIC(flashStatus = FLASH_ProgramHalfPage(currentWriteAddress, (uint32_t *)(&imageBuffer[k])););
                            verifyStatus = FLASH_Verify(currentWriteAddress,(uint32_t *)(&imageBuffer[k]),128);
                         } else {
                            byteIncrement = 4;
                            flashStatus = FLASH_FastProgramWord(currentWriteAddress, (((uint32_t)imageBuffer[k+3]<< 24) + ((uint32_t)imageBuffer[k+2]<< 16) + ((uint32_t)imageBuffer[k+1]<< 8) + (uint32_t)imageBuffer[k]) );                  
                            verifyStatus = FLASH_Verify(currentWriteAddress,(uint32_t *)(&imageBuffer[k]),4);
                         }
                      
                         if (flashStatus != FLASH_COMPLETE)
                            break;
                      
                         if (verifyStatus == FLASH_COMPLETE){  
                            k+=byteIncrement;
                            currentWriteAddress += byteIncrement;
                         } else
                            break;
                      }
                      disableFlitf();
                      
                      /* prepare notification data for next expected block if for both success or notify write/verify failure */
                      if (flashStatus == FLASH_COMPLETE) {
                      
                         if (verifyStatus == FLASH_COMPLETE) {
#ifdef ST_OTA_BTL_ECHO
                         printf("Flash write memory passed \r\n");
#endif
                         totalBytesWritten+=bufPointer;
                         notification.errCode = OTA_SUCCESS;
                         /* reser buffer pointer, everything was successfully written on flash */
                         bufPointer = 0; 
                      } else {
#ifdef ST_OTA_BTL_MINIMAL_ECHO
                         printf("Flash verify failure \r\n");
#endif
                         notification.errCode = OTA_FLASH_VERIFY_ERROR;                
                      }
                   } else {
#ifdef ST_OTA_BTL_MINIMAL_ECHO
                      printf("Flash write memory failed \r\n");
#endif          
                      notification.errCode = OTA_FLASH_WRITE_ERROR;
                   }
#ifdef ST_OTA_BTL_MINIMAL_ECHO
                   printf("** Total bytes written in flash = %d **\r\n",totalBytesWritten);
#endif              
                   }/* end of BUF write management section*/
#ifdef ST_OTA_BTL_ECHO                 
                   printf("Sending notification %d \r\n",notification.replyCounter);
#endif             
                   /* Depending on outcome of code section above send notification related to: 
                    * next sequence number *OR* flash write failure *OR* verify failure 
                    */
                   ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(tHalUint8*)&notification);
                   if (ret != BLE_STATUS_SUCCESS)
                      printf("Error while updating ACC characteristic.\n");
                  
                   if (totalBytesWritten >= imageSize){
                    /* light down led on the BlueNRG platform to advertise beginning of OTA bootloading session */
#ifdef HIGHER_APP
                      SdkEvalLedOff(LED2);
#else
                      SdkEvalLedOff(LED1);
#endif 
                      //GPIO_HIGH(GPIOD,GPIO_Pin_2);
#ifdef ST_OTA_BTL_MINIMAL_ECHO
                      printf("*************JUMPING TO 0x%08X *****************\r\n",imageBase);
#endif
      
                      /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
                      if (((*(__IO uint32_t*)imageBase) & 0x2FFE0000 ) == 0x20000000){
                         DATA_EEPROM_Unlock();
                         flashStatus = DATA_EEPROM_ProgramWord(NEW_APP_MEM_INFO, imageBase);
                         flashStatus |= DATA_EEPROM_ProgramWord(NEW_APP_MEM_INFO + 4, (imageBase+imageSize));
                         DATA_EEPROM_Lock();
                         if (flashStatus == FLASH_COMPLETE){
                         /* bootloadingOngoing = 0; doesn't really matters anymore -> we are jumping towards new app */       
                         /* Reset manager will take care of running the new application*/
                         NVIC_SystemReset();
                         }
                      }   
                   }
                } /* end of notification window section */
             } else { 
                /* notify sequence number failure */
                if ((((receivedSeqNum+1) % NOTIFICATION_WINDOW) == 0) || (((receivedSeqNum+1)*16) >= imageSize)){
#ifdef ST_OTA_BTL_MINIMAL_ECHO
                   printf("Sequence number check failed, expected frame # 0x%02X but 0x%02X was received \r\n", expectedSeqNum,receivedSeqNum);              
                   printf("** Total bytes written in flash = %d **\r\n",totalBytesWritten);
#endif
                   notification.errCode = OTA_SEQUENCE_ERROR;
                   /* null packet due to seq failure : remove from internal buffer through buffer pointer shift */
                   bufPointer-=16;
                   notification.replyCounter = expectedSeqNum;
#ifdef ST_OTA_BTL_ECHO
                   printf("Sending notification %d \r\n",notification.replyCounter);
#endif            
                   ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(tHalUint8*)&notification);
                   if (ret != BLE_STATUS_SUCCESS)
                      printf("Error while updating ACC characteristic.\n");
                }
             }            
          } else { 
            /* notify checksum error */
#ifdef ST_OTA_BTL_MINIMAL_ECHO
             printf("Integrity check failed\r\n");              
             printf("**Total bytes written in flash = %d **\r\n",totalBytesWritten);
#endif
             notification.replyCounter = expectedSeqNum;
             notification.errCode = OTA_CHECKSUM_ERROR;
             /* null packet due to seq failure : remove from internal buffer through buffer pointer shift */
             bufPointer-=16;

#ifdef OTA_BTL_ECHO
             printf("Sending notification %d \r\n",notification.replyCounter);
#endif 
             ret = aci_gatt_update_char_value(btlServHandle, btlExpectedImageTUSeqNumberCharHandle, 0, 4,(tHalUint8*)&notification);
             if (ret != BLE_STATUS_SUCCESS)
                printf("Error while updating ACC characteristic.\n");
          }
          checksum = 0;
       }
    }
}/* end OTA_Write_Request_CB() */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
