/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "I2C.h"
#include "gd32f30x_conf.h"
#include "systick.h"
#include <stdio.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define DELAY_CNT 200

//#define WRITE_SDA1(periph,pin,status) gpio_bit_write(GPIOB,GPIO_PIN_5,x);DelayNus(DELAY_CNT)
//#define WRITE_SCL1(x) gpio_bit_write(GPIOB,GPIO_PIN_6,x);DelayNus(DELAY_CNT)
//#define WRITE_SDA2(x) gpio_bit_write(GPIOB,GPIO_PIN_7,x);DelayNus(DELAY_CNT)
//#define WRITE_SCL2(x) gpio_bit_write(GPIOB,GPIO_PIN_8,x);DelayNus(DELAY_CNT)

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static uint32_t s_CurSDAPort=0;
static uint32_t s_CurSDAPin =0;
static uint32_t s_CurSCLPort=0;
static uint32_t s_CurSCLPin =0;
static uint8_t  s_CurAddr=0;

static uint32_t s_AllSDAPort[ENUM_I2C_MAX]={GPIOB,GPIOB,GPIOC,GPIOA}; // 记录所有I2C的SDA的端口
static uint32_t s_AllSDAPin[ENUM_I2C_MAX]={GPIO_PIN_5,GPIO_PIN_7,GPIO_PIN_9,GPIO_PIN_13};
static uint32_t s_AllSCLPort[ENUM_I2C_MAX]={GPIOB,GPIOB,GPIOC,GPIOA};
static uint32_t s_AllSCLPin[ENUM_I2C_MAX]={GPIO_PIN_6,GPIO_PIN_8,GPIO_PIN_8,GPIO_PIN_14};
static uint8_t  s_ALLDeviceAddr[ENUM_I2C_MAX]={0x28,0x28,0x44,0X000};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void I2CConfigGPIO(void);

static void WRITE_SDA(bit_status status);
static void WRITE_SCL(bit_status status);
static uint8_t READ_SDA(void);  //  用于读取ACK信号

static void I2CStart(void);     //  I2C开始信号
static void I2CStop(void);      //  I2C结束信号
static void I2CSendByte(uint8_t byte);  //  I2C操控SDA发送一字节
static uint8_t I2CReceiveByte(void);    //  I2C读取SDA收取一字节

static void I2CSendACK(uint8_t ack);    //  发送ACK/UACK信号
static uint8_t I2CReceiveACK(void);     //  收取ACK
static uint8_t I2CReadByte(void); //  生成一次I2C接受时序

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
static void I2CConfigGPIO(void)
{
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_GPIOC);


  //  两个流量计
  gpio_init(GPIOB, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, BITS(5,8)); // 因为是I2C协议，故需用开漏工作模式
  gpio_bit_set(GPIOB,BITS(5,8));
  gpio_init(GPIOB,GPIO_MODE_IPD,GPIO_OSPEED_50MHZ,GPIO_PIN_9);  //  EOC位，高电平时代表一次测量结束，可通过I2C读取一次数值
  
  //  温湿度传感器
  gpio_init(GPIOC,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_9|GPIO_PIN_8);
  gpio_init(GPIOC,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_13);      // RST引脚
  gpio_bit_set(GPIOC,GPIO_PIN_9|GPIO_PIN_8|GPIO_PIN_13);
  gpio_init(GPIOC,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_12); //  ALERT引脚
}

