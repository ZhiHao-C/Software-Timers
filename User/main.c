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

/**************************** ȫ�ֱ��� ********************************/

#define KEY1_EVENT (0x01 << 0)//�����¼������λ 0
#define KEY2_EVENT (0x01 << 1)//�����¼������λ 1

static uint32_t TmrCb_Count1 = 0; /* ��¼�����ʱ�� 1 �ص�����ִ�д��� */
static uint32_t TmrCb_Count2 = 0; /* ��¼�����ʱ�� 2 �ص�����ִ�д��� */

/**************************** ������ ********************************/
/* 
 * ��������һ��ָ�룬����ָ��һ�����񣬵����񴴽���֮�����;�����һ��������
 * �Ժ�����Ҫ��������������Ҫͨ�������������������������������Լ�����ô
 * ����������ΪNULL��
 */
 /* ���������� */
static TaskHandle_t AppTaskCreate_Handle = NULL;
//���������ʱ���
static TimerHandle_t  Swtmr1_Handle =NULL;
//���������ʱ���
static TimerHandle_t  Swtmr2_Handle =NULL;



//��������
static void AppTaskCreate(void);
static void Swtmr1_Callback(void* parameter);
static void Swtmr2_Callback(void* parameter);


static void BSP_Init(void)
{
	/* 
	* STM32 �ж����ȼ�����Ϊ 4���� 4bit ��������ʾ��ռ���ȼ�����ΧΪ��0~15 
	* ���ȼ�����ֻ��Ҫ����һ�μ��ɣ��Ժ������������������Ҫ�õ��жϣ� 
	* ��ͳһ��������ȼ����飬ǧ��Ҫ�ٷ��飬�мɡ� 
	*/ 
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 
	LED_GPIO_Config();
	KEY1_GPIO_Config();
	KEY2_GPIO_Config();
	USART_Config();
	
	//����
//	led_G(on);
//	printf("���ڲ���");
}

int main()
{
	BaseType_t xReturn = NULL;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
	
	
	
	BSP_Init();
	printf("����ȫϵ�п�����-FreeRTOS-��̬��������!\r\n");

	  /* ����AppTaskCreate���� */
  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* ������ں��� */
                        (const char*    )"AppTaskCreate",/* �������� */
                        (uint16_t       )512,  /* ����ջ��С */
                        (void*          )NULL,/* ������ں������� */
                        (UBaseType_t    )1, /* ��������ȼ� */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* ������ƿ�ָ�� */ 
																							
	if(xReturn==pdPASS)
	{
		printf("��ʼ���񴴽��ɹ�\r\n");
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
	printf("swtmr1_callback ����ִ�� %d ��\n", TmrCb_Count1); 
	printf("��ǰ�δ��������ֵΪ�� %d \n", tickcount1); 
}


static void Swtmr2_Callback(void* parameter)
{
	TickType_t tickcount2=0;
	
	TmrCb_Count2++;
	tickcount2=xTaskGetTickCount();
	LED_G_TOGGLE();
	printf("swtmr2_callback ����ִ�� %d ��\n", TmrCb_Count2); 
	printf("��ǰ�δ��������ֵΪ�� %d \n", tickcount2); 
}



static void AppTaskCreate(void)
{
	
	taskENTER_CRITICAL();           //�����ٽ���

	
	 Swtmr1_Handle=xTimerCreate((const char* )"AutoReloadTimer",		//��ʱ��������
															(TickType_t)1000,/*��ʱ������ 1000(tick) */ 
															(UBaseType_t)pdTRUE,/* ����ģʽ *///pdTRUE Ϊ����ģʽ��pdFALS Ϊ����ģʽ
	                            (void*)1,           /*Ϊÿ����ʱ������һ��������Ψһ ID */ 
															(TimerCallbackFunction_t)Swtmr1_Callback);/* �����е��õĻص��������Լ�ʵ�֣� */ 

   if(Swtmr1_Handle!=NULL)
	 {
		 xTimerStart(Swtmr1_Handle,portMAX_DELAY);
	 }		 
	 
	 Swtmr2_Handle=xTimerCreate((const char* )"OneShotTimer",		//��ʱ��������
															(TickType_t)5000,/*��ʱ������ 5000(tick) */ 
															(UBaseType_t)pdFALSE,/* ����ģʽ *///pdTRUE Ϊ����ģʽ��pdFALS Ϊ����ģʽ
	                            (void*)2,            /*Ϊÿ����ʱ������һ��������Ψһ ID */ 
															(TimerCallbackFunction_t)Swtmr2_Callback);/* �����е��õĻص��������Լ�ʵ�֣� */ 
															

	 if(Swtmr2_Handle!=NULL)
	 {
		 xTimerStart(Swtmr2_Handle,portMAX_DELAY);
	 }
	
	vTaskDelete(AppTaskCreate_Handle); //ɾ��AppTaskCreate����
	
	taskEXIT_CRITICAL();            //�˳��ٽ���
}


//��̬�����������Ҫ
///**
//  **********************************************************************
//  * @brief  ��ȡ��������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
//								   StackType_t **ppxIdleTaskStackBuffer, 
//								   uint32_t *pulIdleTaskStackSize)
//{
//	*ppxIdleTaskTCBBuffer=&Idle_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxIdleTaskStackBuffer=Idle_Task_Stack;/* �����ջ�ڴ� */
//	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;/* �����ջ��С */
//}



///**
//  *********************************************************************
//  * @brief  ��ȡ��ʱ������������ջ��������ƿ��ڴ�
//	*					ppxTimerTaskTCBBuffer	:		������ƿ��ڴ�
//	*					ppxTimerTaskStackBuffer	:	�����ջ�ڴ�
//	*					pulTimerTaskStackSize	:		�����ջ��С
//  * @author  fire
//  * @version V1.0
//  * @date    2018-xx-xx
//  **********************************************************************
//  */ 
//void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
//									StackType_t **ppxTimerTaskStackBuffer, 
//									uint32_t *pulTimerTaskStackSize)
//{
//	*ppxTimerTaskTCBBuffer=&Timer_Task_TCB;/* ������ƿ��ڴ� */
//	*ppxTimerTaskStackBuffer=Timer_Task_Stack;/* �����ջ�ڴ� */
//	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;/* �����ջ��С */
//}
