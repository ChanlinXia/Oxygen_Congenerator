/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "SWITCH.h"
#include "gd32f30x_conf.h"
//#include <stdio.h>
#include "BT_HW.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/


/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static unsigned short  s_iCnt=0;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void SwitchGPIOConfig(void);
static void SwitchTimerConfig(unsigned short arr, unsigned short psc);
static void SwitchEXTIConfig(void);

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
static void SwitchGPIOConfig(void)
{

}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void SwitchEXTIConfig(void)
{

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
//void TIMER3_IRQHandler(void)
//{
//  
//  if (timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP) == SET) //判断定时器更新中断是否发生
//  {
//    timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);        //清除定时器更新中断标志 
////    BTWriteDATA("dd");
//  }
//  
//  s_iCnt++;
////  printf("Cnt=%d ",s_iCnt);
//  if(s_iCnt >= 2000){
//    s_iCnt=0;
//    printf("关机");
//    timer_counter_value_config(TIMER3,0);
//    timer_disable(TIMER3);
//    SysSwitch(ENUM_SWITCH_OFF);
//  }
//  
//}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
//void EXTI5_9_IRQHandler(void)
//{
//  static uint8_t s_EXTICnt=0;
//  if(exti_interrupt_flag_get(EXTI_7) != RESET){//检查中断标志位
//    s_EXTICnt++;
//    if(s_EXTICnt < 2){  // 按下
//      printf("KEY_ON");
//      exti_init(EXTI_7,EXTI_INTERRUPT,EXTI_TRIG_FALLING);
//      timer_counter_value_config(TIMER3,0);
//      timer_enable(TIMER3); // 打开Timer3
//    }
//    else{ //  松开
//      printf("KEY_OFF");
//      s_EXTICnt = 0;
//      s_iCnt=0;
//      exti_init(EXTI_7,EXTI_INTERRUPT,EXTI_TRIG_RISING);
//      timer_counter_value_config(TIMER3,0);
//      timer_disable(TIMER3); // 关闭Timer3
//      //  返回键相应
//      
//    }
//    exti_interrupt_flag_clear(EXTI_7);
//  }
//  
//  
//}

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
void InitSwitch(void)
{
//  SwitchGPIOConfig();
//  SwitchEXTIConfig();
//  SwitchTimerConfig(999, 119);
  SysSwitch(ENUM_SWITCH_ON);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void SysSwitch(EnumSysSwitch sys_switch)
{
  switch(sys_switch){
    case ENUM_SWITCH_ON:
      gpio_bit_set(GPIOC,GPIO_PIN_6);
      break;
    case ENUM_SWITCH_OFF:
      gpio_bit_reset(GPIOC,GPIO_PIN_6);
      //***还需要LCD提示松开按键
        
      break;
    default:
      break;
  }
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
