/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : bluenrg_hci.c
* Author             : AMS - HEA&RF BU
* Version            : V1.0.0
* Date               : 4-Oct-2013
* Description        : File with HCI commands for BlueNRG FW6.0 and above.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#include "hal_types.h"
#include "osal.h"
#include "ble_status.h"
#include "hal.h"
#include "osal.h"
#include "hci_internal.h"
#include "bluenrg_hci_internal.h"
#include "gatt_server.h"


tBleStatus aci_gatt_init()
{
    struct hci_request rq;
	tHalUint8 status;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GATT_INIT;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}

	return 0;
}

tBleStatus aci_gap_init(uint8_t role, uint16_t* service_handle, uint16_t* dev_name_char_handle, uint16_t* appearance_char_handle)
{
	struct hci_request rq;
    gap_init_cp cp;
	gap_init_rp resp;

    cp.role = role;
    
	Osal_MemSet(&resp, 0, sizeof(resp));

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GAP_INIT;
	rq.cparam = &cp;
	rq.clen = GAP_INIT_CP_SIZE;
	rq.rparam = &resp;
	rq.rlen = GAP_INIT_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (resp.status) {
		return resp.status;
	}
	
	*service_handle = resp.service_handle;
	*dev_name_char_handle = resp.dev_name_char_handle;
    *appearance_char_handle = resp.appearance_char_handle;

	return 0;
}

tBleStatus aci_gap_set_discoverable(tHalUint8 AdvType, tHalUint16 AdvIntervMin, tHalUint16 AdvIntervMax,
                             tHalUint8 OwnAddrType, tHalUint8 AdvFilterPolicy, tHalUint8 LocalNameLen,
                             const char *LocalName, tHalUint8 ServiceUUIDLen, const tHalUint16* ServiceUUIDList,
                             tHalUint16 SlaveConnIntervMin, tHalUint16 SlaveConnIntervMax)
{
	struct hci_request rq;
	uint8_t status;    
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t indx = 0;
    
    buffer[indx] = AdvType;
    indx++;
    
    Osal_MemCpy(buffer + indx, &AdvIntervMin, 2);
    indx +=  2;
    
    Osal_MemCpy(buffer + indx, &AdvIntervMax, 2);
    indx +=  2;
    
    buffer[indx] = OwnAddrType;
    indx++;
    
    buffer[indx] = AdvFilterPolicy;
    indx++;
    
    buffer[indx] = LocalNameLen;
    indx++;
    
    Osal_MemCpy(buffer + indx, LocalName, LocalNameLen);
    indx +=  LocalNameLen;

    buffer[indx] = ServiceUUIDLen;
    indx++;
    
    Osal_MemCpy(buffer + indx, ServiceUUIDList, ServiceUUIDLen);
    indx +=  ServiceUUIDLen;

    Osal_MemCpy(buffer + indx, &SlaveConnIntervMin, 2);
    indx +=  2;
    
    Osal_MemCpy(buffer + indx, &SlaveConnIntervMax, 2);
    indx +=  2;    

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GAP_SET_DISCOVERABLE;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
    rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}

	return 0;
}

tBleStatus aci_gap_update_adv_data(tHalUint8 AdvLen, tHalUint8 *AdvData)
{
	struct hci_request rq;
	uint8_t status;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t indx = 0;
    
    buffer[indx] = AdvLen;
    indx++;
    
    Osal_MemCpy(buffer + indx, AdvData, AdvLen);
    indx +=  AdvLen;
    
	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GAP_UPDATE_ADV_DATA;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
    rq.rparam = &status;
	rq.rlen = 1;
    
	if (hci_send_req(&rq) < 0)
		return -1;
    
	return status;
}


tBleStatus aci_gatt_add_serv(tHalUint8 service_uuid_type, const tHalUint8* service_uuid, tHalUint8 service_type, tHalUint8 max_attr_records, tHalUint16 *serviceHandle)
{
	struct hci_request rq;
    gatt_add_serv_rp resp;    
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t uuid_len;
    uint8_t indx = 0;
    
    buffer[indx] = service_uuid_type;
    indx++;
    
    if(service_uuid_type == 0x01){
        uuid_len = 2;
    }
    else {
        uuid_len = 16;
    }        
    Osal_MemCpy(buffer + indx, service_uuid, uuid_len);
    indx +=  uuid_len;
    
    buffer[indx] = service_type;
    indx++;
    
    buffer[indx] = max_attr_records;
    indx++;
    
    
    Osal_MemSet(&resp, 0, sizeof(resp));

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GATT_ADD_SERV;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
	rq.rparam = &resp;
	rq.rlen = GATT_ADD_SERV_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (resp.status) {
		return resp.status;
	}
    
    *serviceHandle = resp.handle;

	return 0;
}

