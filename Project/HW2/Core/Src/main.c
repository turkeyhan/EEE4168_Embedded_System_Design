/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "cmsis_os.h"
#include "freertos.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "stm32f7xx_hal.h"
#include "timers.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#pragma location=0x2004c000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x2004c0a0
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x2004c000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x2004c0a0))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection"))); /* Ethernet Tx DMA Descriptors */
#endif

ETH_TxPacketConfig TxConfig;

ETH_HandleTypeDef heth;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
#define STACK_SIZE 128
#define mainONE_SHOT_TIMER_PERIOD 1000
#define mainAUTO_RELOAD_TIMER_PERIOD 500
void SwitchTask(void *argument);
void ReviseID(uint8_t num, uint8_t digit);
void WriteNumTask(void *argument);
void ModeTask(void *argument);
void MX_FREERTOS_Init(void);
void ModeactTask(void *argument);
void ModefiveTask(void *argument);
static void prvModifyTimerCallback(TimerHandle_t xTimer);
static void prvOneShotTimerCallback(TimerHandle_t xTimer);
static void prvAutoReloadTimerCallback(TimerHandle_t xTimer);
SemaphoreHandle_t xCountingSemaphore;
//SemaphoreHandle_t xSemone;
//SemaphoreHandle_t xSemtwo;
//SemaphoreHandle_t xSemthree;
//SemaphoreHandle_t xSemfour;
//SemaphoreHandle_t xSemfive;
TimerHandle_t xOneShotTimer;
TimerHandle_t xModifyTimer;
TimerHandle_t xAutoReloadTimer;
QueueHandle_t xQueue;
QueueHandle_t xSecQueue;

QueueHandle_t xOneQueue;
QueueHandle_t xFiveQueue;

