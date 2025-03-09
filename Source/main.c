/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include "lcd.h"

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
rgb_lcd data;// pour le choix de la couleur de LCD
uint32_t adc_value = 0;//valeur recuperee de l'ADC
char buffer_humidite[50],buffer_tension[50];//pour transformer en chaine de caractere pour les afficher sur le LCD
float humidity_percentage ;//valeur d'humidite
float tension;//la tension correspondante
uint8_t flag_finADC = 0;
uint8_t Pwm_Buffer[3] = {48,49,48}; // Buffer pour stocker les 3 octets reçus, 48 ASCII = 0 entier


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void Pin_PC_Init(void);
void Reception_PWM_Value (uint8_t * PWM_buffer);
uint8_t PWM_Value( uint8_t * PWM_buffer);
void pompe_vitesse(uint8_t vitesse);
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
	uint8_t pwmVal = 0;
	uint8_t secondes = 0;//compte les secondes qui sont ecoulees
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
  MX_USART2_UART_Init();
  MX_TIM3_Init();/*pour generer les PWM*/
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();/*pour generer l'interruption chaque 1s*/
  MX_USART1_UART_Init();/*pour communiquer avec le telephone*/
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);//activer le PWM
  lcd_init(&hi2c1,&data);//initilisaliser le LCD
  HAL_TIM_Base_Start_IT(&htim2);  // Démarrer le timer avec interruption

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	/*l'ADC est lance par interruption du timer TIM2 chaque 1s*/

	 Reception_PWM_Value(Pwm_Buffer);//recevoir la valeur PWM du telephone
	 pwmVal = PWM_Value(Pwm_Buffer);//mise a jour de la valeur
	 if(flag_finADC){
		  flag_finADC = 0; //effacer le flag pour la prochiane conversion
		  if(humidity_percentage <= 40){
			  pompe_vitesse(pwmVal);//pompe ON
		  }
		  else {
			  pompe_vitesse(0);//pompe OFF
		  }
		  clearlcd();  // Nettoyage à chaque itération
		  lcd_position(&hi2c1, 0, 0);  // Ligne 1
		  lcd_print(&hi2c1,buffer_humidite);
		  lcd_position(&hi2c1,0,1);  // Ligne 2
		  lcd_print(&hi2c1,buffer_tension);
		  secondes++;
		  if(secondes == 10){
			  //10s sont ecoulees
			   HAL_UART_Transmit(&huart1, (uint8_t*)buffer_humidite, sizeof(buffer_humidite), 1000);//envoyer l'humidite vers l'esp32
			   HAL_UART_Transmit(&huart1, (uint8_t*)buffer_tension, sizeof(buffer_tension), 1000);//envoyer la tension vers l'esp32
			   secondes = 0;//remetre le compteur a zero
		  }
	  }
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/*ISR executee chaque overflow du timer tim2 (1s)*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//Aciver l'ADC chaque 1s (par interruption de timer TIM2)/
    if (htim->Instance == TIM2)//tester si l'interruption vient du TIM2
    {
    	HAL_ADC_Start(&hadc1);//activer l'ADC
    	// Attendre la fin de la conversion
    	if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK){
        adc_value = HAL_ADC_GetValue(&hadc1);// Lire la valeur de l'ADC
    	tension = (float) adc_value*5/4095 ;// calculer la tension correspondante a la valeur ADC
        humidity_percentage = (float) adc_value / 4095 * 100;//Conversion de la valeur analogique en pourcentage d'humidite
        snprintf(buffer_humidite, sizeof(buffer_humidite), "Humidity:%.2f%%  \n\r", humidity_percentage);//transformer la valeur en chaine de caracteres
    	snprintf(buffer_tension, sizeof(buffer_tension), "Tension:%.2f V  \n\r", tension);//transformer la valeur en chaine de caracteres
    		}
    	flag_finADC = 1;//indiquer la fin de conversion

    }

}

/*cette fonction permet de recevoir la valeur PWM du usart1 envoyee par le telephone*/
 void Reception_PWM_Value (uint8_t * PWM_buffer)
{
	HAL_StatusTypeDef status;
	uint8_t i = 0;
	uint8_t buffer_reception[10] = {0} ;// buffer assez grand pour contenir les messages
	status = HAL_UART_Receive(&huart1, buffer_reception,3 , 50);//verification de la reception(bit RXNE)
	if(status == HAL_OK){
		// si on recoit un caractere avant le Timout (50ms) on remplie le tableau
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);// toggle la LED sur PC0 pour indiquer la reception d'une nouvelle valeur PWM par telephone
		for(i = 0 ; i<= 3 ; i++){
			PWM_buffer[i] = buffer_reception[i];//stocker les valeur recu dans le tableau PWM_buffer
		}
		HAL_Delay(50);//s'assurer que le tableau est rempli

	}
	/*si le Timout est deppase sans recevoir un caractere on quitte la foction sans modifier la valeur de PWM*/
}

/*cette fonction transforme la valeur recu en ASCII en valeur entiere et verifie cette valeur recu*/
uint8_t PWM_Value( uint8_t * PWM_buffer)
{
	uint8_t PWM_Val = 0;
	uint8_t PWM_Val_Test = 0;
	//par deffault PWM_buffer = {48,49,48} => PWM_Val_Test = 10%
	PWM_Val_Test = ((PWM_buffer[0] - '0')*100) + ((PWM_buffer[1] - '0')*10) + ((PWM_buffer[2] - '0')*1); // transformer la valeur recu d'ASCII en entier
	if(PWM_Val_Test >= 100){
		 PWM_Val = 100; // 0<PWM<100
	  }
	else {
		PWM_Val = PWM_Val_Test;
	  }
	return PWM_Val;
}

/*changement de la vitesse de la pompe*/
void pompe_vitesse(uint8_t vitesse)
{
	/*la variable vitesse determine la valeur de rapport cyclique 100 = 3.3V, 10 = 0.33V, 0 = 0V ...*/
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, vitesse);
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
