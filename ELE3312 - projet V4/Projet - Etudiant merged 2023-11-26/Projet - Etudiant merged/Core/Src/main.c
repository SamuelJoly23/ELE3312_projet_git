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
#include "dac.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
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

// variables pour l'ultrason 
float value1 = 0;
float value2  = 0;
int flag_captured = 0;
volatile float distanceCm = 0;
float frequence = 0;

//variable pour getFrequency et getTextNote
extern float FREQX[];
int ID_note = 0;
volatile float frequency1 = 0;
volatile float frequency2 = 0;
char* text_note = "";
volatile int value=0;

//variable pour le son sinus
#define TABLE_LENGTH 1000
uint32_t tab_valueSin1[TABLE_LENGTH]; 
uint32_t tab_valueSin2[TABLE_LENGTH]; 
uint32_t tab_value1[TABLE_LENGTH];
uint32_t tab_value2[TABLE_LENGTH]; 
uint32_t * current_tab = tab_value1;
int key = 0;
float t1 = 0, t2 = 0;

volatile float fselect = 0;

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
extern volatile struct Guitar guitar;
int full_timer;
int PWM_on = 1;

// Ultrason
extern volatile float position_us; 
volatile uint8_t rxdata[1];

uint8_t ID_note_received = 0;
uint8_t ID_note_sent = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */



void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac) {
 
		if (current_tab == tab_value1){ 
			current_tab = tab_value2;
		} else { 
			current_tab = tab_value1;
		}

	//DAC TRANSFERT 
	HAL_DAC_Start_DMA(hdac, DAC_CHANNEL_1, (uint32_t *)current_tab, TABLE_LENGTH,DAC_ALIGN_12B_R);
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

// partie A

void createTab1(uint32_t *tab1, float f1){ 
	for (int i = 0; i < TABLE_LENGTH; i++) { 
			tab1[i] = 1010+1010*sin(2*3.1415*f1*((i*0.000025)+t1));
	}
	t1 += TABLE_LENGTH*0.000025;
}

void createTab2(uint32_t *tab2, float f1){ 
	for (int i = 0; i < TABLE_LENGTH; i++) { 
			tab2[i] = 1010+1010*sin(2*3.1415*f1*((i*0.000025)+t2));
	}
	t2 += TABLE_LENGTH*0.000025;
}

void createDuoTab(uint32_t *tab1,uint32_t *tab2,uint32_t *tab3){ 
	for (int i = 0; i < TABLE_LENGTH; i++) { 	
		tab3[i]=tab1[i]+tab2[i];
	}
}

int getIdNote(float distance) {
  if (0 <= distance && distance <= 4) {
    return NOTE_B3;
  } else if (distance > 4 && distance <= 8) {
    return NOTE_C4;
  } else if (distance > 8 && distance <= 12) {
    return NOTE_D4;
  } else if (distance > 12 && distance <= 16) {
    return NOTE_D4_SHARP;
  } else if (distance > 16 && distance <= 20) {
    return NOTE_E4;
  } else if (distance > 20 && distance <= 24) {
    return NOTE_F4_SHARP;
  } else if (distance > 24 && distance <= 28) {
    return NOTE_G4;
  } else if (distance > 28 && distance <= 32) {
    return NOTE_A4;
  } else if (distance > 32 && distance <= 36) {
    return NOTE_B4;
  } else if (distance > 36 && distance <= 40) {
    return NOTE_C5;
  } else if (distance > 40 && distance <= 44) {
    return NOTE_D5;
  } else if (distance > 44 && distance <= 48) {
    return NOTE_D5_SHARP;
  } else if (distance > 48 && distance <= 52) {
    return NOTE_E5;
  } else if (distance > 52 && distance <= 56) {
    return NOTE_F5_SHARP;
  } else if (distance > 56 && distance <= 60) {
    return NOTE_G5;
  } else {
    return SILENCE;
  }
}

float getFrequency(int index){
	return FREQX[index];
}

char* getTextNote(int index){
	switch(index){
		case NOTE_B3 :
			return "B3     ";
			break;
		case NOTE_C4 :
			return "C4     ";
			break;
		case NOTE_D4 :
			return "D4     ";
			break;
		case NOTE_D4_SHARP :
			return "D4#    ";
			break;
		case NOTE_E4 :
			return "E4     ";
			break;
		case NOTE_F4_SHARP :
			return "F4#    ";
			break;
		case NOTE_G4 :
			return "G4     ";
			break;
		case NOTE_A4 :
			return "A4     ";
			break;
		case NOTE_B4 :
			return "B4     ";
			break;
		case NOTE_C5 :
			return "C5     ";
			break;
		case NOTE_D5 :
			return "D5     ";
			break;
		case NOTE_D5_SHARP :
			return "D5#    ";
			break;
		case NOTE_E5 :
			return "E5     ";
			break;
		case NOTE_F5_SHARP :
			return "F5#    ";
			break;
		case NOTE_G5 :
			return "G5     ";
			break;
		default :
			return "SILENCE";
			break;
	}
}


// Méthode appelée lorsque front montant ou descendant est détectée
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4){  // on s'assure que le channel du timer est celui du Input capture (channel4)
	 // Lors d'un front montant, on a rien détecté 
	 if(flag_captured == 0){
		 value1 = TIM2->CCR4;
		 flag_captured = 1; // on a effectué une capture 
	 } 
	 else {
		 value2 = TIM2->CCR4;  
		 distanceCm = fabs(value2 - value1)/(58.82);
		 flag_captured = 0;
		 
	//	 fselect = getFrequency(distanceCm);
		 
		 ID_note = getIdNote(distanceCm); // recupere le id note 
		frequency1 = getFrequency(ID_note); // recupere la frequence avec le id note
	 }
 }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	
		if(GPIO_Pin == GPIOEXT2_Pin){
		//UTILISER PLUTOT READPIN
		if (HAL_GPIO_ReadPin(GPIOEXT2_GPIO_Port, GPIOEXT2_Pin) == GPIO_PIN_SET) {
			HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);			
		}
		else if (HAL_GPIO_ReadPin(GPIOEXT2_GPIO_Port, GPIOEXT2_Pin) == GPIO_PIN_RESET) {
	//		f1=0; 
	//		send(f1);
			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
		}
	}
	
	if(GPIO_Pin == C1_Pin){
			if(HAL_GPIO_ReadPin(C1_GPIO_Port, C1_Pin) == GPIO_PIN_SET){
				ID_note_sent = ID_note;
				HAL_UART_Transmit_DMA(&huart5, &ID_note_sent, sizeof(ID_note_sent)); 
			} else {
				//ID_note = SILENCE;
				frequency1 = getFrequency(SILENCE);
				ID_note_sent = SILENCE;
				HAL_UART_Transmit_DMA(&huart5, &ID_note_sent, sizeof(ID_note_sent)); 
			}
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	//	uart_rx();
    HAL_UART_Receive_IT(&huart5, rxdata, 1); // warning dans la conversion de type
		ID_note_received=rxdata[0];
		frequency2 = getFrequency(ID_note_received);
}
//f2 provient du Rx: Recevoir 
//void uart_rx(UART_HandleTypeDef *huart, uint8_t *rxdata, uint8_t size)
//{
//    HAL_UART_Receive_IT(huart, rxdata, size);//uint8_t = val_rx
//		ID_note_received=rxdata[0];
//		frequency2 = getFrequency(ID_note_received); // recupere la frequence avec le id note
//		
//}

