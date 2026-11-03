
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Light and IR sensor display with LED indication
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
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "light_sensor.h"
#include "OLED.h"
#include <stdio.h>
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

// LED limit levels
const uint16_t LIMIT_OFF = 1000;
const uint16_t LIMIT_1   = 2000;
const uint16_t LIMIT_2   = 3000;
const uint16_t LIMIT_3   = 4000;

// Light sensor I2C address
extern const uint8_t LIGHT_SENSOR_ADDRESS;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Recover I2C bus if SDA is stuck LOW.
 *
 * PB6 = SCL
 * PB7 = SDA
 *
 * Must run before MX_I2C1_Init().
 */
static void I2C_Bus_Recovery(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_6 | GPIO_PIN_7,
        GPIO_PIN_SET
    );

    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOB, &gpio);

    // Generate up to 9 clock pulses
    for (int i = 0;
         i < 9 &&
         HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_RESET;
         i++)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // Generate STOP condition
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_Delay(1);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_Delay(1);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(1);
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

    /* MCU Configuration------------------------------------------------------*/

    HAL_Init();

    /* USER CODE BEGIN Init */
    /* USER CODE END Init */

    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    // Recover I2C bus before peripheral initialization
    I2C_Bus_Recovery();

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();

    /* USER CODE BEGIN 2 */

    // Initialize OLED
    OLED_Init();
    OLED_Fill(BLACK);

    OLED_SetCursor(0, 0);
    OLED_WriteString("Light (CH0):", Font_7x10, WHITE);

    OLED_SetCursor(0, 38);
    OLED_WriteString("IR (CH1):", Font_7x10, WHITE);

    OLED_UpdateScreen();

    // Power ON the light sensor
    Light_Sensor_Power_On(SIMPLE_MODE);

    // Configure default timing:
    // 1x gain, approximately 402 ms integration time
    uint8_t timing_default = 0x02;

    if (HAL_I2C_Mem_Write(
            &hi2c1,
            LIGHT_SENSOR_ADDRESS,
            TIMING,
            I2C_MEMADD_SIZE_8BIT,
            &timing_default,
            1,
            100
        ) != HAL_OK)
    {
        Error_Handler();
    }

    // Allow first conversion to complete
    HAL_Delay(450);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */

        // ------------------------------------------------
        // 1. READ SENSOR VALUES
        // ------------------------------------------------

        // CH0: Broadband light (visible + IR)
        uint16_t light_sensor_value =
            Light_Sensor_Read_Value(SIMPLE_MODE);

        // CH1: Infrared light
        uint16_t ir_sensor_value =
            IR_Sensor_Read_Value(SIMPLE_MODE);


        // ------------------------------------------------
        // 2. LED CONTROL BASED ON LIGHT SENSOR (CH0)
        // ------------------------------------------------

        if (light_sensor_value < LIMIT_OFF)
        {
            HAL_GPIO_WritePin(
                HMI_LED_1_GPIO_Port,
                HMI_LED_1_Pin,
                GPIO_PIN_RESET
            );

            HAL_GPIO_WritePin(
                HMI_LED_2_GPIO_Port,
                HMI_LED_2_Pin,
                GPIO_PIN_RESET
            );

            HAL_GPIO_WritePin(
                HMI_LED_3_GPIO_Port,
                HMI_LED_3_Pin,
                GPIO_PIN_RESET
            );

            HAL_GPIO_WritePin(
                HMI_LED_4_GPIO_Port,
                HMI_LED_4_Pin,
                GPIO_PIN_RESET
            );
        }
        else if (light_sensor_value < LIMIT_1)
        {
            HAL_GPIO_WritePin(
                HMI_LED_1_GPIO_Port,
                HMI_LED_1_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_2_GPIO_Port,
                HMI_LED_2_Pin,
                GPIO_PIN_RESET
            );

            HAL_GPIO_WritePin(
                HMI_LED_3_GPIO_Port,
                HMI_LED_3_Pin,
                GPIO_PIN_RESET
            );

            HAL_GPIO_WritePin(
                HMI_LED_4_GPIO_Port,
                HMI_LED_4_Pin,
                GPIO_PIN_RESET
            );
        }
        else if (light_sensor_value < LIMIT_2)
        {
            HAL_GPIO_WritePin(
                HMI_LED_1_GPIO_Port,
                HMI_LED_1_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_2_GPIO_Port,
                HMI_LED_2_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_3_GPIO_Port,
                HMI_LED_3_Pin,
                GPIO_PIN_RESET
            );

            HAL_GPIO_WritePin(
                HMI_LED_4_GPIO_Port,
                HMI_LED_4_Pin,
                GPIO_PIN_RESET
            );
        }
        else if (light_sensor_value < LIMIT_3)
        {
            HAL_GPIO_WritePin(
                HMI_LED_1_GPIO_Port,
                HMI_LED_1_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_2_GPIO_Port,
                HMI_LED_2_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_3_GPIO_Port,
                HMI_LED_3_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_4_GPIO_Port,
                HMI_LED_4_Pin,
                GPIO_PIN_RESET
            );
        }
        else
        {
            HAL_GPIO_WritePin(
                HMI_LED_1_GPIO_Port,
                HMI_LED_1_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_2_GPIO_Port,
                HMI_LED_2_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_3_GPIO_Port,
                HMI_LED_3_Pin,
                GPIO_PIN_SET
            );

            HAL_GPIO_WritePin(
                HMI_LED_4_GPIO_Port,
                HMI_LED_4_Pin,
                GPIO_PIN_SET
            );
        }


        // ------------------------------------------------
        // 3. DISPLAY BOTH VALUES ON OLED
        // ------------------------------------------------

        char light_text[16];
        char ir_text[16];

        snprintf(
            light_text,
            sizeof(light_text),
            "%5u",
            (unsigned int)light_sensor_value
        );

        snprintf(
            ir_text,
            sizeof(ir_text),
            "%5u",
            (unsigned int)ir_sensor_value
        );

        // Clear previous frame
        OLED_Fill(BLACK);

        // Light sensor label
        OLED_SetCursor(0, 0);
        OLED_WriteString(
            "Light (CH0):",
            Font_7x10,
            WHITE
        );

        // Light sensor value (large font)
        OLED_SetCursor(0, 11);
        OLED_WriteString(
            light_text,
            Font_16x26,
            WHITE
        );

        // IR sensor label
        OLED_SetCursor(0, 38);
        OLED_WriteString(
            "IR (CH1):",
            Font_7x10,
            WHITE
        );

        // IR sensor value (small font)
        OLED_SetCursor(0, 49);
        OLED_WriteString(
            ir_text,
            Font_7x10,
            WHITE
        );

        // Refresh the entire display once
        OLED_UpdateScreen();

        // Wait for next sensor conversion
        HAL_Delay(450);

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

    /** Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(
        PWR_REGULATOR_VOLTAGE_SCALE1
    );

    /** Initializes the RCC Oscillators */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType =
          RCC_CLOCKTYPE_HCLK
        | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1
        | RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV4;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_5
        ) != HAL_OK)
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

    OLED_SetCursor(0, 52);

    OLED_WriteString(
        "I2C HIBA",
        Font_7x10,
        WHITE
    );

    OLED_UpdateScreen();

    __disable_irq();

    while (1)
    {
        HAL_GPIO_TogglePin(
            GPIOD,
            HMI_LED_1_Pin |
            HMI_LED_2_Pin |
            HMI_LED_3_Pin |
            HMI_LED_4_Pin
        );

        for (volatile uint32_t i = 0; i < 3000000; i++);
    }

    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT

/**
  * @brief Reports the name of the source file and source line number
  * @param file: pointer to source file name
  * @param line: source line number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* USER CODE END 6 */
}

#endif /* USE_FULL_ASSERT */
