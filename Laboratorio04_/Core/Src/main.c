/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include 	<stdbool.h>
#define 	META  4

// Estados del juego
typedef enum {
    START,        // En espera de btn inicio
    COUNTDOWN,   // Cuenta regresiva
    RACING,      // Juego iniciado
    FINISHED     // hay ganador, se bloquea
} GameState;

// Contadores jugadores
volatile uint8_t Conteo_J1 = 0;
volatile uint8_t Conteo_J2 = 0;

volatile	uint8_t	Bandera_BTN_inicio = 0;
volatile	uint8_t	Bandera_BTN_J1 = 0;
volatile	uint8_t	Bandera_BTN_J2 = 0;

volatile GameState ESTADO = START;

// 0 = no ganador | 1 = Jugador 1 | 2 = Jugador 2
volatile uint8_t ganador = 0;

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

void ActualizarLEDs(void);				// Mostrar en leds contador de cada jugador
void displayDigit(uint8_t digito); 		// muestra valor display
void processButtons(void);      		// banderas botones
void iniciarCuentaRegresiva(void);
void declararGanador(uint8_t jugador); 	// detener juego y mostrar ganador

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */

  displayDigit(0);
  ActualizarLEDs();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  processButtons();
	  switch (ESTADO){

	  	  case START:
	  		  break;

	  	  case COUNTDOWN:
	  		  break;

	  	  case RACING:
	  		  // Verificar si hay ganador
	  		  if (Conteo_J1 >= META){
	  			  declararGanador(1);
	  		  } else if (Conteo_J2 >= META){
	  			  declararGanador(2);
	  		  }
	  		  break;

	  	  case FINISHED:
	  		  // Juego bloqueado
	  		  break;
	  }

	  HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin|LED5_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED1_Pin|LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED6_Pin|LED7_Pin|LED8_Pin|LED4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SEG_A_Pin SEG_B_Pin SEG_C_Pin SEG_D_Pin
                           SEG_E_Pin SEG_F_Pin SEG_G_Pin LED5_Pin */
  GPIO_InitStruct.Pin = SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin|LED5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
  GPIO_InitStruct.Pin = LED1_Pin|LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED6_Pin LED7_Pin LED8_Pin LED4_Pin */
  GPIO_InitStruct.Pin = LED6_Pin|LED7_Pin|LED8_Pin|LED4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_J1_Pin BTN_J2_Pin BTN_INI_Pin */
  GPIO_InitStruct.Pin = BTN_J1_Pin|BTN_J2_Pin|BTN_INI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// Activar banderas
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == BTN_INI_Pin) {
        Bandera_BTN_inicio = 1;

        HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
    }
    else if (GPIO_Pin == BTN_J1_Pin) {
        Bandera_BTN_J1 = 1;
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);


        //if (ESTADO == RACING && Conteo_J1 < META){
        //	Conteo_J1++;
        //	ActualizarLEDs();
        //}
    }
    else if (GPIO_Pin == BTN_J2_Pin) {
        Bandera_BTN_J2 = 1;
        HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);


		//if (ESTADO == RACING && Conteo_J2 < META){
		//	Conteo_J2++;
		//	ActualizarLEDs();
		//}
    }
}

void processButtons(void){
    // Botón INICIO
    if (Bandera_BTN_inicio) {
        Bandera_BTN_inicio = 0;



        HAL_Delay(50);
        if (ESTADO == START) {
            //iniciarCuentaRegresiva();

        	ESTADO = COUNTDOWN;
        	displayDigit(5);
        	HAL_Delay(1000);
        	displayDigit(4);
			HAL_Delay(1000);
			displayDigit(3);
			HAL_Delay(1000);
			displayDigit(2);
			HAL_Delay(1000);
			displayDigit(1);
			HAL_Delay(1000);
			displayDigit(0);
			// Iniciar carrera
			ESTADO = RACING;
			ActualizarLEDs();
        }
        HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
    }

    // Botón JUGADOR 1
    if (Bandera_BTN_J1) {
        Bandera_BTN_J1 = 0;
        HAL_Delay(50);
        if (ESTADO == RACING && Conteo_J1 < META) {
            Conteo_J1++;
            ActualizarLEDs();
        }
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    }

    // Botón JUGADOR 2
    if (Bandera_BTN_J2) {
        Bandera_BTN_J2 = 0;
        HAL_Delay(50);
        if (ESTADO == RACING && Conteo_J2 < META) {
            Conteo_J2++;
            ActualizarLEDs();
        }
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    }
}

