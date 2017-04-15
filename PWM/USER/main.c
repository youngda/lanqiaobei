
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


	   	/******PWM电平跳变值**********/
//		uint16_t CCR2_Val=500;
 
	/******第一部分 时基初始化**********/
       	 TIM_DeInit(TIM1); //重设为缺省值
		 /*TIM1时钟配置*/
		 TIM_TimeBaseStructure.TIM_Period = period;//装载值 
		 TIM_TimeBaseStructure.TIM_Prescaler = 71; //预分频(时钟分频)
		 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计数
		 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;   //设置了时钟分割 不懂得不管
		 TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;    //周期计数器值  不懂得不管
		 TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);    //初始化TIMx的时间基数单位
		
  	/******第二部分 输出模式初始化**********/
		 /* Channel 1 Configuration in PWM mode 通道一的PWM */
		 TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;     //PWM模式2
		 TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  //正向通道有效  PA8 
		 TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable; //反向通道也有效 PB13
 	/******第三部分 装载跳变值**********/
		 TIM_OCInitStructure.TIM_Pulse = CCR2_Val;        //占空时间  	   CCR2_Val
		 TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;   //输出极性
		 TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;  //使能该通道输出
	     //下面几个参数是高级定时器才会用到，通用定时器不用配置
		 TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;     //互补端的极性 
		 TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;//使能互补端输出 
		 TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;  //空闲状态下的非工作状态 不管
		 TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;  //先不管
		
		 TIM_OC2Init(TIM1,&TIM_OCInitStructure);       //数初始化外设TIMx通道1这里2.0库为TIM_OCInit
     	 TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable);//使能预装载寄存器

	/******死区和刹车功能配置，高级定时器才有的，通用定时器不用配置******/
		TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;//运行模式下输出选择
		TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;//空闲模式下输出选择 
		TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;    //锁定设置
		TIM_BDTRInitStructure.TIM_DeadTime = 50;                  //死区时间设置
		TIM_BDTRInitStructure.TIM_Break = TIM_Break_Enable;        //刹车功能使能
		TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;//刹车输入极性
		TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;//自动输出使能 
	
		TIM_BDTRConfig(TIM1,&TIM_BDTRInitStructure);

		TIM_ARRPreloadConfig(TIM1,ENABLE);	  //使能重装载寄存器
		 /* TIM1 counter enable开定时器 */
//		 TIM_Cmd(TIM1,ENABLE);
		      /* TIM1 Main Output Enable 使能TIM1外设的主输出*/
//		 TIM_CtrlPWMOutputs(TIM1, ENABLE);  //pwm输出使能，一定要记得打开
	/*TIM_OC1PreloadConfig(),TIM_ARRPreloadConfig();这两个函数控制的是ccr1和arr的预装载使能，
	使能和失能的区别就是：使能的时候这两个寄存器的读写需要等待有更新事件发生时才能被改变
	（比如计数溢出就是更新时间）。
	失能的时候可以直接进行读写而没有延迟。
	 另外在运行当中想要改变pwm的频率和占空比调用：TIM_SetAutoreload() 
	TIM_SetCompare1()这两个函数就可以了。*/


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

        /*PB13 设置为PWM的反极性输出*/
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
