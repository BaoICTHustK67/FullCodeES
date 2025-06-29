/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "TETRIS.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SAMPLE_RATE         8000U      // Desired sample rate (Hz)
#define DAC_MAX_VALUE       4095U
#define MAX_WAVE_BUF_SIZE   1024U
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DMA2D_HandleTypeDef hdma2d;
I2C_HandleTypeDef hi2c1;
LTDC_HandleTypeDef hltdc;
RNG_HandleTypeDef hrng;
SPI_HandleTypeDef hspi1;
SDRAM_HandleTypeDef hsdram1;
DAC_HandleTypeDef hdac1;
TIM_HandleTypeDef htim6;
DMA_HandleTypeDef hdma_dac1;



/* USER CODE BEGIN PV */
int isKeyHold = 0;
uint32_t randomNum;
uint32_t seed = 123;
uint16_t wave_buf[MAX_WAVE_BUF_SIZE];

typedef struct {
    float freq;       // Hz; 0 => rest
    uint32_t dur_ms;  // duration in milliseconds
} Note;

// Example melody: "Mary Had a Little Lamb" (partial)
Note melody[] = {
    {659.25511f, 200}, {493.8833f, 200}, {523.25113f, 200}, {587.32954f, 200}, {523.25113f, 200}, {493.8833f, 200}, {440.0f, 200}, {440.0f, 200},
    {523.25113f, 200}, {659.25511f, 200}, {587.32954f, 200}, {523.25113f, 200}, {493.8833f, 200}, {523.25113f, 200}, {587.32954f, 200},
    {659.25511f, 200}, {523.25113f, 200}, {440.0f, 200}, {440.0f, 200}, {440.0f, 200}, {493.8833f, 200}, {523.25113f, 200}, {587.32954f, 200},
    {698.45646f, 200}, {880.0f, 200}, {783.99087f, 200}, {698.45646f, 200}, {659.25511f, 200}, {523.25113f, 200}, {659.25511f, 200},
    {587.32954f, 200}, {523.25113f, 200}, {493.8833f, 200}, {493.8833f, 200}, {523.25113f, 200}, {587.32954f, 200}, {659.25511f, 200},
    {523.25113f, 200}, {440.0f, 200}, {440.0f, 200}
};
const size_t MELODY_LENGTH = sizeof(melody)/sizeof(melody[0]);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_LTDC_Init(void);
static void MX_FMC_Init(void);
static void MX_SPI1_Init(void);
static void MX_DMA2D_Init(void);
static void MX_I2C1_Init(void);
static void MX_RNG_Init(void);
static void MX_DAC_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// Play one note: freq in Hz, duration in ms
void generate_square(uint16_t *buf, uint32_t N) {
    const uint16_t mid = DAC_MAX_VALUE / 2;
    const int32_t peak = (DAC_MAX_VALUE / 2) - 100;
    int32_t high = mid + peak;
    int32_t low  = mid - peak;
    if (high > (int32_t)DAC_MAX_VALUE) high = DAC_MAX_VALUE;
    if (low < 0) low = 0;
    for (uint32_t i = 0; i < N; i++) {
        buf[i] = (i < (N/2)) ? (uint16_t)high : (uint16_t)low;
    }
}

void play_note(float freq, uint32_t dur_ms) {
    if (freq <= 0.0f) {
        // rest: output mid-level or silence
        HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
        HAL_Delay(dur_ms);
        return;
    }
    // Compute buffer length N ~ SAMPLE_RATE / freq
    uint32_t N = (uint32_t)( (float)SAMPLE_RATE / freq );
    if (N < 2) N = 2;
    if (N > MAX_WAVE_BUF_SIZE) {
        N = MAX_WAVE_BUF_SIZE;
    }
    // Fill wave_buf with waveform; choose square or sine
    generate_square(wave_buf, N);
    // Or: generate_sine(wave_buf, N);

    // Start DAC DMA circular
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)wave_buf, N, DAC_ALIGN_12B_R);
    // Wait duration
    HAL_Delay(dur_ms);
//    // Stop DMA
    HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
    // Short gap between notes
    HAL_Delay(50);
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

  /* Initialise all configured peripherals */
  MX_GPIO_Init();
  MX_LTDC_Init();
  MX_FMC_Init();
  MX_SPI1_Init();
  MX_DMA_Init();
  MX_DAC_Init();
  MX_DMA2D_Init();
  MX_I2C1_Init();
  MX_RNG_Init();
  /* USER CODE BEGIN 2 */
  HAL_RNG_Init(&hrng);
  MX_DAC_Init();
  MX_TIM6_Init();
  HAL_TIM_Base_Start_IT(&htim6);
  for (size_t i = 0; i < MELODY_LENGTH; i++) {
      play_note(melody[i].freq, melody[i].dur_ms);
  }
  BSP_LCD_Init();
  BSP_LCD_MspInit();
  BSP_LCD_LayerDefaultInit(0, LCD_FRAME_BUFFER);
  BSP_LCD_Clear(LCD_COLOR_BLACK);


  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
	  TETRIS_main();
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 50;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 7;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 14;
  hltdc.Init.AccumulatedVBP = 5;
  hltdc.Init.AccumulatedActiveW = 654;
  hltdc.Init.AccumulatedActiveH = 485;
  hltdc.Init.TotalWidth = 660;
  hltdc.Init.TotalHeigh = 487;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 0;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 0;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 255;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  pLayerCfg.FBStartAdress = 0;
  pLayerCfg.ImageWidth = 0;
  pLayerCfg.ImageHeight = 0;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief RNG Initialization Function
  * @param None
  * @retval None
  */
