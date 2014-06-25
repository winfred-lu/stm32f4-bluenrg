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

#ifdef WITH_VCP
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_interface.h"
#include "usbd_desc.h"

USBD_HandleTypeDef hUSBDDevice;
#endif

#ifdef WITH_USART
USART_HandleTypeDef UsartHandle;

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
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

#ifdef WITH_USART
  UsartHandle.Instance        = USART2;
  UsartHandle.Init.BaudRate   = 9600;
  UsartHandle.Init.WordLength = USART_WORDLENGTH_8B;
  UsartHandle.Init.StopBits   = USART_STOPBITS_1;
  UsartHandle.Init.Parity     = USART_PARITY_NONE;
  UsartHandle.Init.Mode       = USART_MODE_TX_RX;

  /* Init Device Library */
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

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  int16_t accData[3];
#ifdef WITH_USART
  uint8_t aTxBuf[4] = {0};
  aTxBuf[0] = '0';
#endif

  HAL_Init();
  HW_Init();

  while (1)
  {
    /* Blink the orange LED */
    HAL_Delay(500);
    HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);

    BSP_ACCELERO_GetXYZ(accData);

#ifdef WITH_USART
    if (++aTxBuf[0] > '9')
      aTxBuf[0] = '0';
    if (HAL_USART_Transmit_IT(&UsartHandle, (uint8_t*)aTxBuf, 4) != HAL_OK)
      ColorfulRingOfDeath();
#endif
  }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
