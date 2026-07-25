/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "PSA_HW.h"
#include "Timer.h"
#include "gd32f30x_conf.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static double s_arrPWMDuty[ENUM_PWM_MAX]={0};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void PSAGPIOConfig(void);

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
static void PSAGPIOConfig(void)
{
  rcu_periph_clock_enable(RCU_GPIOA);
  
  gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,BITS(0,3));
  gpio_bit_reset(GPIOA,BITS(0,3));
  gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_6);
  gpio_bit_reset(GPIOA,GPIO_PIN_6);

}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/

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
void InitPSAHW(void)
{
  PSAGPIOConfig();
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void PSA_PinCtr(EnumPSAPin pin,uint8_t bit_status)
{
  switch(pin){
    case ENUM_PSA_PIN_SYS1:
      gpio_bit_write(GPIOA,GPIO_PIN_2,bit_status);
      break;
    case ENUM_PSA_PIN_SYS2:
      gpio_bit_write(GPIOA,GPIO_PIN_3,bit_status);
      break;
    case ENUM_PSA_PIN_FAN :
      gpio_bit_write(GPIOA,GPIO_PIN_1,bit_status);
      break;
    case ENUM_PSA_PIN_AVER:
      gpio_bit_write(GPIOA,GPIO_PIN_0,bit_status);
      break;
    case ENUM_PSA_PIN_O2:
      gpio_bit_write(GPIOA,GPIO_PIN_6,bit_status);
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
void PSA_PWMCtr(EnumPWMName element,double duty)
{
  int pulse = (int)((1-duty)*999);
  
  if(element < 0 || element >=ENUM_PWM_MAX){
    return;
  }
  s_arrPWMDuty[element]=duty;
  WriteChannelDuty(element,pulse);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
double PSA_PWMDutyGet(EnumPWMName element)
{
  return s_arrPWMDuty[element];
}