static void MX_RNG_Init(void)
{

  /* USER CODE BEGIN RNG_Init 0 */

  /* USER CODE END RNG_Init 0 */

  /* USER CODE BEGIN RNG_Init 1 */

  /* USER CODE END RNG_Init 1 */
  hrng.Instance = RNG;
  if (HAL_RNG_Init(&hrng) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RNG_Init 2 */

  /* USER CODE END RNG_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_11;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_8;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_1;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_DISABLE;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 16;
  SdramTiming.ExitSelfRefreshDelay = 16;
  SdramTiming.SelfRefreshTime = 16;
  SdramTiming.RowCycleDelay = 16;
  SdramTiming.WriteRecoveryTime = 16;
  SdramTiming.RPDelay = 16;
  SdramTiming.RCDDelay = 16;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8 |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG; // Critical for DAC output
  GPIO_InitStruct.Pull = GPIO_NOPULL; // Pull-up/down resistors are not used for analog inputs/outputs
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/**
  * @brief  Initialize DAC1 (PA4) channel 1.
  */
static void MX_DAC_Init(void)
{
	  DAC_ChannelConfTypeDef sConfig = {0};

	  hdac1.Instance = DAC;
	  HAL_DAC_Init(&hdac1);

	  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO; // DAC triggered by TIM6 TRGO
	  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE; // Enable output buffer for better driving capability
	  HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1);
}


/**
  * @brief  Initialize TIM6 to trigger an update interrupt at AUDIO_SAMPLE_RATE.
  *         E.g., for 8kHz: timer clock = 72MHz, prescaler =  (72e6/8e3)-1 = 8999, period = 0.
  */
static void MX_TIM6_Init(void)
{
	  TIM_MasterConfigTypeDef sMasterConfig = {0};

	  htim6.Instance = TIM6;
	  // Example: if SystemCoreClock = 16 MHz (HSI), choose Prescaler = 1 -> timer clock = 8 MHz
	  // Period = (8 MHz / SAMPLE_RATE) - 1 = (8000000 / 8000) - 1 = 1000 - 1 = 999
	  htim6.Init.Prescaler = 1;
	  htim6.Init.Period = 4499;
	  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

static void MX_DMA_Init(void)
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE(); // DAC1 typically uses DMA1

  /* Configure DMA request for DAC1 */
  hdma_dac1.Instance = DMA1_Stream5; // Common stream for DAC1, verify with CubeMX
  hdma_dac1.Init.Channel = DMA_CHANNEL_7; // Common channel for DAC1, verify with CubeMX
  hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;
  hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; // 12-bit DAC, so 16-bit (half-word) data
  hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_dac1.Init.Mode = DMA_CIRCULAR; // Crucial for continuous audio playback
  hdma_dac1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  hdma_dac1.Init.FIFOMode = DMA_FIFOMODE_DISABLE; // FIFO can be enabled for more complex scenarios

  HAL_DMA_Init(&hdma_dac1);

  /* Link DAC1 to DMA1 Stream5 */
  __HAL_LINKDMA(&hdac1, DMA_Handle1, hdma_dac1); // Correct macro to link DAC to DMA handle
}

int isUpKey() {
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET) {
    	return 1;
    }
    else return 0;
}

int isDownKey() {
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET) {
	    	return 1;
	}
	    else return 0;
}

int isLeftKey() {
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_RESET) {
	    return 1;
	}
	else return 0;
}

int isRightKey() {
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9) == GPIO_PIN_RESET) {
    	return 1;
	}
	    else return 0;
}

int Random(int n) // Generate a random number
{
	HAL_RNG_Init(&hrng);
	HAL_RNG_MspInit(&hrng);
	HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *) seed);
	randomNum = HAL_RNG_GetRandomNumber(&hrng);
	return randomNum % n;
}

int isAnyKeyPressed() {
	if (isUpKey() == 1) return 1;
	if (isDownKey() == 1) return 1;
	if (isLeftKey() == 1) return 1;
	if (isRightKey() == 1) return 1;
	if (isUSERKey() == 1) return 1;
	return 0;
}

void PrintXY(int x, int y) {
	int loc_X, loc_Y;
	loc_X = PIXEL_SPACE + x * (PIXEL_SPACE + PIXEL_SIZE);
	loc_Y = PIXEL_SPACE + y * (PIXEL_SPACE + PIXEL_SIZE);

	BSP_LCD_FillRect(loc_X, loc_Y, PIXEL_SIZE, PIXEL_SIZE);
}

void Colour(int n) {
	// 1 is bright red		2 is bright green		3 is bright blue
	// 4 is magenta			5 is bright yellow		6 is white
	switch (n) {
	case 0: { BSP_LCD_SetTextColor(LCD_COLOR_BLACK); break; }
	case 1: { BSP_LCD_SetTextColor(LCD_COLOR_RED); break; }
	case 2: { BSP_LCD_SetTextColor(LCD_COLOR_GREEN); break; }
	case 3: { BSP_LCD_SetTextColor(LCD_COLOR_BLUE); break; }
	case 4: { BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA); break; }
	case 5: { BSP_LCD_SetTextColor(LCD_COLOR_YELLOW); break; }
	case 6: { BSP_LCD_SetTextColor(LCD_COLOR_WHITE); break; }
	}
}

int isUSERKey() {
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) return 1;
	else return 0;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