void iniciarCuentaRegresiva(void){
    ESTADO = COUNTDOWN;
    for (int i = 5; i >= 0; i--) {
        displayDigit(i);     			// mostrar numero actual
        HAL_Delay(1000);     			// esperar 1 seg
    }
    // Iniciar carrera
    ESTADO = RACING;
    ActualizarLEDs();
}

void displayDigit(uint8_t digito){

	if (digito > 9) digito = 0;

	// Vector de configuración de segmentos - display
    const uint8_t segMap[10] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111  // 9
    };

    uint8_t mascara = segMap[digito];

    // Primero apagar todos los segmentos
    HAL_GPIO_WritePin(GPIOC, SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | SEG_E_Pin | SEG_F_Pin | SEG_G_Pin, GPIO_PIN_SET);

    // Segundo mostrar valor recibido
    if (mascara & 0x01) HAL_GPIO_WritePin(GPIOC, SEG_A_Pin, GPIO_PIN_RESET);
    if (mascara & 0x02) HAL_GPIO_WritePin(GPIOC, SEG_B_Pin, GPIO_PIN_RESET);
    if (mascara & 0x04) HAL_GPIO_WritePin(GPIOC, SEG_C_Pin, GPIO_PIN_RESET);
    if (mascara & 0x08) HAL_GPIO_WritePin(GPIOC, SEG_D_Pin, GPIO_PIN_RESET);
    if (mascara & 0x10) HAL_GPIO_WritePin(GPIOC, SEG_E_Pin, GPIO_PIN_RESET);
    if (mascara & 0x20) HAL_GPIO_WritePin(GPIOC, SEG_F_Pin, GPIO_PIN_RESET);
    if (mascara & 0x40) HAL_GPIO_WritePin(GPIOC, SEG_G_Pin, GPIO_PIN_RESET);

}

void ActualizarLEDs(void)
{

	// JUGADOR 1 ====================================================================
	// Apagar leds
	HAL_GPIO_WritePin(GPIOA, LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, LED4_Pin, GPIO_PIN_SET);
    // LED1 (PA5) enciende
    HAL_GPIO_WritePin(GPIOA, LED1_Pin, (Conteo_J1 == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // LED2 (PA6) enciende
    HAL_GPIO_WritePin(GPIOA, LED2_Pin, (Conteo_J1 == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // LED3 (PA7) enciende
    HAL_GPIO_WritePin(GPIOA, LED3_Pin, (Conteo_J1 == 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    // LED4 (PB6) enciende
    HAL_GPIO_WritePin(GPIOB, LED4_Pin, (Conteo_J1 == 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);


    // JUGADOR 2 ====================================================================
    // Apagar leds
    HAL_GPIO_WritePin(GPIOC, LED5_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, LED6_Pin | LED7_Pin | LED8_Pin, GPIO_PIN_RESET);
    // LED5 (PC7) enciende
	HAL_GPIO_WritePin(GPIOC, LED5_Pin, (Conteo_J2 == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	// LED6 (PB10) enciende
	HAL_GPIO_WritePin(GPIOB, LED6_Pin, (Conteo_J2 == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	// LED7 (PB4) enciende
	HAL_GPIO_WritePin(GPIOB, LED7_Pin, (Conteo_J2 == 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	// LED8 (PB5) enciende
	HAL_GPIO_WritePin(GPIOB, LED8_Pin, (Conteo_J2 == 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void declararGanador(uint8_t jugador){
	ganador = jugador;
	ESTADO = FINISHED;

    if (jugador == 1) {
        // Encender leds J1
        HAL_GPIO_WritePin(GPIOA, LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, LED4_Pin, GPIO_PIN_SET);
        // Apagar leds J2
        HAL_GPIO_WritePin(GPIOC, LED5_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, LED6_Pin | LED7_Pin | LED8_Pin, GPIO_PIN_RESET);

        displayDigit(1);
    }
    else if (jugador == 2) {
        // Encender leds J2
        HAL_GPIO_WritePin(GPIOC, LED5_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, LED6_Pin | LED7_Pin | LED8_Pin, GPIO_PIN_SET);
        // Apagar leds J2
        HAL_GPIO_WritePin(GPIOA, LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, LED4_Pin, GPIO_PIN_RESET);

        displayDigit(2);
    }

    // desactivar interrupciones btns (J1|J2)
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