tBleStatus aci_gatt_add_char(tHalUint16 serviceHandle,
                      tUuidType charUuidType,
                      const tHalUint8* charUuid, 
                      tHalUint16 charValueLen, 
                      tHalUint8 charProperties,
                      tAttrSecurityFlags secPermissions,
                      tGattServerEvent gattEvtMask,
                      tHalUint8 encryKeySize,
                      tHalUint8 isVariable,
                      tHalUint16* charHandle)
                      
                      
{
	struct hci_request rq;
    gatt_add_serv_rp resp;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t uuid_len;
    uint8_t indx = 0;
    
    Osal_MemCpy(buffer + indx, &serviceHandle, 2);
    indx += 2;
    
    buffer[indx] = charUuidType;
    indx++;
    
    if(charUuidType == 0x01){
        uuid_len = 2;
    }
    else {
        uuid_len = 16;
    }        
    Osal_MemCpy(buffer + indx, charUuid, uuid_len);
    indx +=  uuid_len;
    
    buffer[indx] = charValueLen;
    indx++;
    
    buffer[indx] = charProperties;
    indx++;
    
    buffer[indx] = secPermissions;
    indx++;
    
    buffer[indx] = gattEvtMask;
    indx++;
    
    buffer[indx] = encryKeySize;
    indx++;
    
    buffer[indx] = isVariable;
    indx++;
    
    Osal_MemSet(&resp, 0, sizeof(resp));

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GATT_ADD_CHAR;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
	rq.rparam = &resp;
	rq.rlen = GATT_ADD_CHAR_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (resp.status) {
		return resp.status;
	}
    
    *charHandle = resp.handle;

	return 0;
}

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
                                  tHalUint16* descHandle)
                      
                      
{
	struct hci_request rq;
    gatt_add_char_desc_rp resp;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t uuid_len;
    uint8_t indx = 0;
    
    Osal_MemCpy(buffer + indx, &serviceHandle, 2);
    indx += 2;
    
    Osal_MemCpy(buffer + indx, &charHandle, 2);
    indx += 2;
    
    buffer[indx] = descUuidType;
    indx++;
    
    if(descUuidType == 0x01){
        uuid_len = 2;
    }
    else {
        uuid_len = 16;
    }        
    Osal_MemCpy(buffer + indx, uuid, uuid_len);
    indx +=  uuid_len;
    
    buffer[indx] = descValueMaxLen;
    indx++;
    
    buffer[indx] = descValueLen;
    indx++;
    
    Osal_MemCpy(buffer + indx, descValue, descValueLen);
    indx += descValueLen;
    
    buffer[indx] = secPermissions;
    indx++;
    
    buffer[indx] = accPermissions;
    indx++;
    
    buffer[indx] = gattEvtMask;
    indx++;
    
    buffer[indx] = encryKeySize;
    indx++;
    
    buffer[indx] = isVariable;
    indx++;
    
    Osal_MemSet(&resp, 0, sizeof(resp));

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GATT_ADD_CHAR_DESC;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
	rq.rparam = &resp;
	rq.rlen = GATT_ADD_CHAR_DESC_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (resp.status) {
		return resp.status;
	}
    
    *descHandle = resp.handle;

	return 0;
}


tBleStatus aci_gatt_update_char_value(tHalUint16 servHandle, 
                                    tHalUint16 charHandle,
                                    tHalUint8 charValOffset,
                                    tHalUint8 charValueLen,   
                                    const tHalUint8 *charValue)
{
	struct hci_request rq;
	uint8_t status;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t indx = 0;
    
    Osal_MemCpy(buffer + indx, &servHandle, 2);
    indx += 2;
    
    Osal_MemCpy(buffer + indx, &charHandle, 2);
    indx += 2;
    
    buffer[indx] = charValOffset;
    indx++;
    
    buffer[indx] = charValueLen;
    indx++;
        
    Osal_MemCpy(buffer + indx, charValue, charValueLen);
    indx +=  charValueLen;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GATT_UPD_CHAR_VAL;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}

	return 0;
}

