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
#include <stdio.h>
#include <string.h>

#include "stm32f401_discovery.h"
#include "stm32f401_discovery_accelerometer.h"

#include "hci_internal.h"
#include "bluenrg_hci_internal.h"
#include "gap.h"
#include "sm.h"

#ifdef WITH_VCP
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"
#include "usbd_desc.h"

USBD_HandleTypeDef hUSBDDevice;
#endif


int connected = FALSE;
volatile uint8_t set_connectable = 1;
tHalUint16 connection_handle = 0;


#ifdef WITH_USART
USART_HandleTypeDef UsartHandle;

void HAL_USART_MspInit(USART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable GPIO TX/RX and USART clock */
  __GPIOA_CLK_ENABLE();
  __USART2_CLK_ENABLE();

  /* USART TX/RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* NVIC for USART */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

static void ColorfulRingOfDeath(void)
{
  uint16_t ring = 1;
  while (1)
  {
    uint32_t count = 0;
    while (count++ < 100000)
      ;
    GPIOD->BSRRH = (ring << 12);
    ring = ring << 1;
    if (ring >= 1<<4)
      ring = 1;
    GPIOD->BSRRL = (ring << 12);
  }
}
#endif

static void HW_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

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

  /* Init BlueNRG CS, Reset, and IRQ pin */
  BLUENRG_CS_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin   = BLUENRG_CS_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(BLUENRG_CS_GPIO_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BLUENRG_CS_GPIO_PORT, BLUENRG_CS_PIN, GPIO_PIN_SET);

  BLUENRG_RESET_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin   = BLUENRG_RESET_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(BLUENRG_RESET_GPIO_PORT, &GPIO_InitStruct);

  BLUENRG_IRQ_GPIO_CLK_ENABLE();
  GPIO_InitStruct.Pin   = BLUENRG_IRQ_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(BLUENRG_IRQ_GPIO_PORT, &GPIO_InitStruct);

#ifdef WITH_USART
  /* Init USART port */
  UsartHandle.Instance        = USART2;
  UsartHandle.Init.BaudRate   = 9600;
  UsartHandle.Init.WordLength = USART_WORDLENGTH_8B;
  UsartHandle.Init.StopBits   = USART_STOPBITS_1;
  UsartHandle.Init.Parity     = USART_PARITY_NONE;
  UsartHandle.Init.Mode       = USART_MODE_TX_RX;
  if (HAL_USART_Init(&UsartHandle) != HAL_OK)
  {
    ColorfulRingOfDeath();
  }
#endif

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

static void BlueNRG_Init()
{
  int rc;
  uint8_t bdaddr[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x02};
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  const char *ble_name = "BlueNRG";

  BSP_LED_On(LED3);
  HCI_Init();

  /* Enable and set EXTI for BlueNRG IRQ */
  HAL_NVIC_SetPriority(BLUENRG_IRQ_EXTI_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(BLUENRG_IRQ_EXTI_IRQn);

  BlueNRG_RST();

  BSP_LED_On(LED4);
  rc = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                 CONFIG_DATA_PUBADDR_LEN, bdaddr);
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED4);
  }

  BSP_LED_On(LED5);
  rc = aci_gatt_init();
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED5);
  }

  BSP_LED_On(LED6);
  rc = aci_gap_init(1, &service_handle, &dev_name_char_handle,
                    &appearance_char_handle);
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED6);
  }

  BSP_LED_Off(LED3);
  rc = aci_gatt_update_char_value(service_handle, dev_name_char_handle,
                                  0, strlen(ble_name), (uint8_t *)ble_name);
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED3);
  }

  BSP_LED_Off(LED4);
  rc = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                    OOB_AUTH_DATA_ABSENT,
                                    NULL,
                                    7,
                                    16,
                                    USE_FIXED_PIN_FOR_PAIRING,
                                    123456,
                                    BONDING);
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED4);
  }

  /* sensors have been initialized in HW_Init() */

  BSP_LED_Off(LED5);
  rc = Add_Acc_Service();
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED5);
  }

  BSP_LED_Off(LED6);
  rc = Add_Environmental_Sensor_Service();
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED6);
  }

  /* Init_User_Timer();
  Start_User_Timer(); */

  BSP_LED_On(LED3);
  /* -2 dBm output power */
  rc = aci_hal_set_tx_power_level(1, 4);
  while (rc) {
    HAL_Delay(100);
    BSP_LED_Toggle(LED3);
  }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  int16_t accData[3];
#ifdef WITH_USART
  char msg[3] = {'0','\r','\n'};
#endif

  HAL_Init();
  HW_Init();
  BlueNRG_Init();

  while (1)
  {
    HCI_Process();
    if (set_connectable) {
      set_bluenrg_connectable();
      set_connectable = 0;
    }

    /* Blink the orange LED */
    if (HAL_GetTick() % 500 == 0)
      BSP_LED_Toggle(LED3);

    if (HAL_GetTick() % 100 == 0) {
      BSP_ACCELERO_GetXYZ(accData);
      Acc_Update(accData);
    }

#ifdef WITH_USART
    msg[0] = (msg[0] == '9')? '0' : msg[0]+1;
    if (HAL_USART_Transmit(&UsartHandle, (uint8_t *)msg, 3, 5000) != HAL_OK)
      ColorfulRingOfDeath();
#endif
  }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

/**
 * @brief EXTI line detection callbacks
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == BLUENRG_IRQ_PIN)
    HCI_Isr();
}

/**
 * @brief  This function handles External line 0 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(BLUENRG_IRQ_PIN);
}

/**
 * @brief  This function puts the device in connectable mode.
 * @note   If you want to specify a UUID list in the advertising data, those data
 *         can be specified as a parameter in aci_gap_set_discoverable().
 *         For manufacture data, aci_gap_update_adv_data must be called.
 *         Ex.:
 *           tBleStatus ret;
 *           const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','l','u','e','N','R','G'};
 *           const uint8_t serviceUUIDList[] = {AD_TYPE_16_BIT_SERV_UUID, 0x34, 0x12};
 *           const uint8_t manuf_data[] = {4, AD_TYPE_MANUFACTURER_SPECIFIC_DATA, 0x05, 0x02, 0x01};
 *           ret = aci_gap_set_discoverable(ADV_IND, 0, 0, RANDOM_ADDR, NO_WHITE_LIST_USE,
 *                                          8, local_name, 3, serviceUUIDList, 0, 0);
 *           ret = aci_gap_update_adv_data(5, manuf_data);
 */
void set_bluenrg_connectable(void)
{
  tBleStatus ret;
  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'B','l','u','e','N','R','G'};

  /* disable scan response */
  hci_le_set_scan_resp_data(0, NULL);
  /*PRINTF("General Discoverable Mode.\n");*/

  ret = aci_gap_set_discoverable(ADV_IND, 0, 0, RANDOM_ADDR, NO_WHITE_LIST_USE,
                                 8, local_name, 0, NULL, 0, 0);
  if (ret != 0)
    BSP_LED_On(LED4);
}

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
