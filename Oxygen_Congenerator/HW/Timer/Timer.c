/*********************************************************************************************************
* 模块名称：Timer.c
* 摘    要：Timer模块
* 当前版本：1.0.0
* 作    者：Leyutek(COPYRIGHT 2018 - 2021 Leyutek. All rights reserved.)
* 完成日期：2021年07月01日  
* 内    容：
* 注    意：
**********************************************************************************************************
* 取代版本：
* 作    者：
* 完成日期：
* 修改内容：
* 修改文件：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "Timer.h"
#include "gd32f30x_conf.h"
#include "All_SENSORS.h"
#include "PSA.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static unsigned char s_i2msFlag  = FALSE; //将2ms标志位的值设置为FALSE
static unsigned char s_i1secFlag = FALSE; //将1s标志位的值设置为FALSE

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigTimer2(unsigned short arr, unsigned short psc); //配置TIMER2
static void ConfigTimer1(unsigned short arr, unsigned short psc); //配置TIMER1
static void ConfigTimer2PWM(void);
static void ConfigTimer1PWM(void);

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigTimer2
* 函数功能：配置TIMER2 
* 输入参数：arr-自动重装值，psc-预分频器值
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：APB1时钟为60MHz，TIMER2和TIMER1时钟选择为APB1的2倍，因此，TIMER2和TIMER1时钟为120MHz
*********************************************************************************************************/
static void ConfigTimer2(unsigned short arr, unsigned short psc)
{
  timer_parameter_struct timer_initpara;               //timer_initpara用于存放定时器的参数

  //使能RCU相关时钟 
  rcu_periph_clock_enable(RCU_TIMER2);                 //使能TIMER2的时钟

  //复位TIMER2
  timer_deinit(TIMER2);                                //设置TIMER2参数恢复默认值
  timer_struct_para_init(&timer_initpara);             //初始化timer_initpara

  //配置TIMER2
  timer_initpara.prescaler         = psc;              //设置预分频器值
  timer_initpara.counterdirection  = TIMER_COUNTER_UP; //设置向上计数模式
  timer_initpara.period            = arr;              //设置自动重装载值
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1; //设置时钟分割
  timer_init(TIMER2, &timer_initpara);                 //根据参数初始化定时器

  //使能定时器及其中断
  timer_interrupt_enable(TIMER2, TIMER_INT_UP);        //使能定时器的更新中断
  nvic_irq_enable(TIMER2_IRQn, 1, 0);                  //配置NVIC设置优先级
  timer_enable(TIMER2);                                //使能定时器
}

/*********************************************************************************************************
* 函数名称：ConfigTimer4PWM
* 函数功能：配置TIMER3CH为PWM0模式 
* 输入参数：arr-自动重装值，psc-预分频器值
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：APB1时钟为60MHz，TIMER2和TIMER4时钟选择为APB1的2倍，因此，TIMER2和TIMER4时钟为120MHz
*********************************************************************************************************/
static void ConfigTimer2PWM(void)
{
  timer_oc_parameter_struct timer_ocinitpara;
  uint16_t channels_list[2]={/*TIMER_CH_0,*/TIMER_CH_1};
  int i=0;  
  // 打开通道引脚
  rcu_periph_clock_enable(RCU_GPIOA);                 //使能TIMER2的时钟
  rcu_periph_clock_enable(RCU_AF);       //使能AF时钟
  
  gpio_init(GPIOA,GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, /*GPIO_PIN_6|*/GPIO_PIN_7);  //开启复用
  
  timer_channel_output_struct_para_init(&timer_ocinitpara);  
  
  timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;                    //设置通道输出状态
  timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;                  //设置互补通道输出状态
  timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;              //设置通道输出极性
  timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;             //设置互补通道输出极性
  timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;             //设置空闲状态下通道输出极性
  timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;            //设置空闲状态下互补通道输出极性
  
  for(i=0;i<1;i++){
    timer_channel_output_config(TIMER2, channels_list[i], &timer_ocinitpara);  //初始化结构体
  
  timer_channel_output_pulse_value_config(TIMER2, channels_list[i], 50);  //设置占空比
  timer_channel_output_mode_config(TIMER2, channels_list[i], TIMER_OC_MODE_PWM0);         //设置通道比较模式
  timer_channel_output_shadow_config(TIMER2, channels_list[i], TIMER_OC_SHADOW_DISABLE);  //失能比较影子寄存器
  }
  timer_auto_reload_shadow_enable(TIMER2);                                          //自动重载影子使能 
  
  timer_enable(TIMER2);
}