uint8_t modifychk = 0;
uint8_t dig = 1;
uint8_t moving_point = 0;
uint8_t shift_count = 0;
uint8_t secchk = 0;
uint8_t mode = 0;
uint8_t SOGANG[8][56] = {
		{ 1, 0, 0, 0, 0, 0, 1, 0,   1, 1, 0, 0, 0, 0, 1, 1,   1, 1, 0, 0, 0, 0, 0, 0,   1, 1, 1, 0, 0, 1, 1, 1,   0, 0, 1, 1, 1, 0, 0, 0,   1, 1, 0, 0, 0, 0, 0, 0,   1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1,
				0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1,
				1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 1, 1, 1, 1, 1, 1, 0,
				0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
				1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1 }, { 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1,
				1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1,
				1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1,
				0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0,
				1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 }, { 0, 1, 1, 1, 1, 1, 1, 0,
				0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1,
				1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1,
				1, 1, 1, 1, 1, 1 }, { 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1,
				0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1,
				1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0,
				1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0,
				0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };
uint8_t STUDID[8][56] = { { 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1,
		1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1,
				0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1,
				1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1,
				0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
				1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
				1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,
				1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1,
				1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1,
				0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0,
				1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1,
				0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1,
				1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1,
				1, 1, 1, 1, 1, 1 }, { 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1,
				0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1,
				1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0,
				1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0,
				0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };
uint8_t ZERO[8][8] = { { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 0, 0, 0, 0, 1 }, {
		1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 0, 0, 1, 1,
		0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 0, 0, 0, 0, 0, 0, 1 }, { 1,
		1, 0, 0, 0, 0, 1, 1 } };
uint8_t ONE[8][8] = { { 1, 1, 1, 0, 0, 1, 1, 1 }, { 1, 1, 0, 0, 0, 1, 1, 1 }, {
		1, 1, 0, 0, 0, 1, 1, 1 }, { 1, 1, 1, 0, 0, 1, 1, 1 }, { 1, 1, 1, 0, 0,
		1, 1, 1 }, { 1, 1, 1, 0, 0, 1, 1, 1 }, { 1, 1, 1, 0, 0, 1, 1, 1 }, { 1,
		0, 0, 0, 0, 0, 0, 1 } };
uint8_t TWO[8][8] = { { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 0, 0, 0, 0, 1 }, {
		1, 0, 1, 1, 1, 0, 0, 1 }, { 1, 1, 1, 1, 0, 0, 1, 1 }, { 1, 1, 0, 0, 1,
		1, 1, 1 }, { 1, 0, 0, 1, 1, 1, 1, 1 }, { 1, 0, 0, 0, 0, 0, 0, 1 }, { 1,
		1, 0, 0, 0, 0, 1, 1 } };
uint8_t THREE[8][8] = { { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 0, 0, 1 }, { 1, 1, 1, 0, 0, 0, 1, 1 }, { 1, 1, 1, 0, 0,
				0, 0, 1 }, { 1, 1, 1, 1, 1, 0, 0, 1 },
		{ 1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 1, 0, 0, 0, 0, 1, 1 } };
uint8_t FOUR[8][8] = { { 1, 1, 1, 1, 0, 0, 1, 1 }, { 1, 1, 1, 0, 0, 0, 1, 1 }, {
		1, 1, 0, 1, 0, 0, 1, 1 }, { 1, 0, 1, 1, 0, 0, 1, 1 }, { 1, 0, 0, 0, 0,
		0, 0, 1 }, { 1, 0, 0, 0, 0, 0, 0, 1 }, { 1, 1, 1, 1, 0, 0, 1, 1 }, { 1,
		1, 1, 1, 0, 0, 1, 1 } };
uint8_t FIVE[8][8] = { { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 1, 1, 1, 1, 1 }, {
		1, 0, 0, 1, 1, 1, 1, 1 }, { 1, 0, 0, 0, 0, 0, 1, 1 }, { 1, 1, 1, 1, 1,
		0, 0, 1 }, { 1, 1, 1, 1, 1, 0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 }, { 1,
		1, 0, 0, 0, 0, 1, 1 } };
uint8_t SIX[8][8] = { { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 1, 1, 1, 0, 1 }, {
		1, 0, 0, 1, 1, 1, 1, 1 }, { 1, 0, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 1, 1,
		0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 }, { 1,
		1, 0, 0, 0, 0, 1, 1 } };
uint8_t SEVEN[8][8] = { { 1, 0, 0, 0, 0, 0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 },
		{ 1, 1, 1, 1, 1, 0, 0, 1 }, { 1, 1, 1, 1, 0, 0, 1, 1 }, { 1, 1, 1, 0, 0,
				1, 1, 1 }, { 1, 1, 1, 0, 0, 1, 1, 1 },
		{ 1, 1, 1, 0, 0, 1, 1, 1 }, { 1, 1, 1, 0, 0, 1, 1, 1 } };
uint8_t EIGHT[8][8] = { { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 },
		{ 1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 1, 1,
				0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 },
		{ 1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 1, 0, 0, 0, 0, 1, 1 } };
uint8_t NINE[8][8] = { { 1, 1, 0, 0, 0, 0, 1, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 }, {
		1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 0, 0, 1, 1, 0, 0, 1 }, { 1, 1, 0, 0, 0,
		0, 0, 1 }, { 1, 1, 1, 1, 1, 0, 0, 1 }, { 1, 1, 1, 1, 1, 0, 0, 1 }, { 1,
		1, 1, 1, 1, 0, 0, 1 } };
int main(void) {

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
	MX_ETH_Init();
	MX_USART3_UART_Init();
	MX_USB_OTG_FS_PCD_Init();
	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */
	MX_FREERTOS_Init();

	/* Start scheduler */
	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}
void ReviseID(uint8_t num, uint8_t digit) {
	uint8_t (*ptr)[8];
	switch (num) {
	case 0:
		ptr = ZERO;
		break;
	case 1:
		ptr = ONE;
		break;
	case 2:
		ptr = TWO;
		break;
	case 3:
		ptr = THREE;
		break;
	case 4:
		ptr = FOUR;
		break;
	case 5:
		ptr = FIVE;
		break;
	case 6:
		ptr = SIX;
		break;
	case 7:
		ptr = SEVEN;
		break;
	case 8:
		ptr = EIGHT;
		break;
	case 9:
		ptr = NINE;
		break;
	}
	for (uint8_t i = 0; i < 8; i++) {
		for (uint8_t j = 0; j < 8; j++) {
			STUDID[i][j + (digit - 1) * 8] = ptr[i][j];
		}
	}
}
void MX_FREERTOS_Init(void) {
	xOneShotTimer = xTimerCreate("OneShot",
	mainONE_SHOT_TIMER_PERIOD,
	pdFALSE, 0, prvOneShotTimerCallback);

	xModifyTimer = xTimerCreate("Modify",
	mainONE_SHOT_TIMER_PERIOD * 3,
	pdFALSE, 0, prvModifyTimerCallback);

	xAutoReloadTimer = xTimerCreate("AutoReload",
	mainAUTO_RELOAD_TIMER_PERIOD,
	pdTRUE, 0, prvAutoReloadTimerCallback);

	xCountingSemaphore = xSemaphoreCreateCounting(10, 0);

	ReviseID(2, 1);
	ReviseID(1, 2);
	ReviseID(1, 3);
	ReviseID(6, 4);
	ReviseID(0, 5);
	ReviseID(6, 6);

//	xSemone = xSemaphoreCreateCounting(10, 0);
//	xSemtwo = xSemaphoreCreateCounting(10, 0);
//	xSemthree = xSemaphoreCreateCounting(10, 0);
//	xSemfour = xSemaphoreCreateCounting(10, 0);
//	xSemfive = xSemaphoreCreateCounting(10, 0);

	xQueue = xQueueCreate(10, sizeof(uint8_t));
	xSecQueue = xQueueCreate(10, sizeof(uint8_t));
	xOneQueue = xQueueCreate(10, sizeof(uint8_t));
	xFiveQueue = xQueueCreate(10, sizeof(uint8_t));

	if (xOneShotTimer != NULL && xModifyTimer != NULL && xAutoReloadTimer != NULL && xCountingSemaphore != NULL && xQueue != NULL && xSecQueue != NULL && xOneQueue != NULL && xFiveQueue != NULL) {
		xTaskCreate(SwitchTask, "SwitchTask", STACK_SIZE, NULL,
		tskIDLE_PRIORITY + 3, NULL);
		xTaskCreate(WriteNumTask, "WriteNumTask", STACK_SIZE, NULL,
		tskIDLE_PRIORITY + 2, NULL);
		xTaskCreate(ModeTask, "ModeTask", STACK_SIZE, NULL,
		tskIDLE_PRIORITY + 2, NULL);
		xTaskCreate(ModeactTask, "ModeactTask", STACK_SIZE, NULL,
		tskIDLE_PRIORITY + 2, NULL);
		xTaskCreate(ModefiveTask, "ModefiveTask", STACK_SIZE, NULL,
		tskIDLE_PRIORITY + 2, NULL);
	}
}
static void prvModifyTimerCallback(TimerHandle_t xTimer) {
	modifychk = 1;
}
static void prvOneShotTimerCallback(TimerHandle_t xTimer) {
	secchk = 1;
}
static void prvAutoReloadTimerCallback(TimerHandle_t xTimer) {
	moving_point++;
	if (moving_point > 55) {
		moving_point = 0;
	}
	if (moving_point == 48) {
		shift_count++;
	}
}

void ModeactTask(void *argument) {
	uint8_t flag = 0;
	uint8_t num;
	uint8_t (*ptr)[56];
	uint8_t cur;
	while (1) {
		if (flag == 0) {
			xQueueReceive(xOneQueue, &num, portMAX_DELAY);
			switch (num) {
			case 1:
			case 2:
				ptr = SOGANG;
				break;
			case 3:
			case 4:
				ptr = STUDID;
				break;
			}
			flag = 1;
		}
		cur = moving_point;
		for (uint8_t i = 0; i < 8; i++) {
			uint8_t ti = (cur + i) > 55 ? (cur + i - 56) : (cur + i);
			uint16_t COL;

			switch (i) {
			case 0:
				COL = GPIO_PIN_8;
				break;
			case 1:
				COL = GPIO_PIN_9;
				break;
			case 2:
				COL = GPIO_PIN_10;
				break;
			case 3:
				COL = GPIO_PIN_11;
				break;
			case 4:
				COL = GPIO_PIN_12;
				break;
			case 5:
				COL = GPIO_PIN_13;
				break;
			case 6:
				COL = GPIO_PIN_14;
				break;
			case 7:
				COL = GPIO_PIN_15;
				break;
			}

			HAL_GPIO_WritePin(GPIOE, COL, GPIO_PIN_SET);
			for (uint8_t j = 0; j < 8; j++) {
				uint16_t ROW;
				switch (j) {
				case 0:
					ROW = GPIO_PIN_0;
					break;
				case 1:
					ROW = GPIO_PIN_1;
					break;
				case 2:
					ROW = GPIO_PIN_2;
					break;
				case 3:
					ROW = GPIO_PIN_3;
					break;
				case 4:
					ROW = GPIO_PIN_4;
					break;
				case 5:
					ROW = GPIO_PIN_5;
					break;
				case 6:
					ROW = GPIO_PIN_6;
					break;
				case 7:
					ROW = GPIO_PIN_7;
					break;
				}
				if (ptr[j][ti] == 1) {
					HAL_GPIO_WritePin(GPIOD, ROW, GPIO_PIN_SET);
				} else {
					HAL_GPIO_WritePin(GPIOD, ROW, GPIO_PIN_RESET);
				}
			}
			HAL_GPIO_WritePin(GPIOE, COL, GPIO_PIN_RESET);
		}
		if (shift_count == 3) {
			xTimerStop(xAutoReloadTimer, 0);
			mode = 0;
			xQueueReceive(xOneQueue, &num, portMAX_DELAY);
			switch (num) {
			case 1:
			case 2:
				ptr = SOGANG;
				break;
			case 3:
			case 4:
				ptr = STUDID;
				break;
			}
		}
	}
}
void ModefiveTask(void *argument) {
	uint8_t (*ptr)[8];
	uint8_t flag = 0;
	while (1) {
		uint8_t num;
		xQueueReceive(xFiveQueue, &num, portMAX_DELAY);
		mode = 6;
		switch (num) {
		case 0:
			ptr = ZERO;
			break;
		case 1:
			ptr = ONE;
			break;
		case 2:
			ptr = TWO;
			break;
		case 3:
			ptr = THREE;
			break;
		case 4:
			ptr = FOUR;
			break;
		case 5:
			ptr = FIVE;
			break;
		case 6:
			ptr = SIX;
			break;
		case 7:
			ptr = SEVEN;
			break;
		case 8:
			ptr = EIGHT;
			break;
		case 9:
			ptr = NINE;
			break;
		}
		if (flag == 0) {
			xTimerStart(xModifyTimer, 0);
			flag = 1;
		} else
			xTimerReset(xModifyTimer, 0);
		while (1) {
			for (uint8_t i = 0; i < 8; i++) {
				uint16_t COL;
				switch (i) {
				case 0:
					COL = GPIO_PIN_8;
					break;
				case 1:
					COL = GPIO_PIN_9;
					break;
				case 2:
					COL = GPIO_PIN_10;
					break;
				case 3:
					COL = GPIO_PIN_11;
					break;
				case 4:
					COL = GPIO_PIN_12;
					break;
				case 5:
					COL = GPIO_PIN_13;
					break;
				case 6:
					COL = GPIO_PIN_14;
					break;
				case 7:
					COL = GPIO_PIN_15;
					break;
				}
				HAL_GPIO_WritePin(GPIOE, COL, GPIO_PIN_SET);
				for (uint8_t j = 0; j < 8; j++) {
					uint16_t ROW;
					switch (j) {
					case 0:
						ROW = GPIO_PIN_0;
						break;
					case 1:
						ROW = GPIO_PIN_1;
						break;
					case 2:
						ROW = GPIO_PIN_2;
						break;
					case 3:
						ROW = GPIO_PIN_3;
						break;
					case 4:
						ROW = GPIO_PIN_4;
						break;
					case 5:
						ROW = GPIO_PIN_5;
						break;
					case 6:
						ROW = GPIO_PIN_6;
						break;
					case 7:
						ROW = GPIO_PIN_7;
						break;
					}
					if (ptr[j][i] == 1) {
						HAL_GPIO_WritePin(GPIOD, ROW, GPIO_PIN_SET);
					} else {
						HAL_GPIO_WritePin(GPIOD, ROW, GPIO_PIN_RESET);
					}
				}
				HAL_GPIO_WritePin(GPIOE, COL, GPIO_PIN_RESET);
			}
			if (modifychk == 1) {
				break;
			}
			if(mode == 7){
				break;
			}
		}
		if (modifychk == 1) {
			if (dig < 6)
				ReviseID(num, dig);
			else {
				ReviseID(num, dig);
				mode = 0;
				dig = 0;
			}
			dig++;
			flag = 0;
			modifychk = 0;
		}

	}
}
void ModeTask(void *argument) {
	uint8_t num;
	while (1) {
		xQueueReceive(xSecQueue, &num, portMAX_DELAY);
		secchk = 0;

		switch (num) {
		case 1:
		case 3:
			mode = num;
			xTimerChangePeriod(xAutoReloadTimer, mainAUTO_RELOAD_TIMER_PERIOD,
					0);
			xTimerStart(xAutoReloadTimer, 0);
			xQueueSend(xOneQueue, &num, 0);
			break;
		case 2:
		case 4:
			mode = num;
			xTimerChangePeriod(xAutoReloadTimer,
					mainAUTO_RELOAD_TIMER_PERIOD / 2, 0);
			xTimerStart(xAutoReloadTimer, 0);
			xQueueSend(xOneQueue, &num, 0);
			break;
		case 5:
			mode = num;
			num = 0;
			xQueueSend(xFiveQueue, &num, 0);
			break;
		}
	}
}
void WriteNumTask(void *argument) {
	uint8_t (*ptr)[8];
	uint8_t flag = 0;
	while (1) {
		uint8_t num;
		xQueueReceive(xQueue, &num, portMAX_DELAY);
		mode = 8;
		switch (num) {
		case 1:
			ptr = ONE;
			break;
		case 2:
			ptr = TWO;
			break;
		case 3:
			ptr = THREE;
			break;
		case 4:
			ptr = FOUR;
			break;
		case 5:
			ptr = FIVE;
			break;
		}
		if (flag == 0) {
			xTimerStart(xOneShotTimer, 0);
			flag = 1;
		} else
			xTimerReset(xOneShotTimer, 0);
		while (1) {
			for (uint8_t i = 0; i < 8; i++) {
				uint16_t COL;
				switch (i) {
				case 0:
					COL = GPIO_PIN_8;
					break;
				case 1:
					COL = GPIO_PIN_9;
					break;
				case 2:
					COL = GPIO_PIN_10;
					break;
				case 3:
					COL = GPIO_PIN_11;
					break;
				case 4:
					COL = GPIO_PIN_12;
					break;
				case 5:
					COL = GPIO_PIN_13;
					break;
				case 6:
					COL = GPIO_PIN_14;
					break;
				case 7:
					COL = GPIO_PIN_15;
					break;
				}
				HAL_GPIO_WritePin(GPIOE, COL, GPIO_PIN_SET);
				for (uint8_t j = 0; j < 8; j++) {
					uint16_t ROW;
					switch (j) {
					case 0:
						ROW = GPIO_PIN_0;
						break;
					case 1:
						ROW = GPIO_PIN_1;
						break;
					case 2:
						ROW = GPIO_PIN_2;
						break;
					case 3:
						ROW = GPIO_PIN_3;
						break;
					case 4:
						ROW = GPIO_PIN_4;
						break;
					case 5:
						ROW = GPIO_PIN_5;
						break;
					case 6:
						ROW = GPIO_PIN_6;
						break;
					case 7:
						ROW = GPIO_PIN_7;
						break;
					}
					if (ptr[j][i] == 1) {
						HAL_GPIO_WritePin(GPIOD, ROW, GPIO_PIN_SET);
					} else {
						HAL_GPIO_WritePin(GPIOD, ROW, GPIO_PIN_RESET);
					}
				}
				HAL_GPIO_WritePin(GPIOE, COL, GPIO_PIN_RESET);
			}
			if (secchk == 1) {
				mode = 10;
				break;
			}
			if (mode == 9) {
				break;
			}
		}
		if (secchk == 1) {
			flag = 0;
			xQueueSend(xSecQueue, &num, 0);
		}
	}
}
void SwitchTask(void *argument) {
	uint8_t scounter = 0;
	uint8_t modefive = 0;
	uint8_t cur_dig = 10;
	while (1) {
		xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);
		if (mode == 5 || mode == 7) {
			modefive++;
			if (modefive > 9 || cur_dig != dig)
				modefive = 0;
			if (modifychk == 0)
				xQueueSend(xFiveQueue, &modefive, 0);
			cur_dig = dig;
		} else {
			secchk = 0;
			scounter++;
			if (scounter > 5)
				scounter = 1;
			xQueueSend(xQueue, &scounter, 0);
		}

	}
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure LSE Drive Capability
	 */
	HAL_PWR_EnableBkUpAccess();

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 72;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 3;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ETH Initialization Function
 * @param None
 * @retval None
 */
static void MX_ETH_Init(void) {

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

	if (HAL_ETH_Init(&heth) != HAL_OK) {
		Error_Handler();
	}

	memset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
	TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM
			| ETH_TX_PACKETS_FEATURES_CRCPAD;
	TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
	TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
	/* USER CODE BEGIN ETH_Init 2 */

	/* USER CODE END ETH_Init 2 */

}

/**
 * @brief USART3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART3_UART_Init(void) {

	/* USER CODE BEGIN USART3_Init 0 */

	/* USER CODE END USART3_Init 0 */

	/* USER CODE BEGIN USART3_Init 1 */

	/* USER CODE END USART3_Init 1 */
	huart3.Instance = USART3;
	huart3.Init.BaudRate = 115200;
	huart3.Init.WordLength = UART_WORDLENGTH_8B;
	huart3.Init.StopBits = UART_STOPBITS_1;
	huart3.Init.Parity = UART_PARITY_NONE;
	huart3.Init.Mode = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;
	huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART3_Init 2 */

	/* USER CODE END USART3_Init 2 */

}

/**
 * @brief USB_OTG_FS Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_OTG_FS_PCD_Init(void) {

	/* USER CODE BEGIN USB_OTG_FS_Init 0 */

	/* USER CODE END USB_OTG_FS_Init 0 */

	/* USER CODE BEGIN USB_OTG_FS_Init 1 */

	/* USER CODE END USB_OTG_FS_Init 1 */
	hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
	hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
	hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
	hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
	hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
	hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
	hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
	if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USB_OTG_FS_Init 2 */

	/* USER CODE END USB_OTG_FS_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LD1_Pin | LD3_Pin | LD2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
			GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
					| GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin,
			GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD,
			GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4
					| GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_RESET);

	/*Configure GPIO pin : USER_Btn_Pin */
	GPIO_InitStruct.Pin = USER_Btn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
	GPIO_InitStruct.Pin = LD1_Pin | LD3_Pin | LD2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PE8 PE9 PE10 PE11
	 PE12 PE13 PE14 PE15 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11
			| GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_PowerSwitchOn_Pin */
	GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : USB_OverCurrent_Pin */
	GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PD0 PD1 PD2 PD3
	 PD4 PD5 PD6 PD7 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
			| GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	static uint32_t pre_itr_t = 0;
	uint32_t cur_itr_t = HAL_GetTick();
	if ((cur_itr_t - pre_itr_t > 80)) {
		shift_count = 0;
		moving_point = 0;
		switch (mode) {
		case 0:
		case 5:
			xSemaphoreGiveFromISR(xCountingSemaphore, NULL);
			break;
		case 6:
			mode = 7;
			xSemaphoreGiveFromISR(xCountingSemaphore, NULL);
			break;
		case 8:
			mode = 9;
			xSemaphoreGiveFromISR(xCountingSemaphore, NULL);
			break;
		default:
			break;
		}
	}
	pre_itr_t = cur_itr_t;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM4 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM4) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
