/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : gatt_db.c
* Author             : AMS - HEA&RF BU
* Version            : V1.0.0
* Date               : 04-Oct-2013
* Description        : Functions to build GATT DB and handle GATT events.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


#include "hal_types.h"
#include "gatt_server.h"
#include "gap.h"
#include "string.h"
#include "bluenrg_hci.h"
#include "hci_internal.h"
#include <stdio.h>
#include <lis3dh_driver.h>
#include <stlm75.h>
#include <HTS221.h>
#include <LPS25H.h>
#include <hal.h>
#include <stdlib.h>
#include <hw_config.h>
#include <gp_timer.h>
#include <platform_config.h>

#include "SDK_EVAL_Spi_Driver.h"
#include "SDK_EVAL_Config.h"

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif


#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
        uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
            uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
                uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
}while(0)


#define COPY_ACC_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x02,0x36,0x6e,0x80, 0xcf,0x3a, 0x11,0xe1, 0x9a,0xb4, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_FREE_FALL_UUID(uuid_struct)    COPY_UUID_128(uuid_struct,0xe2,0x3e,0x78,0xa0, 0xcf,0x4a, 0x11,0xe1, 0x8f,0xfc, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_ACC_UUID(uuid_struct)          COPY_UUID_128(uuid_struct,0x34,0x0a,0x1b,0x80, 0xcf,0x4b, 0x11,0xe1, 0xac,0x36, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)

#define COPY_ENV_SENS_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x42,0x82,0x1a,0x40, 0xe4,0x77, 0x11,0xe2, 0x82,0xd0, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_TEMP_CHAR_UUID(uuid_struct)         COPY_UUID_128(uuid_struct,0xa3,0x2e,0x55,0x20, 0xe4,0x77, 0x11,0xe2, 0xa9,0xe3, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_PRESS_CHAR_UUID(uuid_struct)        COPY_UUID_128(uuid_struct,0xcd,0x20,0xc4,0x80, 0xe4,0x8b, 0x11,0xe2, 0x84,0x0b, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)
#define COPY_HUMIDITY_CHAR_UUID(uuid_struct)     COPY_UUID_128(uuid_struct,0x01,0xc5,0x0b,0x60, 0xe4,0x8c, 0x11,0xe2, 0xa0,0x73, 0x00,0x02,0xa5,0xd5,0xc5,0x1b)



tHalUint16 accServHandle, freeFallCharHandle, accCharHandle;
tHalUint16 envSensServHandle, tempCharHandle, pressCharHandle, humidityCharHandle;

volatile tHalUint8 request_free_fall_notify = FALSE;
extern tHalUint16 connection_handle;
extern bool sensor_board;

