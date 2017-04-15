
#include <stdio.h>
#include "stm32f10x.h"
#include "lcd.h"
#include "led.h"
u32 TimingDelay = 0;


void LCD_init(void);
void Delay_Ms(u32 nTime);
void LED_init(void);
void LED_C(u16 led,u8 status);
void GPIO_Int(void);
void TIM1_Configuration(uint16_t period,uint16_t CCR2_Val);
#define RB1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
#define RB2 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)
#define RB3 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)
#define RB4 GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2)
u32 v = 1;
void key_scan(void);
void TIM1_cmd(void);
int main(void)
{	
	SysTick_Config(SystemCoreClock/1000);
	LCD_init();
	LED_init();
	LED_C(LEDALL,0);
	GPIO_Int();
	TIM1_Configuration(399,80);
	TIM1_cmd();
	while(1)
	{	
	}
}
void TIM1_Configuration(uint16_t period,uint16_t CCR2_Val)
{
	   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
       TIM_OCInitTypeDef  TIM_OCInitStructure;
	   TIM_BDTRInitTypeDef TIM_BDTRInitStructure;


	   	/******PWM��ƽ����ֵ**********/
//		uint16_t CCR2_Val=500;
 
	/******��һ���� ʱ����ʼ��**********/
       	 TIM_DeInit(TIM1); //����Ϊȱʡֵ
		 /*TIM1ʱ������*/
		 TIM_TimeBaseStructure.TIM_Period = period;//װ��ֵ 
		 TIM_TimeBaseStructure.TIM_Prescaler = 71; //Ԥ��Ƶ(ʱ�ӷ�Ƶ)
		 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //���ϼ���
		 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;   //������ʱ�ӷָ� �����ò���
		 TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;    //���ڼ�����ֵ  �����ò���
		 TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);    //��ʼ��TIMx��ʱ�������λ
		
  	/******�ڶ����� ���ģʽ��ʼ��**********/
		 /* Channel 1 Configuration in PWM mode ͨ��һ��PWM */
		 TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;     //PWMģʽ2
		 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  //����ͨ����Ч  PA8 
		 TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; //����ͨ��Ҳ��Ч PB13
 	/******�������� װ������ֵ**********/
		 TIM_OCInitStructure.TIM_Pulse = CCR2_Val;        //ռ��ʱ��  	   CCR2_Val
		 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;   //�������
		 TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;  //ʹ�ܸ�ͨ�����
	     //���漸�������Ǹ߼���ʱ���Ż��õ���ͨ�ö�ʱ����������
		 TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;     //�����˵ļ��� 
		 TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//ʹ�ܻ�������� 
		 TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;  //����״̬�µķǹ���״̬ ����
		 TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;  //�Ȳ���
		
		 TIM_OC2Init(TIM1,&TIM_OCInitStructure);       //����ʼ������TIMxͨ��1����2.0��ΪTIM_OCInit
     	 TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable);//ʹ��Ԥװ�ؼĴ���

	/******������ɲ���������ã��߼���ʱ�����еģ�ͨ�ö�ʱ����������******/
		TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//����ģʽ�����ѡ��
		TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//����ģʽ�����ѡ�� 
		TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;    //��������
		TIM_BDTRInitStructure.TIM_DeadTime = 50;                  //����ʱ������
		TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;        //ɲ������ʹ��
		TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;//ɲ�����뼫��
		TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;//�Զ����ʹ�� 
	
		TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);

		TIM_ARRPreloadConfig(TIM1,ENABLE);	  //ʹ����װ�ؼĴ���
		 /* TIM1 counter enable����ʱ�� */
//		 TIM_Cmd(TIM1,ENABLE);
		      /* TIM1 Main Output Enable ʹ��TIM1����������*/
//		 TIM_CtrlPWMOutputs(TIM1, ENABLE);  //pwm���ʹ�ܣ�һ��Ҫ�ǵô�
	/*TIM_OC1PreloadConfig(),TIM_ARRPreloadConfig();�������������Ƶ���ccr1��arr��Ԥװ��ʹ�ܣ�
	ʹ�ܺ�ʧ�ܵ�������ǣ�ʹ�ܵ�ʱ���������Ĵ����Ķ�д��Ҫ�ȴ��и����¼�����ʱ���ܱ��ı�
	���������������Ǹ���ʱ�䣩��
	ʧ�ܵ�ʱ�����ֱ�ӽ��ж�д��û���ӳ١�
	 ���������е�����Ҫ�ı�pwm��Ƶ�ʺ�ռ�ձȵ��ã�TIM_SetAutoreload() 
	TIM_SetCompare1()�����������Ϳ����ˡ�*/


}
void TIM1_cmd(void)
{
    TIM_Cmd(TIM1,ENABLE);
  	TIM_CtrlPWMOutputs(TIM1, ENABLE);
	 
}
void LED_C(u16 led,u8 status)
{
	if(status == 0)
	{
		GPIO_SetBits(GPIOC,led);
		GPIO_SetBits(GPIOD,GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	}
	else if(status == 1)
	{
		GPIO_ResetBits(GPIOC,led);
		GPIO_SetBits(GPIOD,GPIO_Pin_2);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
	}
}
void GPIO_Int(void)
{
	    GPIO_InitTypeDef GPIO_InitStructure;;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);

    /******************************************TIM1 INIT***********************************************************/
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
       GPIO_Init(GPIOA, &GPIO_InitStructure);

        /*PB13 ����ΪPWM�ķ��������*/
       GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
       GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
       GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
       GPIO_Init(GPIOB, &GPIO_InitStructure);

    /******************************************KEY INIT***********************************************************/

	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_8;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD ;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD ;
	    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
	
	  	/******************************************LED INIT***********************************************************/
	
	    GPIO_InitStructure.GPIO_Pin = LEDALL;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	    GPIO_Init(GPIOD, &GPIO_InitStructure);
	
		/******************************************ADC INIT***********************************************************/
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	    GPIO_Init(GPIOB, &GPIO_InitStructure); 
	/******************************************LED Config***********************************************************/
		GPIO_SetBits(GPIOD,GPIO_Pin_2);
	    GPIO_SetBits(GPIOC,LEDALL);
		GPIO_ResetBits(GPIOC,LED1);
		GPIO_ResetBits(GPIOD,GPIO_Pin_2);
 
}
void LED_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /* Configure PD0 and PD2 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
}
void LCD_init(void)
{	
	STM3210B_LCD_Init();
	LCD_Clear(Blue);
	LCD_SetBackColor(Blue);
	LCD_SetTextColor(White);
	LCD_DisplayStringLine(Line0,"                    ");	
	LCD_DisplayStringLine(Line1,"                    ");	
	LCD_DisplayStringLine(Line2,"                    ");
	LCD_DisplayStringLine(Line3,"                    ");
	LCD_DisplayStringLine(Line4,"                    ");					
	LCD_DisplayStringLine(Line5,"                    ");	
	LCD_DisplayStringLine(Line6,"                    ");	
	LCD_DisplayStringLine(Line7,"                    ");	
	LCD_DisplayStringLine(Line8,"                    ");		
	LCD_DisplayStringLine(Line9,"                    ");		
}
void Delay_Ms(u32 nTime)
{
	TimingDelay = nTime;
	while(TimingDelay != 0);	
}
