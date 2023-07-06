/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal.h"
#include "stm32h743xx.h"
#include "stdio.h"
#include <math.h>
#include <MRAM_memory_consistency.h>
#include <stdlib.h>
#include <time.h>
#include "mram_commons.h"
#include "MRAM_driver.h"
#include "FLASH_SECTOR_H7.h"
#include "test_flash.h"
#include "test_embbeded_SRAM.h"
#include "test_MRAM.h"
#include "clht_lb.h"
#include "test_hashmap_numbers.h"
#include "benchmark_lpht.h"
#include "MRAM_driver.h"
#include "benchmark_clht.h"
#include "settings.h"
#include <malloc.h>
#include KEYS_FILE

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
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x30000000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30000200
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30000000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30000200))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */

#endif

ETH_TxPacketConfig TxConfig;

CRC_HandleTypeDef hcrc;

ETH_HandleTypeDef heth;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_FMC_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

typedef enum writeBitSize{
	BITS_8 = 8,
	BITS_16 = 16,
	BITS_32 = 32,
	BITS_64 = 64

} WriteBitSize;



int __io_putchar(int ch)
{
	ITM_SendChar(ch);
	return(ch);
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}


void MPU_RegionConfig()
{
	MPU_Region_InitTypeDef MPU_InitStruct = {0};

	HAL_MPU_Disable();

	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = MRAM_BANK_ADDR;
	MPU_InitStruct.Size = MPU_REGION_SIZE_4MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void test_flash(){
	uint32_t total_write_size[12] = {8192U, 131072U};//, 64, 128, 256, 416, 256U, 512U, 1024U, 2048U, 4096U,}; //In bits
		int nrAvgIterations = 50;
		long double avg_throughput = 0;
		WriteBitSize writeChunkSize = BITS_32; //In bits, 8, 16, 32, or 64

		for(int i = 0; i < 2; i++){
			if(total_write_size[i] >= writeChunkSize){
				//avg_throughput = test_write_throughput_MRAM(writeChunkSize, total_write_size[i], nrAvgIterations);
				avg_throughput = test_flash_write_throughput(writeChunkSize, total_write_size[i], nrAvgIterations);
				printf("Avg Flash throughput for writing %ld bits in %d-bit chunk sizes, averaged over %d iterations: %.3Lf KB/s\n",total_write_size[i],writeChunkSize, nrAvgIterations, avg_throughput);
				puts("");
			}
	}
}


void test_MRAM_throughput( uint8_t read, uint8_t write){
	uint32_t operation_size[6] = {2U,4U,8U,512U,4096U,32768U};//, 64, 128, 256, 416, 256U, 512U, 1024U, 2048U, 4096U,}; //In bits
	int nrAvgIterations;
	long double avg_throughput = 0;
	WriteBitSize chunkSize = BITS_16; //In bits, 8, 16, 32, or 64. Always tries to use biggest possible
	uint32_t totalBytes = 536870912; ///500MiB per test

	for(int i = 0; i < 6; i++){

		if(operation_size[i] < 2U){
			chunkSize = BITS_8;
		} else if(operation_size[i] < 4U){
			chunkSize = BITS_16;
		} else if(operation_size[i] < 8U){
			chunkSize = BITS_32;
		}

		nrAvgIterations = totalBytes / operation_size[i]; //Division surplus is ignored, may write slightly less than 500MiB. Average throughput calculation is not affected.
		if(write){
			avg_throughput = test_write_throughput_MRAM(chunkSize, operation_size[i], nrAvgIterations, 0);
			//avg_throughput = test_flash_write_throughput(writeChunkSize, total_write_size[i], nrAvgIterations);
			printf("Access type: Sequential; Write operation size: %ld; Bytes written: %ld; Avg throughput: %.3Lf KB/s\n", operation_size[i], nrAvgIterations * operation_size[i], avg_throughput);
			puts("");

			avg_throughput = test_write_throughput_MRAM(chunkSize, operation_size[i], nrAvgIterations, 1);
			//avg_throughput = test_flash_write_throughput(writeChunkSize, total_write_size[i], nrAvgIterations);
			printf("Access type: Random; Write operation size: %ld; Bytes written: %ld; Avg throughput: %.3Lf KB/s\n", operation_size[i], nrAvgIterations * operation_size[i], avg_throughput);
			puts("");
		}

		if (read){
			avg_throughput = test_read_throughput_MRAM(chunkSize, operation_size[i], nrAvgIterations, 0);
			//avg_throughput = test_flash_write_throughput(writeChunkSize, total_write_size[i], nrAvgIterations);
			printf("Access type: Sequential; Read operation size: %ld; Bytes read: %ld; Avg throughput: %.3Lf KB/s\n", operation_size[i], nrAvgIterations * operation_size[i], avg_throughput);
			puts("");

			avg_throughput = test_read_throughput_MRAM(chunkSize, operation_size[i], nrAvgIterations, 1);
			//avg_throughput = test_flash_write_throughput(writeChunkSize, total_write_size[i], nrAvgIterations);
			printf("Access type: Random; Read operation size: %ld; Bytes read: %ld; Avg throughput: %.3Lf KB/s\n", operation_size[i], nrAvgIterations * operation_size[i], avg_throughput);
			puts("");
		}




	}
}

void test_MRAM_reads(){
	uint32_t total_read_size[12] = {512U};//, 64, 128, 256, 416, 256U, 512U, 1024U, 2048U, 4096U,}; //In bits
	int nrAvgIterations = 10000000;
	long double avg_throughput = 0;
	WriteBitSize readChunkSize = BITS_64; //In bits, 8, 16, 32, or 64
	uint8_t type = 0; //0 = sequential access; 1 = random access;

	for(int i = 0; i < 1; i++){
		if(total_read_size[i] >= readChunkSize){
			avg_throughput = test_read_throughput_MRAM(readChunkSize, total_read_size[i], nrAvgIterations, type);
			printf("Avg Flash throughput for reading %ld bits in %d-bit chunk sizes, averaged over %d iterations: %.3Lf KB/s\n",total_read_size[i],readChunkSize, nrAvgIterations, avg_throughput);
			puts("");
		}
	}
}

void test_MRAM_memcpy(){
	uint32_t numBytesToWrite = MRAM_NR_BYTES - 16;
	uint32_t block_size[] = {2,4,8,512, 4096, 32768};
	int numIterations = 5;

	for(int i = 0; i < 6; i++){
		test_write_throughput_MRAM_memcpy_string(block_size[i], numBytesToWrite, numIterations);
	}
}

void test_memories_throughput(){

	//test_MRAM_memcpy();
	test_MRAM_throughput(1,1);




	//printf("Avg SRAM throughput for 64 bits: %.3lf B/s\n",avg_throughput[3]);
	//printf("Avg SRAM throughput for 128 bits: %.3lf B/s\n",avg_throughput[4]);
	//printf("Avg SRAM throughput for 256 bits: %.3lf B/s\n",avg_throughput[5]);
	//printf("Avg SRAM throughput for 416 bits: %.3lf B/s\n",avg_throughput[6]);
	//printf("Avg SRAM throughput for 1024 bits: %.3lf B/s\n",avg_throughput[7]);
	//printf("Avg SRAM throughput for 2048 bits: %.3lf B/s\n",avg_throughput[8]);


}

void test_clht(){
	uint32_t startTime = 0, elapsedTime = 0;
	uint8_t test[] = {49,50,51,52};
	uint32_t uwCRCValue = HAL_CRC_Calculate(&hcrc,(uint32_t *) test, 4);
	printf("CRC32 value: %lu\n", uwCRCValue);

	uint64_t key = 10;
	uint64_t value = 20, get_value = 5;

	clht_t *hash =  clht_create(10);
	clht_put(hash, (clht_addr_t) key, (clht_val_t) value);
	//value = 1337;
	startTime = HAL_GetTick();
	for(int i = 0; i < 1000000; i++){
		clht_put(hash, (clht_addr_t) key, (clht_val_t) value);
	}
	elapsedTime = HAL_GetTick() - startTime;
	get_value = clht_get(hash->ht, (clht_addr_t) key);
	printf("Value obtained from hashtable: %llu\n", get_value);
	printf("Elapsed time: %lu ms\n", elapsedTime);
}

void _malloc_occupy_32byte(uint8_t * memoryMap, uint32_t startAddress, uint32_t refAddress){
	uint32_t index = (startAddress - refAddress) / 32;
	uint8_t aligned = !((startAddress - refAddress) % 32);

	if((memoryMap[index] == '-') && aligned){
		memoryMap[index] = '*'; // '*' character
	} else if ((memoryMap[index] == '-') &&  !aligned){
		memoryMap[index] = '/'; // '/' bucket targets this address, but unaligned, should not happen
	} else {
		memoryMap[index] = 'X';// 'X' -> Targeted by more than one bucket, should not happen
	}

}

void print_malloc_map(){
	uint8_t memoryMap[100];

	void * pointerArray[100];

	for(int i = 0; i < 100; i++){
		pointerArray[i] = memalign(32, 32);
	}

	for(int i = 0; i < 100; i++){
		memoryMap[i] = '-';
	}

	for(int i = 0; i < 100; i++){
		printf("%lx\n", (uint32_t)pointerArray[i]);
		_malloc_occupy_32byte(memoryMap, (uint32_t) pointerArray[i], (uint32_t)pointerArray[0]);
	}

	for(int i = 0; i < 100; i++){
		printf("%c", memoryMap[i]);
		if((i != 0) && (i % 100 == 0)){
			printf("\n");
			HAL_Delay(500);
		}

	}
	printf("\n");

}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//From errata sheet, to avoid read inconsistencies from AXI SRAM

  //Note: May not be needed.
  MPU_RegionConfig();
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  //addr = SRAM_BANK_ADDR + 2;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_FMC_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  //======
  // MPU binary: XN = 1
  //	  	  	 AP = 011
  //			 TEX = 000 C = 0 B = 0 Shareable = 1
  //			 SRD = 0
  //  	  	  	 SIZE =

  //Fix from errata sheet
  uint32_t address = GPV_BASE + 0x8108;
  address = address | 0x01;

  HAL_Delay(1000);
  //int randSeeds[5] = {129879, 98019823, 1239889, 12980, 928308};

  srand(237);
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	printf("Program starting... \n");
	printf("Keys pointer: %p\n", keys);
	printf("First string pointer: %p\n", keys[0]);
    printf("First string: %s\n", keys[0]);
	//printf("Wiping MRAM\n");

	//mram_wipe();
	HAL_Delay(1000);
	//NVIC_SystemReset();
	//test_memory_consistency(16);
	//test_memory_consistency_random(32);
	//test_memories_throughput();


	//key_value_store_test_string();
	 //Iterate over seed list

	//print_malloc_map();
	//benchmark_clht_write_throughput();
	//test_clht();
	benchmark_lpht_throughput();


	HAL_Delay(30*1000);
	NVIC_SystemReset();
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
  hcrc.Init.GeneratingPolynomial = 517762881;
  hcrc.Init.CRCLength = CRC_POLYLENGTH_32B;
  hcrc.Init.InitValue = 0xffffffff;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NORSRAM_TimingTypeDef Timing = {0};
  FMC_NORSRAM_TimingTypeDef ExtTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_ENABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 1;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 10;
  Timing.BusTurnAroundDuration = 4;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */
  ExtTiming.AddressSetupTime = 1;
  ExtTiming.AddressHoldTime = 15;
  ExtTiming.DataSetupTime = 10;
  ExtTiming.BusTurnAroundDuration = 4;
  ExtTiming.CLKDivision = 16;
  ExtTiming.DataLatency = 17;
  ExtTiming.AccessMode = FMC_ACCESS_MODE_A;

  if (HAL_SRAM_Init(&hsram1, &Timing, &ExtTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  HAL_SetFMCMemorySwappingConfig(FMC_SWAPBMAP_SDRAM_SRAM);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