/*************************************************
*   
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void TempGPIOConfig(void)
{
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_AF);
  
    // 压缩机温度传感器，开启后无法使用LINK
  gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP,ENABLE);
//    AFIO_PCF0 &= 0XFFFFFFFC;
  gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_14);
  gpio_init(GPIOA,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_13);
  gpio_bit_reset(GPIOA,GPIO_PIN_13|GPIO_PIN_14);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void WRITE_SDA(bit_status status)
{
  gpio_bit_write(s_CurSDAPort,s_CurSDAPin,status);
  DelayNus(DELAY_CNT);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void WRITE_SCL(bit_status status)
{
  gpio_bit_write(s_CurSCLPort,s_CurSCLPin,status);
  DelayNus(DELAY_CNT);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static uint8_t READ_SDA(void)
{
  return (uint8_t)gpio_input_bit_get(s_CurSDAPort,s_CurSDAPin);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void I2CStart(void)
{
  //  在SCL高电平时检测到下降沿，即为开始
  WRITE_SDA(SET);
  WRITE_SCL(SET);
//  DelayNms(5);
  WRITE_SDA(RESET);
  WRITE_SCL(RESET);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void I2CStop(void)
{
  //  在SCL高电平时检测到上升沿，即为结束
  WRITE_SDA(RESET);
  WRITE_SCL(SET);
  WRITE_SDA(SET);
  WRITE_SCL(RESET);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void I2CSendByte(uint8_t byte)
{
  int i=0;
  
  for(i = 0;i < 8;i++){
    if((byte & (0x80>>i)) == 0){ // 高位先行
      WRITE_SDA(RESET);  
//      printf("0");
    }
    else{
      WRITE_SDA(SET);
//        printf("1");
    }
//    printf("  ");
    
    WRITE_SCL(SET);
    WRITE_SCL(RESET);
  }
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static uint8_t I2CReceiveByte(void)
{
  uint8_t i=0,byte=0;
  
  WRITE_SDA(SET); 
  for(i=0;i<8;i++){
    WRITE_SCL(SET);
    byte|=READ_SDA() << (7-i); // 数据赋位
    WRITE_SCL(RESET);
  }
  
  return byte;
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void I2CSendACK(uint8_t ack)
{
  WRITE_SDA(ack & 0x01);
  WRITE_SCL(SET);
  WRITE_SCL(RESET);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static uint8_t I2CReceiveACK(void)
{
  uint8_t ack_bit=0;
  WRITE_SDA(SET); // 释放SDA
  WRITE_SCL(SET);
//  DelayNms(5);
  ack_bit = READ_SDA(); // 读取ACK信号，数据正确时，设备拉低信号
  WRITE_SCL(RESET);
  
  return ack_bit; // 0:1 ? ACK:UACK
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static uint8_t I2CReadByte(void)
{
  return I2CReceiveByte();
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
void InitI2C(void)
{
  I2CConfigGPIO();
  
  I2C_SetCSFlag(ENUM_I2C_BOARDTEMP);  //  默认片选
//  DelayNms(11);
}

/*************************************************
*   选用I2C设备
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void I2C_SetCSFlag(EnumCSI2C I2C_NAME)
{
  //  更新device信息
  s_CurSDAPort = s_AllSDAPort[I2C_NAME];
  s_CurSDAPin  = s_AllSDAPin[I2C_NAME];
  s_CurSCLPort = s_AllSCLPort[I2C_NAME];
  s_CurSCLPin  = s_AllSCLPin[I2C_NAME];
  s_CurAddr    = s_ALLDeviceAddr[I2C_NAME];  
  
  // 初始化电平
  WRITE_SCL(RESET);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void I2C_Start(void)
{
  I2CStart();
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void I2C_Stop(void)
{
  I2CStop();
}  

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void I2C_SendHeader(uint8_t w_or_r)
{
  uint8_t header=(s_CurAddr<<1)+w_or_r;   // header=addr+w/r_bit
//  printf("%x ",header);
  I2CSendByte(header);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void I2C_SendACK(uint8_t a_or_u)
{
  I2CSendACK(a_or_u);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
uint8_t I2C_ReceiveACK(void)
{
  return I2CReceiveACK();
}  

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void I2C_WriteByte(uint8_t byte)
{
  I2CSendByte(byte);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
uint8_t I2C_ReadByte(void)
{
  return I2CReadByte();
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
uint8_t I2C_GetEOC(void)
{
  return (uint8_t)gpio_input_bit_get(GPIOB,GPIO_PIN_9);
}  

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
uint8_t I2C_ReadSDA(void)
{
  return READ_SDA();
}
