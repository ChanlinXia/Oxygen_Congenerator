/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "BT_HW.h"
#include "gd32f30x_conf.h"
#include "UART.h"
#include "systick.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define BT_CDS_BIT(x) gpio_bit_write(GPIOB,GPIO_PIN_13,x)
#define BT_RTS_BIT(x) gpio_bit_write(GPIOB,GPIO_PIN_14,x)

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigGPIO(void);  //配置蓝牙的GPIO
static unsigned int BTSendByte(unsigned char byte);   //  单字节发送
static unsigned int BTSendBytes(unsigned char* bytes);//  字符串发送

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*************************************************
*   配置蓝牙的引脚
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void ConfigGPIO(void)
{
  rcu_periph_clock_enable(RCU_GPIOB);
  
  gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_13|GPIO_PIN_14);  // CDS与RST
  gpio_init(GPIOB,GPIO_MODE_IPD,GPIO_OSPEED_50MHZ,GPIO_PIN_15);   //  Link标志位
  gpio_bit_set(GPIOB,GPIO_PIN_13|GPIO_PIN_14);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static unsigned int BTSendByte(unsigned char byte)
{
  return WriteUART(USART2,&byte,1);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static unsigned int BTSendBytes(unsigned char* bytes)
{
  int i=0;
  
  while(1){
    if(bytes[i] == '\0'){
      i++;
      break;
    }
    BTSendByte(bytes[i]);
//    printf("%c",bytes[i]);
    i++;
  }
  
  return i;
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
void InitBTHW(void)
{
  ConfigGPIO();
  
  //  复位一次蓝牙模块
  BTReset();

}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
unsigned int BTWriteCMD(unsigned char* bytes)
{
  BT_CDS_BIT(RESET);
  
  return BTSendBytes(bytes);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
unsigned int BTWriteDATA(unsigned char* bytes)
{
  BT_CDS_BIT(SET);
  
  return BTSendBytes(bytes);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
unsigned int BTReadDATA(unsigned char* pbuf)
{
  int i=0;
  
  while(1){
    if(0 == ReadUART(USART2,pbuf+i,1)){ // 队列为空时返回0
      return i;
    }
    i++;
    
  }
//  return i;
}

/*************************************************
*   检测蓝牙是否连接
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
unsigned char BTLinkFlag(void)
{
  return (unsigned char)gpio_input_bit_get(GPIOB,GPIO_PIN_15);
}

/*************************************************
*   蓝牙复位
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void BTReset(void)
{
  BT_RTS_BIT(RESET);
  DelayNms(20);
  BT_RTS_BIT(SET);
}