/*******************************************************************************
* Function Name  : Add_Chat_Service
* Description    : Add the 'Accelerometer' service.
* Input          : None
* Return         : Status.
*******************************************************************************/
tBleStatus Add_Acc_Service(void)
{
    tBleStatus ret;
    uint8_t uuid[16];
    
    COPY_ACC_SERVICE_UUID(uuid);
    ret = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 7, &accServHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;    
    
    COPY_FREE_FALL_UUID(uuid);
    ret =  aci_gatt_add_char(accServHandle, UUID_TYPE_128, uuid, 1, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0,
                             16, 0, &freeFallCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;
    
    COPY_ACC_UUID(uuid);  
    ret =  aci_gatt_add_char(accServHandle, UUID_TYPE_128, uuid, 6, CHAR_PROP_NOTIFY|CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT,
                             16, 0, &accCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;
    
    PRINTF("Service ACC added. Handle 0x%04X, Free fall Charac handle: 0x%04X, Acc Charac handle: 0x%04X\n",accServHandle, freeFallCharHandle, accCharHandle);	
    return BLE_STATUS_SUCCESS; 
    
fail:
    PRINTF("Error while adding ACC service.\n");
    return BLE_STATUS_ERROR ;
    
}

/*******************************************************************************
* Function Name  : Free_Fall_Notify
* Description    : Send a notification for a Free Fall detection.
* Input          : None
* Return         : Status.
*******************************************************************************/
tBleStatus Free_Fall_Notify(void)
{  
  	tHalUint8 val;
	tBleStatus ret;
	
	val = 0x01;	
    ret = aci_gatt_update_char_value(accServHandle, freeFallCharHandle, 0, 1, &val);
	
	if (ret != BLE_STATUS_SUCCESS){
		PRINTF("Error while updating ACC characteristic.\n") ;
		return BLE_STATUS_ERROR ;
	}
	return BLE_STATUS_SUCCESS;
	
}

/*******************************************************************************
* Function Name  : Acc_Update
* Description    : Update acceleration characteristic value
* Input          : AxesRaw_t structure containing acceleration value in mg.
* Return         : Status.
*******************************************************************************/
tBleStatus Acc_Update(AxesRaw_t *data)
{  
	tBleStatus ret;    
    tHalUint8 buff[6];
    
    HOST_TO_LE_16(buff,data->AXIS_X);
    HOST_TO_LE_16(buff+2,data->AXIS_Y);
    HOST_TO_LE_16(buff+4,data->AXIS_Z);
	
    ret = aci_gatt_update_char_value(accServHandle, accCharHandle, 0, 6, buff);
	
	if (ret != BLE_STATUS_SUCCESS){
		PRINTF("Error while updating ACC characteristic.\n") ;
		return BLE_STATUS_ERROR ;
	}
	return BLE_STATUS_SUCCESS;
	
}

/*******************************************************************************
* Function Name  : Add_Environmental_Sensor_Service
* Description    : Add the Environmental Sensor service.
* Input          : None
* Return         : Status.
*******************************************************************************/
tBleStatus Add_Environmental_Sensor_Service(void)
{
  	tBleStatus ret;
    uint8_t uuid[16];
    uint16_t uuid16;
    charactFormat charFormat;
    tHalUint16 descHandle;
    
    COPY_ENV_SENS_SERVICE_UUID(uuid);
    ret = aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 10, &envSensServHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;
    
#if 1    
    /* Temperature Characteristic */
    
    COPY_TEMP_CHAR_UUID(uuid);  
    ret =  aci_gatt_add_char(envSensServHandle, UUID_TYPE_128, uuid, 2, CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT,
                             16, 0, &tempCharHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;
    
    charFormat.format = FORMAT_SINT16;
    charFormat.exp = -1;
    charFormat.unit = UNIT_TEMP_CELSIUS;
    charFormat.name_space = 0;
    charFormat.desc = 0;
    
    uuid16 = CHAR_FORMAT_DESC_UUID;
    
    ret = aci_gatt_add_char_desc(envSensServHandle,
                                 tempCharHandle,
                                 UUID_TYPE_16,
                                 (tHalUint8 *)&uuid16, 
                                 7,
                                 7,
                                 (void *)&charFormat, 
                                 ATTR_PERMISSION_NONE,
                                 ATTR_ACCESS_READ_ONLY,
                                 0,
                                 16,
                                 FALSE,
                                 &descHandle);
    if (ret != BLE_STATUS_SUCCESS) goto fail;
#endif    
    /* Pressure Characteristic */
    if(sensor_board){
        COPY_PRESS_CHAR_UUID(uuid);  
        ret =  aci_gatt_add_char(envSensServHandle, UUID_TYPE_128, uuid, 3, CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT,
                                 16, 0, &pressCharHandle);
        if (ret != BLE_STATUS_SUCCESS) goto fail;
        
        charFormat.format = FORMAT_SINT24;
        charFormat.exp = -5;
        charFormat.unit = UNIT_PRESSURE_BAR;
        charFormat.name_space = 0;
        charFormat.desc = 0;
        
        uuid16 = CHAR_FORMAT_DESC_UUID;
        
        ret = aci_gatt_add_char_desc(envSensServHandle,
                                     pressCharHandle,
                                     UUID_TYPE_16,
                                     (tHalUint8 *)&uuid16, 
                                     7,
                                     7,
                                     (void *)&charFormat, 
                                     ATTR_PERMISSION_NONE,
                                     ATTR_ACCESS_READ_ONLY,
                                     0,
                                     16,
                                     FALSE,
                                     &descHandle);
        if (ret != BLE_STATUS_SUCCESS) goto fail;
    }    
    /* Humidity Characteristic */
    if(sensor_board){   
        COPY_HUMIDITY_CHAR_UUID(uuid);  
        ret =  aci_gatt_add_char(envSensServHandle, UUID_TYPE_128, uuid, 2, CHAR_PROP_READ, ATTR_PERMISSION_NONE, GATT_INTIMATE_APPL_WHEN_READ_N_WAIT,
                                 16, 0, &humidityCharHandle);
        if (ret != BLE_STATUS_SUCCESS) goto fail;
        
        charFormat.format = FORMAT_UINT16;
        charFormat.exp = -1;
        charFormat.unit = UNIT_UNITLESS;
        charFormat.name_space = 0;
        charFormat.desc = 0;
        
        uuid16 = CHAR_FORMAT_DESC_UUID;
        
        ret = aci_gatt_add_char_desc(envSensServHandle,
                                     humidityCharHandle,
                                     UUID_TYPE_16,
                                     (tHalUint8 *)&uuid16, 
                                     7,
                                     7,
                                     (void *)&charFormat, 
                                     ATTR_PERMISSION_NONE,
                                     ATTR_ACCESS_READ_ONLY,
                                     0,
                                     16,
                                     FALSE,
                                     &descHandle);
        if (ret != BLE_STATUS_SUCCESS) goto fail;
    } 
	PRINTF("Service ENV_SENS added. Handle 0x%04X, TEMP Charac handle: 0x%04X, PRESS Charac handle: 0x%04X, HUMID Charac handle: 0x%04X\n",envSensServHandle, tempCharHandle, pressCharHandle, humidityCharHandle);	
	return BLE_STATUS_SUCCESS; 
	
fail:
  	PRINTF("Error while adding ENV_SENS service.\n");
	return BLE_STATUS_ERROR ;
    
}

/*******************************************************************************
* Function Name  : Temp_Update
* Description    : Update temperature characteristic value
* Input          : temperature in tenths of degree
* Return         : Status.
*******************************************************************************/
tBleStatus Temp_Update(int16_t temp)
{  
	tBleStatus ret;
	
    ret = aci_gatt_update_char_value(envSensServHandle, tempCharHandle, 0, 2, (tHalUint8*)&temp);
	
	if (ret != BLE_STATUS_SUCCESS){
		PRINTF("Error while updating TEMP characteristic.\n") ;
		return BLE_STATUS_ERROR ;
	}
	return BLE_STATUS_SUCCESS;
	
}

/*******************************************************************************
* Function Name  : Press_Update
* Description    : Update pressure characteristic value
* Input          : Pressure in mbar
* Return         : Status.
*******************************************************************************/
tBleStatus Press_Update(int32_t press)
{  
	tBleStatus ret;
	
    ret = aci_gatt_update_char_value(envSensServHandle, pressCharHandle, 0, 3, (tHalUint8*)&press);
	
	if (ret != BLE_STATUS_SUCCESS){
		PRINTF("Error while updating TEMP characteristic.\n") ;
		return BLE_STATUS_ERROR ;
	}
	return BLE_STATUS_SUCCESS;
	
}

/*******************************************************************************
* Function Name  : Humidity_Update
* Description    : Update humidity characteristic value
* Input          : RH (Relative Humidity) in thenths of %.
* Return         : Status.
*******************************************************************************/
tBleStatus Humidity_Update(uint16_t humidity)
{  
	tBleStatus ret;
	
    ret = aci_gatt_update_char_value(envSensServHandle, humidityCharHandle, 0, 2, (tHalUint8*)&humidity);
	
	if (ret != BLE_STATUS_SUCCESS){
		PRINTF("Error while updating TEMP characteristic.\n") ;
		return BLE_STATUS_ERROR ;
	}
	return BLE_STATUS_SUCCESS;
	
}

void Read_Request_CB(tHalUint16 handle)
{
    AxesRaw_t data;
    int response = 1;
    //signed short refvalue;
    
    if(handle == accCharHandle + 1){
        response = LIS3DH_GetAccAxesRaw(&data);
        if(response){
            LIS3DH_ConvAccValue(&data);
            Acc_Update(&data);
        }
        else {
            // Error while reading from accelerometer
#if !defined(ST_OTA_BTL) && !defined(ST_OTA_BASIC_APPLICATION)
            SdkEvalLedOn(LED1);
#endif
        }
    }
    else if(handle == tempCharHandle + 1){
        int16_t data;
#ifdef SENSOR_EMULATION
        data = 270 + ((uint64_t)rand()*15)/RAND_MAX;
#else
        if(sensor_board){
            response = Lps25hStartNReadTemperature(&data);
        }
        else {
            response = STLM75_Read_Temperature_Signed(&data);
        }
#endif
        
        if(response){
            Temp_Update(data);
        }
        else{
            PRINTF("Temp Error\n");
            
#if !defined(ST_OTA_BTL) && !defined(ST_OTA_BASIC_APPLICATION)
            SdkEvalLedOn(LED1);
#endif
        }   
    }
    else if(handle == pressCharHandle + 1){
        int32_t data;
        struct timer t;  
        Timer_Set(&t, CLOCK_SECOND/10);
        
#ifdef SENSOR_EMULATION
        data = 100000 + ((uint64_t)rand()*1000)/RAND_MAX;
#else      
        response = Lps25hReadPressure(&data);
#endif 
        if(response)
            Press_Update(data);
    }
    else if(handle == humidityCharHandle + 1){
        uint16_t data;
#ifdef SENSOR_EMULATION      
        data = 450 + ((uint64_t)rand()*100)/RAND_MAX;
#else
        HTS221_Read_Humidity(&data);
#endif      
        if(response){
            Humidity_Update(data);
        }
        else{
            PRINTF("HTS221 Error\n");
#if !defined(ST_OTA_BTL) && !defined(ST_OTA_BASIC_APPLICATION)
            SdkEvalLedOn(LED1);
#endif
        }
    }  
    
//EXIT:
    if(connection_handle !=0)
        aci_gatt_allow_read(connection_handle);
}


