/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"

#include "ili9341.h"
#include "ili9341_gfx.h"

#include "guitar.h"
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
// Game
int life;
int score;
unsigned char received_data;
// Screen
extern ili9341_t *_screen;

// Systick
volatile int frameUp = 1;
int timerFPS = 0;
float fps = 75; 	// 75ms (1 frame par 75ms)

extern volatile int nextNoteUp;
float tempoNextNote = 900;
extern volatile unsigned char voice1[];
extern volatile unsigned char voice2[];
extern volatile int current_time;
extern volatile struct Guitar guitar2;
int full_timer;
int PWM_on = 1;

// Ultrason
extern volatile float position_us; //TODO

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (PWM_on == 1) {
		HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
		PWM_on =0;
	}
	else if (PWM_on == 0) {
		HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
		PWM_on =1;
	}
}




/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_SYSTICK_Callback(void) {
	full_timer++;
	timerFPS++;
	
	if (timerFPS >= fps){
		frameUp = 1;
		timerFPS = 0;
	}
}











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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM12_Init();
  MX_UART5_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	
	// Screen
	_screen = ili9341_new(
		&hspi1,
		Void_Display_Reset_GPIO_Port, Void_Display_Reset_Pin,
		TFT_CS_GPIO_Port, TFT_CS_Pin,
		TFT_DC_GPIO_Port, TFT_DC_Pin,
		isoLandscape,
		NULL,  NULL,
		NULL, NULL,
		itsNotSupported,
		itnNormalized);

unsigned char a = 0;
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		initGame();
		el_condor();
		
		while(life) { 											//tant que life est plus grand que 0
			while(!frameUp) {									//On fait rien pendant 75 ms
				display_guitar(position_us) ; 	//A part afficher ou est placer le curseur du joueur
				// play the right frequency
			}
			frameUp = 0;
			next_state(); 										// si la note n'est pas la bonne on perd une vie, et si on est a la fin du tableau des voix on recommence
			

			
			
			//sending and receiving data
 			a = voice2[current_time+30]; 				// la note que est jouer maintenant est donner au joueur 2 pour la jouer comme prochaine note
			send_next(&a, sizeof(a)); 					// on envoi la note au joueur 2
	



			//received_data = 0;
			receive_next((unsigned char *)guitar2.note); 		// je joueur recois la note a jouer
			

			//add_notes_future((unsigned char *)voice2, received_data); // on ajouter la note pour current_time+30 (prochaine note a jouer)

			//affiche numerique prochaine note a jouer
			//displaying_test(received_data);   //affiche la note que est jouer pas le premier joueur dans le present
			
			
			
			display_score(); // affichage des notes a jouer
			display_guitar(position_us);
			display_guitar2((float) guitar2.posx);
		}	
		gameOver();
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

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
