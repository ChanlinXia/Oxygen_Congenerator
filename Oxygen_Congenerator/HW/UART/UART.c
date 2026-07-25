/*********************************************************************************************************
* 模块名称：UART0.c
* 摘    要：串口模块，包括串口模块初始化，以及中断服务函数处理，以及读写串口函数实现
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
#include "UART.h"
#include "gd32f30x_conf.h"
#include "Queue.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
// 分别配置UART023
static StructCirQue  s_structUART0RecCirQue;       //接收串口循环队列
static unsigned char s_arrRecBuf0[UART_BUF_SIZE]; //接收串口循环队列的缓冲区

static StructCirQue  s_structUART2RecCirQue;       //接收串口循环队列
static unsigned char s_arrRecBuf2[UART_BUF_SIZE]; //接收串口循环队列的缓冲区

static StructCirQue  s_structUART3RecCirQue;       //接收串口循环队列
static unsigned char s_arrRecBuf3[UART_BUF_SIZE]; //接收串口循环队列的缓冲区

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigUART0(unsigned int bound); //配置串口相关的参数，包括GPIO、RCU、USART和NVIC 
static void ConfigUART2(unsigned int bound); //配置串口相关的参数，包括GPIO、RCU、USART和NVIC 
static void ConfigUART3(unsigned int bound); //配置串口相关的参数，包括GPIO、RCU、USART和NVIC 

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：ConfigUART0
* 函数功能：配置串口相关的参数，包括GPIO、RCU、USART和NVIC  
* 输入参数：bound，波特率
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigUART0(unsigned int bound)
{
  //使能GPIO时钟和复用时钟
  rcu_periph_clock_enable(RCU_GPIOA); //使能GPIOA时钟
  rcu_periph_clock_enable(RCU_AF);    //使能复用时钟

  //配置TX的GPIO
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

  //配置RX的GPIO
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

  //配置USART的参数
  rcu_periph_clock_enable(RCU_USART0);                  //使能串口时钟
  usart_deinit(USART0);                                 //RCU配置恢复默认值
  usart_baudrate_set(USART0, bound);                    //设置波特率
  usart_stop_bit_set(USART0, USART_STB_1BIT);           //设置停止位
  usart_word_length_set(USART0, USART_WL_8BIT);         //设置数据字长度
  usart_parity_config(USART0, USART_PM_NONE);           //设置奇偶校验位
  usart_receive_config(USART0, USART_RECEIVE_ENABLE);   //使能接收
  usart_transmit_config(USART0, USART_TRANSMIT_ENABLE); //使能发送
  usart_interrupt_enable(USART0, USART_INT_RBNE);       //使能接收缓冲区非空中断

  //配置NVIC，并设置中断优先级
  nvic_irq_enable(USART0_IRQn, 0, 0);

  //使能串口
  usart_enable(USART0);
}

/*********************************************************************************************************
* 函数名称：ConfigUART2  
* 函数功能：配置串口相关的参数，包括GPIO、RCU、USART和NVIC  
* 输入参数：bound，波特率
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigUART2(unsigned int bound)
{
  //使能GPIO时钟和复用时钟
  rcu_periph_clock_enable(RCU_GPIOB); //使能GPIOA时钟
  rcu_periph_clock_enable(RCU_AF);    //使能复用时钟

  //配置TX的GPIO
  gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

  //配置RX的GPIO
  gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

  //配置USART的参数
  rcu_periph_clock_enable(RCU_USART2);                  //使能串口时钟
  usart_deinit(USART2);                                 //RCU配置恢复默认值
  usart_baudrate_set(USART2, bound);                    //设置波特率
  usart_stop_bit_set(USART2, USART_STB_1BIT);           //设置停止位
  usart_word_length_set(USART2, USART_WL_8BIT);         //设置数据字长度
  usart_parity_config(USART2, USART_PM_NONE);           //设置奇偶校验位
  usart_receive_config(USART2, USART_RECEIVE_ENABLE);   //使能接收
  usart_transmit_config(USART2, USART_TRANSMIT_ENABLE); //使能发送
  usart_interrupt_enable(USART2, USART_INT_RBNE);       //使能接收缓冲区非空中断

  //配置NVIC，并设置中断优先级
  nvic_irq_enable(USART2_IRQn, 0, 0);

  //使能串口
  usart_enable(USART2);
}

/*********************************************************************************************************
* 函数名称：ConfigUART2  
* 函数功能：配置串口相关的参数，包括GPIO、RCU、USART和NVIC  
* 输入参数：bound，波特率
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
static void ConfigUART3(unsigned int bound)
{
  //使能GPIO时钟和复用时钟
  rcu_periph_clock_enable(RCU_GPIOC); //使能GPIOA时钟
  rcu_periph_clock_enable(RCU_AF);    //使能复用时钟

  //配置TX的GPIO
  gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

  //配置RX的GPIO
  gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_11);

  //配置USART的参数
  rcu_periph_clock_enable(RCU_UART3);                  //使能串口时钟
  usart_deinit(UART3);                                 //RCU配置恢复默认值
  usart_baudrate_set(UART3, bound);                    //设置波特率
  usart_stop_bit_set(UART3, USART_STB_1BIT);           //设置停止位
  usart_word_length_set(UART3, USART_WL_8BIT);         //设置数据字长度
  usart_parity_config(UART3, USART_PM_NONE);           //设置奇偶校验位
  usart_receive_config(UART3, USART_RECEIVE_ENABLE);   //使能接收
  usart_transmit_config(UART3, USART_TRANSMIT_ENABLE); //使能发送
  usart_interrupt_enable(UART3, USART_INT_RBNE);       //使能接收缓冲区非空中断

  //配置NVIC，并设置中断优先级
  nvic_irq_enable(UART3_IRQn, 0, 0);

  //使能串口
  usart_enable(UART3);
}

/*********************************************************************************************************
* 函数名称：USART0_IRQHandler
* 函数功能：USART0中断服务函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void USART0_IRQHandler(void)
{
  unsigned char uData;

  if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET)  //接收缓冲区非空中断
  {
    usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);          //清除USART0中断挂起
    uData = usart_data_receive(USART0);                               //将USART0接收到的数据保存到uData
    EnQueue(&s_structUART0RecCirQue, &uData, 1);                       //将接收到的数据写入接收缓冲区
  }

  if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_ERR_ORERR) == SET) //溢出错误标志为1
  {
    usart_interrupt_flag_clear(USART0, USART_INT_FLAG_ERR_ORERR);       //清除溢出错误标志
    usart_data_receive(USART0);                                         //读取USART_DATA
  }
}

/*********************************************************************************************************
* 函数名称：USART2_IRQHandler
* 函数功能：USART2中断服务函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void USART2_IRQHandler(void)
{
  DATA_TYPE uData;

  if(usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE) != RESET)  //接收缓冲区非空中断
  {
    usart_interrupt_flag_clear(USART2, USART_INT_FLAG_RBNE);          //清除USART2中断挂起
    uData = usart_data_receive(USART2);                               //将USART2接收到的数据保存到uData
    EnQueue(&s_structUART2RecCirQue, &uData, 1);                       //将接收到的数据写入接收缓冲区
  }

  if(usart_interrupt_flag_get(USART2, USART_INT_FLAG_ERR_ORERR) == SET) //溢出错误标志为1
  {
    usart_interrupt_flag_clear(USART2, USART_INT_FLAG_ERR_ORERR);       //清除溢出错误标志
    usart_data_receive(USART2);                                         //读取USART_DATA
  }
}

/*********************************************************************************************************
* 函数名称：USART2_IRQHandler
* 函数功能：USART2中断服务函数 
* 输入参数：void
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void UART3_IRQHandler(void)
{
  DATA_TYPE uData;

  if(usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE) != RESET)  //接收缓冲区非空中断
  {
    usart_interrupt_flag_clear(UART3, USART_INT_FLAG_RBNE);          //清除USART2中断挂起
    uData = usart_data_receive(UART3);                               //将USART2接收到的数据保存到uData
    EnQueue(&s_structUART3RecCirQue, &uData, 1);                       //将接收到的数据写入接收缓冲区
  }

  if(usart_interrupt_flag_get(UART3, USART_INT_FLAG_ERR_ORERR) == SET) //溢出错误标志为1
  {
    usart_interrupt_flag_clear(UART3, USART_INT_FLAG_ERR_ORERR);       //清除溢出错误标志
    usart_data_receive(UART3);                                         //读取USART_DATA
  }
}

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：InitUART0
* 函数功能：初始化UART0模块
* 输入参数：bound,波特率
* 输出参数：void
* 返 回 值：void
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
void InitUART(unsigned int bound)
{
  //初始化串口接收队列，同时接收缓冲区内所有数据将会被清零
  InitQueue(&s_structUART0RecCirQue, s_arrRecBuf0, UART_BUF_SIZE);
  InitQueue(&s_structUART2RecCirQue, s_arrRecBuf2, UART_BUF_SIZE);
  InitQueue(&s_structUART3RecCirQue, s_arrRecBuf3, UART_BUF_SIZE);

  //配置串口相关的参数，包括GPIO、RCU、USART和NVIC
  ConfigUART0(bound);
  ConfigUART2(bound);
  ConfigUART3(9600);
}

/*********************************************************************************************************
* 函数名称：WriteUART0
* 函数功能：写串口，即写数据到的串口发送缓冲区
* 输入参数：pBuf，要写入数据的首地址，len，期望写入数据的个数
* 输出参数：void
* 返 回 值：成功写入数据的个数，不一定与形参len相等
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
unsigned int WriteUART(unsigned int uart_periph,unsigned char *pBuf, unsigned int len)
{
  unsigned int i;

  //依次发送所有数据
  for(i = 0; i < len; i++)
  {
    //等待上次传输完成
    while(RESET == usart_flag_get(uart_periph, USART_FLAG_TBE)){}

    //发送数据
    usart_data_transmit(uart_periph, pBuf[i]);
  }

  return len;
}

/*********************************************************************************************************
* 函数名称：ReadUART0
* 函数功能：读串口，即读取串口接收缓冲区中的数据
* 输入参数：pBuf，读取的数据存放的首地址，len，期望读取数据的个数
* 输出参数：pBuf，读取的数据存放的首地址
* 返 回 值：成功读取数据的个数，不一定与形参len相等
* 创建日期：2021年07月01日
* 注    意：
*********************************************************************************************************/
unsigned int ReadUART(unsigned int uart_periph,unsigned char *pBuf, unsigned int len)
{
  switch(uart_periph){   //从接收队列中获取数据
    case USART0:
      return DeQueue(&s_structUART0RecCirQue, pBuf, len);
      break;
    case USART2:
      return DeQueue(&s_structUART2RecCirQue, pBuf, len);
      break;
    case UART3:
      return DeQueue(&s_structUART3RecCirQue, pBuf, len);
      break;
    default:
      break;
  }
}

/*********************************************************************************************************
* 函数名称：fputc
* 函数功能：重定向写字符文件函数
* 输入参数：ch：要写入的字符，f：要写入的文件
* 输出参数：void
* 返 回 值：写入成功的字符，返回EOF(-1) 则代表写入失败
* 创建日期：2021年07月01日
* 注    意：1、链接器检查到用户编写了与C库函数相同名字的函数时，优先使用用户函数，这样就可实现重定向
*          2、C语言中printf函数最终会调用fputc函数向一个文件写入字符，这里的文件是抽象的，可以是磁盘里的文件，
*             可以是OLED或LCD等显示屏，也可以是MCU的外设，如UART0
*          3、串口设备是一个全局资源，必须保证资源的互斥访问，所以要先等待串口发送缓冲区为空，然后才能写入数据
*********************************************************************************************************/
int fputc(int ch, FILE *f)
{
  //等待上次传输完成
  while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));

  //发送数据
  usart_data_transmit(USART0, (uint8_t)ch);

  //返回写入成功的字符
  return ch;
}