/*********************************************************************************************************
* 函数名称：ConfigTIMER1
* 函数功能：配置TIMER1 
* 输入参数：arr-自动重装值，psc-预分频器值
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：APB1时钟为60MHz，TIMER2和TIMER1时钟选择为APB1的2倍，因此，TIMER2和TIMER1时钟为120MHz
*********************************************************************************************************/
static void ConfigTimer1(unsigned short arr, unsigned short psc)
{
  timer_parameter_struct timer_initpara;                //timer_initpara用于存放定时器的参数

  //使能RCU相关时钟
  rcu_periph_clock_enable(RCU_TIMER1);                  //使能TIMER1的时钟

  //复位TIMER1
  timer_deinit(TIMER1);                                 //设置TIMER1参数恢复默认值
  timer_struct_para_init(&timer_initpara);              //初始化timer_initpara
 
  //配置TIMER1
  timer_initpara.prescaler         = psc;               //设置预分频器值
  timer_initpara.counterdirection  = TIMER_COUNTER_UP;  //设置向上计数模式
  timer_initpara.period            = arr;               //设置自动重装载值
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;  //设置时钟分割
  timer_init(TIMER1, &timer_initpara);                  //根据参数初始化定时器

  //使能定时器及其中断
  timer_interrupt_enable(TIMER1, TIMER_INT_UP);         //使能定时器的更新中断
  nvic_irq_enable(TIMER1_IRQn, 1, 0);                   //配置NVIC设置优先级
  timer_enable(TIMER1);                                 //使能定时器
}

/*********************************************************************************************************
* 函数名称：ConfigTimer4PWM
* 函数功能：配置TIMER3CH为PWM0模式 
* 输入参数：arr-自动重装值，psc-预分频器值
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：APB1时钟为60MHz，TIMER2和TIMER4时钟选择为APB1的2倍，因此，TIMER2和TIMER4时钟为120MHz
*********************************************************************************************************/
static void ConfigTimer1PWM(void)
{
  timer_oc_parameter_struct timer_ocinitpara;
  uint16_t channels_list[4]={TIMER_CH_1,TIMER_CH_0,TIMER_CH_2,TIMER_CH_3};
  int i=0;
  
  // 打开通道引脚
  rcu_periph_clock_enable(RCU_GPIOA);                 //使能TIMER2的时钟
  rcu_periph_clock_enable(RCU_AF);       //使能AF时钟
  
  gpio_init(GPIOA,GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, BITS(0,3));  //开启复用
  
  timer_channel_output_struct_para_init(&timer_ocinitpara);  
  
  timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;                    //设置通道输出状态
  timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;                  //设置互补通道输出状态
  timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;              //设置通道输出极性
  timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;             //设置互补通道输出极性
  timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;             //设置空闲状态下通道输出极性
  timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;            //设置空闲状态下互补通道输出极性
  
  for(i=0;i<4;i++){
    timer_channel_output_config(TIMER1, channels_list[i], &timer_ocinitpara);  //初始化结构体
  
  timer_channel_output_pulse_value_config(TIMER1, channels_list[i], 0);  //设置占空比
  timer_channel_output_mode_config(TIMER1, channels_list[i], TIMER_OC_MODE_PWM0);         //设置通道比较模式
  timer_channel_output_shadow_config(TIMER1, channels_list[i], TIMER_OC_SHADOW_DISABLE);  //失能比较影子寄存器
  }
  
  timer_auto_reload_shadow_enable(TIMER1);                                          //自动重载影子使能 
  
  timer_enable(TIMER1);
}


