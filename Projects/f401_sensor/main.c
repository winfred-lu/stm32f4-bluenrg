/**
  ******************************************************************************
  * @file    main.c
  * @author  Winfred Lu
  * @version V1.0.0
  * @date    24-June-2014
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
#include "stm32f4xx_hal.h"
#include "stm32f401_discovery.h"
#include "stm32f401_discovery_accelerometer.h"

#include "hci_internal.h"
#include "bluenrg_hci_internal.h"

#ifdef WITH_VCP
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"
#include "usbd_desc.h"

USBD_HandleTypeDef hUSBDDevice;
#endif

static void HW_Init(void)
{
  /* Init STM32F401 discovery LEDs */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);

  /* Init SPI and I2C */
  GYRO_IO_Init();
  COMPASSACCELERO_IO_Init();

  /* Init on-board AccelMag */
  BSP_ACCELERO_Init();

#ifdef WITH_VCP
  /* Init Device Library */
  USBD_Init(&hUSBDDevice, &VCP_Desc, 0);

  /* Add Supported Class */
  USBD_RegisterClass(&hUSBDDevice, &USBD_CDC);

  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&hUSBDDevice, &USBD_CDC_fops);

  /* Start Device Process */
  USBD_Start(&hUSBDDevice);
#endif
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  HAL_Init();
  HW_Init();
  HCI_Init();

  while (1)
  {
    /* Blink the orange LED */
    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
  }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

int connected = FALSE;
volatile uint8_t set_connectable = 1;
tHalUint16 connection_handle = 0;

void GAP_DisconnectionComplete_CB(void)
{
  connected = FALSE;
  //PRINTF("Disconnected\n");
  /* Make the device connectable again. */
  set_connectable = TRUE;
}

void GAP_ConnectionComplete_CB(tHalUint8 addr[6], tHalUint16 handle)
{
  connected = TRUE;
  connection_handle = handle;
  /*
  PRINTF("Connected to device:");
  for (int i = 5; i > 0; i--)
    PRINTF("%02X-", addr[i]);
  PRINTF("%02X\r\n", addr[0]);
  */
}

void Read_Request_CB(tHalUint16 handle);
void HCI_Event_CB(void *pckt)
{
  hci_uart_pckt *hci_pckt = pckt;
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

  if (hci_pckt->type != HCI_EVENT_PKT)
    return;

  switch(event_pckt->evt)
  {
  case EVT_DISCONN_COMPLETE:
    //evt_disconn_complete *evt = (void *)event_pckt->data;
    GAP_DisconnectionComplete_CB();
    break;

  case EVT_LE_META_EVENT:
  {
    evt_le_meta_event *evt = (void *)event_pckt->data;
    switch(evt->subevent)
    {
    case EVT_LE_CONN_COMPLETE:
    {
      evt_le_connection_complete *cc = (void *)evt->data;
      GAP_ConnectionComplete_CB(cc->peer_bdaddr,cc->handle);
      break;
    }
    }
    break;
  }

  case EVT_VENDOR:
  {
    evt_blue_aci *blue_evt = (void*)event_pckt->data;
    switch(blue_evt->ecode)
    {
    case EVT_BLUE_GATT_READ_PERMIT_REQ:
    {
      evt_gatt_read_permit_req *pr = (void*)blue_evt->data;
      Read_Request_CB(pr->attr_handle);
      break;
#ifdef ST_OTA_BTL
    case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
      evt_gatt_attr_modified *am = (void*)blue_evt->data;
      //PRINTF("-> modified attribute 0x%04X \r\n",am->attr_handle);
      OTA_Write_Request_CB(am);
      break;
#endif
    }
    }
    break;
  }
  }
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