tBleStatus aci_gatt_allow_read(tHalUint16 conn_handle)
{
    struct hci_request rq;
    gatt_allow_read_cp cp;
	tHalUint8 status;
    
    cp.conn_handle = conn_handle;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GATT_ALLOW_READ;
    rq.cparam = &cp;
	rq.clen = GATT_ALLOW_READ_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}

	return 0;
}

tBleStatus aci_gatt_set_desc_value(tHalUint16 servHandle, 
                                    tHalUint16 charHandle,
                                    tHalUint16 charDescHandle,
                                    tHalUint16 charDescValOffset,
                                    tHalUint8 charDescValueLen,   
                                    const tHalUint8 *charDescValue)
{
	struct hci_request rq;
	uint8_t status;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t indx = 0;
    
    Osal_MemCpy(buffer + indx, &servHandle, 2);
    indx += 2;
    
    Osal_MemCpy(buffer + indx, &charHandle, 2);
    indx += 2;
    
    Osal_MemCpy(buffer + indx, &charDescHandle, 2);
    indx += 2;
    
    Osal_MemCpy(buffer + indx, &charDescValOffset, 2);
    indx += 2;
    
    buffer[indx] = charDescValueLen;
    indx++;
        
    Osal_MemCpy(buffer + indx, charDescValue, charDescValueLen);
    indx +=  charDescValueLen;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GATT_SET_DESC_VAL;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}

	return 0;
}

tBleStatus aci_gatt_write_without_response(uint16_t conn_handle, uint16_t attr_handle, 
					   uint8_t value_len, uint8_t *attr_value)
{
  struct hci_request rq;
  uint8_t status;
  uint8_t buffer[HCI_MAX_PACKET_SIZE];
  uint8_t indx = 0;
    
  Osal_MemCpy(buffer + indx, &conn_handle, 2);
  indx += 2;
    
  Osal_MemCpy(buffer + indx, &attr_handle, 2);
  indx += 2;

  buffer[indx] = value_len;
  indx++;
        
  Osal_MemCpy(buffer + indx, attr_value, value_len);
  indx +=  value_len;

  Osal_MemSet(&rq, 0, sizeof(rq));
  rq.ogf = OGF_VENDOR_CMD;
  rq.ocf = OCF_GATT_WRITE_WITHOUT_RESPONSE;
  rq.cparam = (void *)buffer;
  rq.clen = indx;
  rq.rparam = &status;
  rq.rlen = 1;

  if (hci_send_req(&rq) < 0)
    return -1;

  return status;

}

tBleStatus aci_gatt_write_charac_descriptor(uint16_t conn_handle, uint16_t attr_handle, 
					   uint8_t value_len, uint8_t *attr_value)
{
  struct hci_request rq;
  uint8_t status;
  uint8_t buffer[HCI_MAX_PACKET_SIZE];
  uint8_t indx = 0;
    
  Osal_MemCpy(buffer + indx, &conn_handle, 2);
  indx += 2;
    
  Osal_MemCpy(buffer + indx, &attr_handle, 2);
  indx += 2;

  buffer[indx] = value_len;
  indx++;
        
  Osal_MemCpy(buffer + indx, attr_value, value_len);
  indx +=  value_len;

  Osal_MemSet(&rq, 0, sizeof(rq));
  rq.ogf = OGF_VENDOR_CMD;
  rq.ocf = OCF_GATT_WRITE_CHAR_DESCRIPTOR;
  rq.cparam = (void *)buffer;
  rq.clen = indx;
  rq.rparam = &status;
  rq.rlen = 1;

  if (hci_send_req(&rq) < 0)
    return -1;

  return status;

}

tBleStatus aci_hal_write_config_data(tHalUint8 offset, 
                                    tHalUint8 len,
                                    const tHalUint8 *val)
{
	struct hci_request rq;
	uint8_t status;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t indx = 0;
    
    buffer[indx] = offset;
    indx++;
    
    buffer[indx] = len;
    indx++;
        
    Osal_MemCpy(buffer + indx, val, len);
    indx +=  len;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_HAL_WRITE_CONFIG_DATA;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}

	return 0;
}

