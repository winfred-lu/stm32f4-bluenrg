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

static void hw_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* STM32F401 discovery LEDs */
	__GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	HAL_Init();

	hw_init();

	while (1) {
		/* Blink the orange LED */
		HAL_Delay(500);
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
	}
}

void SysTick_Handler(void)
{
	HAL_IncTick();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
