/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : bluenrg_hci.h
* Author             : AMS - HEA&RF BU
* Version            : V1.0.0
* Date               : 4-Oct-2013
* Description        : Header file with HCI commands for BlueNRG FW6.0 and above.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef __BLUENRG_HCI_H__
#define __BLUENRG_HCI_H__

#include "gatt_server.h"

tBleStatus aci_gatt_init();
tBleStatus aci_gap_init(uint8_t role,
                 uint16_t* service_handle,
                 uint16_t* dev_name_char_handle,
                 uint16_t* appearance_char_handle);
tBleStatus aci_gap_set_discoverable(tHalUint8 AdvType, tHalUint16 AdvIntervMin, tHalUint16 AdvIntervMax,
                             tHalUint8 OwnAddrType, tHalUint8 AdvFilterPolicy, tHalUint8 LocalNameLen,
                             const char *LocalName, tHalUint8 ServiceUUIDLen, const tHalUint16* ServiceUUIDList,
                             tHalUint16 SlaveConnIntervMin, tHalUint16 SlaveConnIntervMax);
tBleStatus aci_gap_set_auth_requirement(uint8_t mitm_mode,
                                        uint8_t oob_enable,
                                        uint8_t oob_data[16],
                                        uint8_t min_encryption_key_size,
                                        uint8_t max_encryption_key_size,
                                        uint8_t use_fixed_pin,
                                        uint32_t fixed_pin,
                                        uint8_t bonding_mode);
tBleStatus aci_gap_create_connection(uint16_t scanInterval, uint16_t scanWindow,
				     uint8_t peer_bdaddr_type, tBDAddr peer_bdaddr,	
				     uint8_t own_bdaddr_type, uint16_t conn_min_interval,	
				     uint16_t conn_max_interval, uint16_t conn_latency,	
				     uint16_t supervision_timeout, uint16_t min_conn_length, 
				     uint16_t max_conn_length);
tBleStatus aci_gap_terminate(uint16_t conn_handle, uint8_t reason);
tBleStatus aci_gatt_add_serv(tHalUint8 service_uuid_type,
                      const tHalUint8* service_uuid,
                      tHalUint8 service_type,
                      tHalUint8 max_attr_records,
                      tHalUint16 *serviceHandle);
tBleStatus aci_gatt_add_char(tHalUint16 serviceHandle,
                      tUuidType charUuidType,
                      const tHalUint8* charUuid, 
                      tHalUint16 charValueLen, 
                      tHalUint8 charProperties,
                      tAttrSecurityFlags secPermissions,
                      tGattServerEvent gattEvtMask,
                      tHalUint8 encryKeySize,
                      tHalUint8 isVariable,
                      tHalUint16* charHandle);
tBleStatus aci_gatt_add_char_desc(tHalUint16 serviceHandle,
                                  tHalUint16 charHandle,
                                  tUuidType descUuidType,
                                  const tHalUint8* uuid, 
                                  tHalUint8 descValueMaxLen,
                                  tHalUint8 descValueLen,
                                  const void* descValue, 
                                  tAttrSecurityFlags secPermissions,
                                  tAttrSecurityFlags accPermissions,
                                  tGattServerEvent gattEvtMask,
                                  tHalUint8 encryKeySize,
                                  tHalUint8 isVariable,
                                  tHalUint16* descHandle);
tBleStatus aci_gatt_update_char_value(tHalUint16 servHandle, 
                                    tHalUint16 charHandle,
                                    tHalUint8 charValOffset,
                                    tHalUint8 charValueLen,   
                                    const tHalUint8 *charValue);

tBleStatus aci_gatt_set_desc_value(tHalUint16 servHandle, 
                                    tHalUint16 charHandle,
                                    tHalUint16 charDescHandle,
                                    tHalUint16 charDescValOffset,
                                    tHalUint8 charDescValueLen,   
                                    const tHalUint8 *charDescValue);

tBleStatus aci_hal_set_tx_power_level(uint8_t en_high_power, uint8_t pa_level);

tBleStatus aci_hal_write_config_data(tHalUint8 offset, 
                                    tHalUint8 len,
                                    const tHalUint8 *val);
tBleStatus aci_hal_tone_start(uint8_t rf_channel);

tBleStatus aci_gatt_allow_read(tHalUint16 conn_handle);

tBleStatus aci_gatt_write_charac_descriptor(uint16_t conn_handle, uint16_t attr_handle, 
					   uint8_t value_len, uint8_t *attr_value);
tBleStatus aci_gatt_write_without_response(uint16_t conn_handle, uint16_t attr_handle, 
					   uint8_t value_len, uint8_t *attr_value);

#endif /* __BLUENRG_HCI_H__ */