tBleStatus aci_hal_set_tx_power_level(uint8_t en_high_power, uint8_t pa_level)
{
	struct hci_request rq;
    hal_set_tx_power_level_cp cp;    
	uint8_t status;
    
    cp.en_high_power = en_high_power;
    cp.pa_level = pa_level;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_HAL_SET_TX_POWER_LEVEL;
	rq.cparam = &cp;
	rq.clen = HAL_SET_TX_POWER_LEVEL_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}

	return 0;
}

tBleStatus aci_gap_set_auth_requirement(uint8_t mitm_mode,
                                        uint8_t oob_enable,
                                        uint8_t oob_data[16],
                                        uint8_t min_encryption_key_size,
                                        uint8_t max_encryption_key_size,
                                        uint8_t use_fixed_pin,
                                        uint32_t fixed_pin,
                                        uint8_t bonding_mode)
{
	struct hci_request rq;
    gap_set_auth_requirement_cp cp;    
	uint8_t status;
    
    cp.mitm_mode = mitm_mode;
    cp.oob_enable = oob_enable;
    Osal_MemCpy(cp.oob_data, oob_data, 16);
    cp.min_encryption_key_size = min_encryption_key_size;
    cp.max_encryption_key_size = max_encryption_key_size;
    cp.use_fixed_pin = use_fixed_pin;
    cp.fixed_pin = fixed_pin;
    cp.bonding_mode = bonding_mode;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GAP_SET_AUTH_REQUIREMENT;
	rq.cparam = &cp;
	rq.clen = GAP_SET_AUTH_REQUIREMENT_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}
    
    return 0;

}

tBleStatus aci_gap_create_connection(uint16_t scanInterval, uint16_t scanWindow,
				     uint8_t peer_bdaddr_type, tBDAddr peer_bdaddr,	
				     uint8_t own_bdaddr_type, uint16_t conn_min_interval,	
				     uint16_t conn_max_interval, uint16_t conn_latency,	
				     uint16_t supervision_timeout, uint16_t min_conn_length, 
				     uint16_t max_conn_length)
{
  struct hci_request rq;
  gap_create_connection_cp cp;
  uint8_t status;  

  cp.scanInterval = scanInterval;
  cp.scanWindow = scanWindow;
  cp.peer_bdaddr_type = peer_bdaddr_type;
  Osal_MemCpy(cp.peer_bdaddr, peer_bdaddr, 6);
  cp.own_bdaddr_type = own_bdaddr_type;
  cp.conn_min_interval = conn_min_interval;
  cp.conn_max_interval = conn_max_interval;
  cp.conn_latency = conn_latency;
  cp.supervision_timeout = supervision_timeout;
  cp.min_conn_length = min_conn_length;
  cp.max_conn_length = max_conn_length;

  Osal_MemSet(&rq, 0, sizeof(rq));
  rq.ogf = OGF_VENDOR_CMD;
  rq.ocf = OCF_GAP_CREATE_CONNECTION;
  rq.cparam = &cp;
  rq.clen = GAP_CREATE_CONNECTION_CP_SIZE;
  rq.event = EVT_CMD_STATUS;
  rq.rparam = &status;
  rq.rlen = 1;
  
  if (hci_send_req(&rq) < 0)
    return -1;

  return status;
}

tBleStatus aci_gap_terminate(uint16_t conn_handle, uint8_t reason)
{
  struct hci_request rq;
  gap_terminate_cp cp;
  uint8_t status;  

  cp.handle = conn_handle;
  cp.reason = reason;

  Osal_MemSet(&rq, 0, sizeof(rq));
  rq.ogf = OGF_VENDOR_CMD;
  rq.ocf = OCF_GAP_TERMINATE;
  rq.cparam = &cp;
  rq.clen = GAP_TERMINATE_CP_SIZE;
  rq.event = EVT_CMD_STATUS;
  rq.rparam = &status;
  rq.rlen = 1;
  
  if (hci_send_req(&rq) < 0)
    return -1;

  return status; 
}

tBleStatus aci_hal_tone_start(uint8_t rf_channel)
{
	struct hci_request rq;
    hal_tone_start_cp cp;    
	uint8_t status;
    
    cp.rf_channel = rf_channel;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_HAL_TONE_START;
	rq.cparam = &cp;
	rq.clen = HAL_TONE_START_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	if (status) {
		return status;
	}
    
    return 0;
}

int aci_updater_start()
{
  	struct hci_request rq;
	tHalUint8 status;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_START;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	return status;  
}

int aci_updater_reboot()
{
  	struct hci_request rq;
	tHalUint8 status;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_REBOOT;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	return status;  
}