#ifndef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */





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
  MX_TIM4_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
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
			
		ili9341_fill_screen(_screen, ILI9341_BLACK);
		ili9341_text_attr_t text_attr = {&ili9341_font_11x18,ILI9341_WHITE, ILI9341_BLACK,0,0};	
	
		
		// start receiving data
		HAL_UART_Receive_IT(&huart5, rxdata, 1); // warning dans la conversion de type
		
		unsigned char a = 0;
	// start timer part A
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_4);
	// timer partie A 2e semaine 
	HAL_TIM_Base_Start(&htim5);
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)current_tab, TABLE_LENGTH,DAC_ALIGN_12B_R);

	// DÉMARRER LE TIMER 4
	HAL_TIM_Base_Start(&htim4);
	// clavier	
	HAL_GPIO_WritePin(R1_GPIO_Port, R1_Pin, GPIO_PIN_SET); // rangée 1,4,7,*
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		initGame();
		el_condor();
		
		while(life && (full_timer < 22000)) { 										
			while(!frameUp) {									// 75 ms
//				ID_note = getIdNote(distanceCm);
				text_note = getTextNote(ID_note);
//				frequence = getFrequency(ID_note);
				char buf[80];
				sprintf(buf,"Note: %s", text_note);
				ili9341_draw_string(_screen, text_attr, buf);
				
				createTab1(tab_valueSin1, frequency1);
				createTab2(tab_valueSin2, frequency2);
				if(current_tab == tab_value1){
					createDuoTab(tab_valueSin1,tab_valueSin2, tab_value1);
				} else {
					createDuoTab(tab_valueSin1,tab_valueSin2, tab_value2);
				}
				
				//display_guitar(position_us) ; 	
			}
			frameUp = 0;
			next_state(); 										
			
			
			// a = guitar.note; // note a jouer selon el condor (partie B)
			a = ID_note;
			HAL_UART_Transmit_DMA(&huart5, &a, sizeof(a)); // Transmits from IR LED
			
			display_guitar2(rxdata[0]); // Draw 2nd player circle in blue 

			display_score(); // Affichage note a jouer par le joueur
			
			display_guitar(ID_note); // Display 1st player circle in green 
			
			liveScore();
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
// to use putty 
/**
	* @brief Retargets the C library printf function to the USART.
	* @param None
	* @retval None
	*/
	PUTCHAR_PROTOTYPE
	{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART2 and Loop until the end
	of transmission */
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
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
