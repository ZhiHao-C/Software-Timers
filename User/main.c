//#include "stm32f10x.h"                  // Device header
#include "string.h"
#include <stdio.h>

#include "bps_led.h"
#include "bps_usart.h"
#include "key.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/**************************** 全局变量 ********************************/

#define KEY1_EVENT (0x01 << 0)//设置事件掩码的位 0
#define KEY2_EVENT (0x01 << 1)//设置事件掩码的位 1

static uint32_t TmrCb_Count1 = 0; /* 记录软件定时器 1 回调函数执行次数 */
static uint32_t TmrCb_Count2 = 0; /* 记录软件定时器 2 回调函数执行次数 */

/**************************** 任务句柄 ********************************/
/* 
 * 任务句柄是一个指针，用于指向一个任务，当任务创建好之后，它就具有了一个任务句柄
 * 以后我们要想操作这个任务都需要通过这个任务句柄，如果是自身的任务操作自己，那么
 * 这个句柄可以为NULL。
 */
 /* 创建任务句柄 */
static TaskHandle_t AppTaskCreate_Handle = NULL;
//创建软件延时句柄
static TimerHandle_t  Swtmr1_Handle =NULL;
//创建软件延时句柄
static TimerHandle_t  Swtmr2_Handle =NULL;



//声明函数
static void AppTaskCreate(void);
static void Swtmr1_Callback(void* parameter);
static void Swtmr2_Callback(void* parameter);


static void BSP_Init(void)
{
	/* 
	* STM32 中断优先级分组为 4，即 4bit 都用来表示抢占优先级，范围为：0~15 
	* 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断， 
	* 都统一用这个优先级分组，千万不要再分组，切忌。 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//测试
//	led_G(on);
//	printf("串口测试");
}

int main()
{
	BaseType_t xReturn = NULL;/* 定义一个创建信息返回值，默认为pdPASS */
	
	
	
	BSP_Init();
	printf("这是全系列开发板-FreeRTOS-动态创建任务!\r\n");

	  /* 创建AppTaskCreate任务 */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
																							
	if(xReturn==pdPASS)
	{
		printf("初始任务创建成功\r\n");
		vTaskStartScheduler();
	}
	else 
	{
		return -1;
	}
	while(1)
	{
		
	}

}



static void Swtmr1_Callback(void* parameter)
{
	TickType_t tickcount1=0;
	
	TmrCb_Count1++;
	tickcount1=xTaskGetTickCount();
	LED_G_TOGGLE();
	printf("swtmr1_callback 函数执行 %d 次\n", TmrCb_Count1); 
	printf("当前滴答计数器的值为： %d \n", tickcount1); 
}


static void Swtmr2_Callback(void* parameter)
{
	TickType_t tickcount2=0;
	
	TmrCb_Count2++;
	tickcount2=xTaskGetTickCount();
	LED_G_TOGGLE();
	printf("swtmr2_callback 函数执行 %d 次\n", TmrCb_Count2); 
	printf("当前滴答计数器的值为： %d \n", tickcount2); 
}



static void AppTaskCreate(void)
{
	
	taskENTER_CRITICAL();           //进入临界区

	
	 Swtmr1_Handle=xTimerCreate((const char* )"AutoReloadTimer",		//定时函数名称
															(TickType_t)1000,/*定时器周期 1000(tick) */ 
															(UBaseType_t)pdTRUE,/* 周期模式 *///pdTRUE 为周期模式，pdFALS 为单次模式
	                            (void*)1,           /*为每个计时器分配一个索引的唯一 ID */ 
															(TimerCallbackFunction_t)Swtmr1_Callback);/* 任务中调用的回调函数（自己实现） */ 

   if(Swtmr1_Handle!=NULL)
	 {
		 xTimerStart(Swtmr1_Handle,portMAX_DELAY);
	 }		 
	 
	 Swtmr2_Handle=xTimerCreate((const char* )"OneShotTimer",		//定时函数名称
															(TickType_t)5000,/*定时器周期 5000(tick) */ 
															(UBaseType_t)pdFALSE,/* 周期模式 *///pdTRUE 为周期模式，pdFALS 为单次模式
	                            (void*)2,            /*为每个计时器分配一个索引的唯一 ID */ 
															(TimerCallbackFunction_t)Swtmr2_Callback);/* 任务中调用的回调函数（自己实现） */ 
															

	 if(Swtmr2_Handle!=NULL)
	 {
		 xTimerStart(Swtmr2_Handle,portMAX_DELAY);
	 }
	
	vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
	
	taskEXIT_CRITICAL();            //退出临界区
}


//静态创建任务才需要
///**
//  **********************************************************************
//  * @brief  获取空闲任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* 任务控制块内存 */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* 任务堆栈内存 */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* 任务堆栈大小 */
//}



///**
//  *********************************************************************
//  * @brief  获取定时器任务的任务堆栈和任务控制块内存
//	*					ppxTimerTaskTCBBuffer	:		任务控制块内存
//	*					ppxTimerTaskStackBuffer	:	任务堆栈内存
//	*					pulTimerTaskStackSize	:		任务堆栈大小
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* 任务控制块内存 */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* 任务堆栈内存 */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* 任务堆栈大小 */
//}