int aci_get_updater_version(uint8_t *version)
{
	struct hci_request rq;
	get_updater_version_rp resp;

	Osal_MemSet(&resp, 0, sizeof(resp));

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GET_UPDATER_VERSION;
	rq.rparam = &resp;
	rq.rlen = GET_UPDATER_VERSION_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;
    
    *version = resp.version;

    return resp.status;
}

int aci_get_updater_buffer_size(uint8_t *buffer_size)
{
	struct hci_request rq;
	get_updater_bufsize_rp resp;

	Osal_MemSet(&resp, 0, sizeof(resp));

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_GET_UPDATER_BUFSIZE;
	rq.rparam = &resp;
	rq.rlen = GET_UPDATER_BUFSIZE_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;
    
    *buffer_size = resp.buffer_size;

    return resp.status;
}

int aci_erase_blue_flag()
{
  	struct hci_request rq;
	tHalUint8 status;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_ERASE_BLUE_FLAG;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	return status;  
}

int aci_reset_blue_flag()
{
  	struct hci_request rq;
	tHalUint8 status;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_RESET_BLUE_FLAG;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	return status;  
}

int aci_updater_erase_sector(uint32_t address)
{
	struct hci_request rq;
    updater_erase_sector_cp cp;    
	uint8_t status;
    
    cp.address = address;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_ERASE_SECTOR;
	rq.cparam = &cp;
	rq.clen = UPDATER_ERASE_SECTOR_CP_SIZE;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;

	return status;
}

int aci_updater_program_data_block(uint32_t address, 
                                    uint16_t len,
                                    const uint8_t *data)
{
	struct hci_request rq;
	uint8_t status;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    uint8_t indx = 0;
    
    if(len > HCI_MAX_PACKET_SIZE)
        return -1;
    
    Osal_MemCpy(buffer + indx, &address, 4);
    indx += 4;
    
    Osal_MemCpy(buffer + indx, &len, 2);
    indx += 2;
        
    Osal_MemCpy(buffer + indx, data, len);
    indx +=  len;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_PROG_DATA_BLOCK;
	rq.cparam = (void *)buffer;
	rq.clen = indx;
	rq.rparam = &status;
	rq.rlen = 1;

	if (hci_send_req(&rq) < 0)
		return -1;
    
	return status;
}

int aci_updater_read_data_block(uint32_t address,
                                       uint16_t data_len,
                                       uint8_t *data)
{
	struct hci_request rq;
	updater_read_data_block_cp cp;
    uint8_t buffer[HCI_MAX_PACKET_SIZE];
    
    if(data_len > HCI_MAX_PACKET_SIZE - 1)
        return -1;
    
    cp.address = address;
    cp.data_len = data_len;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_READ_DATA_BLOCK;
	rq.cparam = &cp;
	rq.clen = UPDATER_READ_DATA_BLOCK_CP_SIZE;
	rq.rparam = buffer;
	rq.rlen = data_len + 1;

	if (hci_send_req(&rq) < 0)
		return -1;
    
    Osal_MemCpy(data, buffer+1, data_len);

	return buffer[0];
}

int aci_updater_calc_crc(uint32_t address,
                         uint8_t num_sectors,
                         uint32_t *crc)
{
	struct hci_request rq;
    updater_calc_crc_cp cp;
    updater_calc_crc_rp resp;
    
    Osal_MemSet(&resp, 0, sizeof(resp));
    
    cp.address = address;
    cp.num_sectors = num_sectors;

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_CALC_CRC;
	rq.cparam = &cp;
	rq.clen = UPDATER_CALC_CRC_CP_SIZE;
	rq.rparam = &resp;
	rq.rlen = UPDATER_CALC_CRC_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;
    
    *crc = resp.crc;
    
    return resp.status;
}

int aci_updater_hw_version(uint8_t *version)
{
	struct hci_request rq;
	updater_hw_version_rp resp;

	Osal_MemSet(&resp, 0, sizeof(resp));

	Osal_MemSet(&rq, 0, sizeof(rq));
	rq.ogf = OGF_VENDOR_CMD;
	rq.ocf = OCF_UPDATER_HW_VERSION;
	rq.rparam = &resp;
	rq.rlen = UPDATER_HW_VERSION_RP_SIZE;

	if (hci_send_req(&rq) < 0)
		return -1;
    
    *version = resp.version;

    return resp.status;
}