/*********************************************************************************************************
* 函数名称：TIMER2_IRQHandler
* 函数功能：TIMER2中断服务函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每毫秒进入一次中断服务函数
*********************************************************************************************************/
void TIMER2_IRQHandler(void)
{  
  static  unsigned short s_iCnt2 = 0;                              //定义一个静态变量s_iCnt2作为2ms计数器

  if(timer_interrupt_flag_get(TIMER2, TIMER_INT_FLAG_UP) == SET)   //判断定时器更新中断是否发生
  {
    timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);         //清除定时器更新中断标志
  }

  s_iCnt2++;            //2ms计数器的计数值加1
//  PSA_Task(1);

  if(s_iCnt2 >= 2)      //2ms计数器的计数值大于或等于2
  {
    s_iCnt2 = 0;        //重置2ms计数器的计数值为0
//    Sensor_Meature();
    s_i2msFlag = TRUE;  //将2ms标志位的值设置为TRUE 
  }
}

/*********************************************************************************************************
* 函数名称：TIMER1_IRQHandler
* 函数功能：TIMER1中断服务函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：每毫秒进入一次中断服务函数
*********************************************************************************************************/
void TIMER1_IRQHandler(void)
{
  static  signed short s_iCnt1000  = 0;                            //定义一个静态变量s_iCnt1000作为1s计数器

  if (timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_UP) == SET) //判断定时器更新中断是否发生
  {
    timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);        //清除定时器更新中断标志 
  }

  s_iCnt1000++;           //1000ms计数器的计数值加1

  if(s_iCnt1000 >= 1000)  //1000ms计数器的计数值大于或等于1000
  {
    s_iCnt1000 = 0;       //重置1000ms计数器的计数值为0
    s_i1secFlag = TRUE;   //将1s标志位的值设置为TRUE
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitTimer
* 函数功能：初始化Timer模块 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：使用的TIMER2和TIMER1由APB1(60MHz)预分频后输出。如果预分频为1，则由APB1*1输出，否则由APB1*2(120MHz)输出
*********************************************************************************************************/
void InitTimer(void)
{
  ConfigTimer2(999, 119);  //120MHz/(119+1)=1MHz，由0计数到999为1ms 
  ConfigTimer1(999, 119);  //120MHz/(119+1)=1MHz，由0计数到999为1ms
  
  ConfigTimer2PWM();
//  ConfigTimer1PWM();
}

/*********************************************************************************************************
* 函数名称：Get2msFlag
* 函数功能：获取2ms标志位的值  
* 输入参数：void
* 输出参数：void
* 返 回 值：s_i2msFlag-2ms标志位的值
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
unsigned char Get2msFlag(void)
{
  return(s_i2msFlag);     //返回2ms标志位的值
}

/*********************************************************************************************************
* 函数名称：Clr2msFlag
* 函数功能：清除2ms标志位  
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void Clr2msFlag(void)
{
  s_i2msFlag = FALSE;     //将2ms标志位的值设置为FALSE 
}

/*********************************************************************************************************
* 函数名称：Get1SecFlag
* 函数功能：获取1s标志位的值  
* 输入参数：void
* 输出参数：void
* 返 回 值：s_i1secFlag-1s标志位的值
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
unsigned char Get1SecFlag(void)
{
  return(s_i1secFlag);    //返回1s标志位的值
}

/*********************************************************************************************************
* 函数名称：Clr1SecFlag
* 函数功能：清除1s标志位  
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void Clr1SecFlag(void)
{
  s_i1secFlag = FALSE;    //将1s标志位的值设置为FALSE
}

/*********************************************************************************************************
* 函数名称：Clr1SecFlag
* 函数功能：清除1s标志位  
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void WriteChannelDuty(EnumPWMName CHName,unsigned int pulse)
{
//  uint32_t periph_list[2]={TIMER1,TIMER2};
  
  if(pulse > 999 || pulse < 0){ // 溢出判断
  
  }
  
  timer_channel_output_pulse_value_config(TIMER2, CHName , pulse);
}
