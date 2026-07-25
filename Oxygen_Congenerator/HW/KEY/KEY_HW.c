/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "KEY_HW.h"
//#include "LCD.h"
#include "gd32f30x_conf.h"
#include <stdio.h>
#include "Switch.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static void(*s_KEYCBFunList[ENUM_NAME_MAX])(int)={0};       //KEY的回调函数列表
static funarg  s_KEYCBFunArgList[ENUM_NAME_MAX]={0};    //KEY的回调函数参数
static int  s_iCnt=0;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void GPIOConfig(void);
static void EXTIConfig(void);

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*************************************************
*   
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void GPIOConfig(void)
{
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOC);
  
//  gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);    // 重映射恢复PA15 PB3 PB4为IO 
  gpio_init(GPIOB,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_3|GPIO_PIN_4);
  gpio_bit_set(GPIOB,GPIO_PIN_3|GPIO_PIN_4);
//    rcu_periph_clock_enable(RCU_GPIOC);
  
  //  配置POWER-ON和PWR_KEY_IN引脚
  gpio_init(GPIOC,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_6);
  gpio_bit_set(GPIOC,GPIO_PIN_6);   //  开机上拉，保持系统上电
  
  gpio_init(GPIOC,GPIO_MODE_IPD,GPIO_OSPEED_50MHZ,GPIO_PIN_7);
  gpio_bit_reset(GPIOC,GPIO_PIN_7);

}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void EXTIConfig(void)
{
  rcu_periph_clock_enable(RCU_AF); //使能AF时钟
  rcu_periph_clock_enable(RCU_GPIOC);
  
  // 将PB3配置为 EXTI_RISINGTRIG
  gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB,GPIO_PIN_SOURCE_3);   // 连接GPIO与EXTI
  exti_init(EXTI_3,EXTI_INTERRUPT,EXTI_TRIG_RISING);            // 将EXTI设置为上升沿触发
  exti_interrupt_enable(EXTI_3);                                // 打开EXTI3的中断
  nvic_irq_enable(EXTI3_IRQn, 2, 2);                            // 中断使能，设置优先级
  
  // 将PB4配置为 EXTI_FALLINGTRIG                               
  gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB,GPIO_PIN_SOURCE_4);   // 连接GPIO与EXTI
  exti_init(EXTI_4,EXTI_INTERRUPT,EXTI_TRIG_RISING);           // 将EXTI设置为下降沿触发
  exti_interrupt_enable(EXTI_4);                                // 打开EXTI4的中断
  nvic_irq_enable(EXTI4_IRQn, 2, 2);                            // 中断使能，设置优先级
  
  // 将PB3配置为 EXTI_RISINGTRIG
  gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC,GPIO_PIN_SOURCE_7);   // 连接GPIO与EXTI
  exti_init(EXTI_7,EXTI_INTERRUPT,EXTI_TRIG_RISING);            // 将EXTI设置为上升沿触发
  exti_interrupt_enable(EXTI_7);                                // 打开EXTI3的中断
  nvic_irq_enable(EXTI5_9_IRQn, 2, 2);                            // 中断使能，设置优先级
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void SwitchTimerConfig(unsigned short arr, unsigned short psc)
{
  timer_parameter_struct timer_initpara;               //timer_initpara用于存放定时器的参数

  //使能RCU相关时钟 
  rcu_periph_clock_enable(RCU_TIMER3);                 //使能TIMER2的时钟

  //复位TIMER2
  timer_deinit(TIMER3);                                //设置TIMER2参数恢复默认值
  timer_struct_para_init(&timer_initpara);             //初始化timer_initpara

  //配置TIMER2
  timer_initpara.prescaler         = psc;              //设置预分频器值
  timer_initpara.counterdirection  = TIMER_COUNTER_UP; //设置向上计数模式
  timer_initpara.period            = arr;              //设置自动重装载值
  timer_initpara.clockdivision     = TIMER_CKDIV_DIV1; //设置时钟分割
  timer_init(TIMER3, &timer_initpara);                 //根据参数初始化定时器

  //使能定时器及其中断
  timer_interrupt_enable(TIMER3, TIMER_INT_UP);        //使能定时器的更新中断
  nvic_irq_enable(TIMER3_IRQn, 1, 0);                  //配置NVIC设置优先级
  timer_disable(TIMER3);                               //使能定时器
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void KeyHook(int arg)
{
  printf("Just KEY%d Hook Func\r\n",arg+1);
}


/*********************************************************************************************************
*                                              API实现
*********************************************************************************************************/
/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void InitKEYHW(void)
{
  int i=0;
  
  GPIOConfig();
  EXTIConfig();
  SwitchTimerConfig(999, 119);
  
  for(i=0;i<ENUM_NAME_MAX;i++){
    SetKEYCallBack(KeyHook,i,i);
  }
  printf("%d",s_KEYCBFunArgList[ENUM_NAME_PWR]);
  
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void EXTI3_IRQHandler(void)
{
  if(exti_interrupt_flag_get(EXTI_3) != RESET){ //检查中断标志位
    //printf("KEY1 From IRQHandler");
//    LCDKeyProcTest(ENUM_NAME_KEY1);
    s_KEYCBFunList[ENUM_NAME_KEY1](s_KEYCBFunArgList[ENUM_NAME_KEY1]);  //KEY1回调函数
    

    exti_interrupt_flag_clear(EXTI_3);    //清除中断标志位
  }
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void EXTI4_IRQHandler(void)
{
  if(exti_interrupt_flag_get(EXTI_4) != RESET){//检查中断标志位
    s_KEYCBFunList[ENUM_NAME_KEY2](s_KEYCBFunArgList[ENUM_NAME_KEY2]); //KEY2回调函数
    //printf("KEY2 From IRQHandler");
//    LCDKeyProcTest(ENUM_NAME_KEY2);
    exti_interrupt_flag_clear(EXTI_4);    //清除中断标志位
  }
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void TIMER3_IRQHandler(void)
{
  
  if (timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP) == SET) //判断定时器更新中断是否发生
  {
    timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);        //清除定时器更新中断标志 
//    BTWriteDATA("dd");
  }
  
  s_iCnt++;
//  printf("Cnt=%d ",s_iCnt);
  if(s_iCnt >= 2000){
    s_iCnt=0;
    printf("关机");
    timer_counter_value_config(TIMER3,0);
    timer_disable(TIMER3);
    SysSwitch(ENUM_SWITCH_OFF);
  }
  
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void EXTI5_9_IRQHandler(void)
{
  static uint8_t s_EXTICnt=0;
  if(exti_interrupt_flag_get(EXTI_7) != RESET){//检查中断标志位
//    printf("KEY_ON");
    s_EXTICnt++;
    if(s_EXTICnt < 2){  // 按下
//      printf("KEY_ON");
      exti_init(EXTI_7,EXTI_INTERRUPT,EXTI_TRIG_FALLING);
      timer_counter_value_config(TIMER3,0);
      timer_enable(TIMER3); // 打开Timer3
    }
    else{ //  松开
//      printf("KEY_OFF");
      s_EXTICnt = 0;
      s_iCnt=0;
      exti_init(EXTI_7,EXTI_INTERRUPT,EXTI_TRIG_RISING);
      timer_counter_value_config(TIMER3,0);
      timer_disable(TIMER3); // 关闭Timer3
      
      //  返回键相应
//      LCDKeyProcTest(ENUM_NAME_PWR);

      s_KEYCBFunList[ENUM_NAME_PWR](s_KEYCBFunArgList[ENUM_NAME_PWR]);
    }
    exti_interrupt_flag_clear(EXTI_7);
  }
  
  
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void SetKEYCallBack(void (*Func_CB)(int),funarg Arg,EnumKEYName KEY_Name)
{
  s_KEYCBFunList[KEY_Name] = Func_CB;  // 设置对应KEY的回调函数
  s_KEYCBFunArgList[KEY_Name] = Arg;   // 输入参数
}

