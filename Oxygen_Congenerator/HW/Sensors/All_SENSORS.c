/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "All_SENSORS.h"
#include "I2C.h"
#include "systick.h"
#include "O2_SENSOR.h"
#include "gd32f30x_conf.h"
#include <stdio.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
typedef enum
{
  ENUM_SENSOR_STATUS_MEATUREING=0,
  ENUM_SENSOR_STATUS_IDLE,
  ENUM_MAX
}EnumSensorWorkStatus;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static uint8_t s_arrRawDataBuffer[50]={0};
static double s_arrSensorVal[ENUM_SENSOR_MAX]={0};
static uint8_t s_iSensorStatus=ENUM_SENSOR_STATUS_MEATUREING;
static uint8_t s_iFlowFlag=0;

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
// 测量的I2C时序
static void BoardTempMeature(unsigned short cnt);
static void O2Meature(unsigned short cnt);
static void BumpTempMeature(unsigned short cnt);
static void PressureMeature(unsigned short cnt);
static void WaveMeature(unsigned short cnt);

// 基于各传感器返回数据得到测量值
static void CalcBoardTempVal(void);
static void CalcO2_FlowVal(void);
static void CalcWaveVal(void);
static void CalcPressureVal(void);
static void CalcBumpTempVal(void);

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*************************************************
*   生成一次温度测量序列，读取测量温度结果
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void BoardTempMeature(unsigned short cnt)
{
  uint8_t i=0;
  static unsigned short s_iCnt=0;
  
  s_iCnt++;
  if(s_iCnt < cnt){
    return;
  }
  s_iCnt=0;
  
  I2C_SetCSFlag(ENUM_I2C_BOARDTEMP);  // 片选为温湿度传感器
  
  I2C_Start();        // 开始信号
  I2C_SendHeader(0);  // 发送写入数据头
  while(I2C_ReceiveACK() == 1){printf("while1");}  // 等待ACK
    
    // clock stretching
  // 写入两字节的 meaturement command
  I2C_WriteByte(0x2C);
  while(I2C_ReceiveACK() == 1){}
  I2C_WriteByte(0x0D);        
  while(I2C_ReceiveACK() == 1){}
  I2C_Stop();
  DelayNms(50);
    
  // 开始读取
  I2C_Start();
  I2C_SendHeader(1);  // 读取数据头
  while(I2C_ReceiveACK() == 1){}
  DelayNms(20);   // 等待测量完毕
//    printf("temp  ");
  for(i=0;i<6;i++){
    s_arrRawDataBuffer[i]=I2C_ReadByte();
//    printf("%x ",s_arrRawDataBuffer[i]);  
    I2C_SendACK(i/5); // 发送ACK
  }
  
    I2C_Stop();
  
  // 计算出此时的温度
  CalcBoardTempVal();
  printf("[[1,%2f]]\r\n",s_arrSensorVal[ENUM_SENSOR_BOARDTEMP]);

}

/*************************************************
*   读取一次O2浓度传感器的串口内容
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void O2Meature(unsigned short cnt)
{
  unsigned char num=0,i=0;
  
  static unsigned short s_iCnt=0;
  
  s_iCnt++;
  if(s_iCnt < cnt){
    return;
  }
  s_iCnt=0;

  num=ReadO2Sensor(s_arrRawDataBuffer);
//  
//  printf("num=%d\r\n",num);
//  for(i=0;i<num;i++){
//    printf("%x ",s_arrRawDataBuffer[i]);
//  }
////  printf("\r\n");
  
  //  计算出此时的O2浓度和流量
  CalcO2_FlowVal();
  printf("[[2,%2f]]\r\n",s_arrSensorVal[ENUM_SENSOR_O2]);
  printf("[[3,%2f]]\r\n",s_arrSensorVal[ENUM_SENSOR_FLOW]);  
}

/*************************************************
*   生成一次ABP的测量时序
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void WaveMeature(unsigned short cnt)
{
  uint8_t i=0;
  static unsigned short s_iCnt=0;
  
  s_iCnt++;
  if(s_iCnt < cnt){
    return;
  }
  s_iCnt=0;
  
  
  I2C_SetCSFlag(ENUM_I2C_ABP);
  
  //  发出开始信号与写入数据头
  I2C_Start();
  I2C_SendHeader(0);
  while(I2C_ReceiveACK() == 1){printf("Wave");} //  等待ACK
  
  //  写入meature command
  I2C_WriteByte(0xAA);
  while(I2C_ReceiveACK() == 1){}
  I2C_WriteByte(0x00);
  while(I2C_ReceiveACK() == 1){}
  I2C_WriteByte(0x00);
  while(I2C_ReceiveACK() == 1){}
  I2C_Stop();
  while(I2C_GetEOC() == 0){}  // 等待测量完毕
  
  //  开始读取
  I2C_Start();
  I2C_SendHeader(1);  // 读取数据头
  while(I2C_ReceiveACK() == 1){}
//  DelayNms(20);   // 等待测量完毕
//  printf("wave ");
  for(i=0;i<7;i++){
    s_arrRawDataBuffer[i]=I2C_ReadByte();
//    printf("%x ",s_arrRawDataBuffer[i]);  
    I2C_SendACK(i/6); // 发送ACK
  }
//  printf("\r\n");
  I2C_Stop();

  CalcWaveVal();
  

  printf("Wave=%f\r\n",s_arrSensorVal[ENUM_SENSOR_WAVE]);
}

/*************************************************
*   生成一次SSC的测量时序
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void PressureMeature(unsigned short cnt)
{
  uint8_t i=0;
  static unsigned short s_iCnt=0;
  
  s_iCnt++;
  if(s_iCnt < cnt){
    return;
  }
  s_iCnt=0;
  
  //  片选SSC传感器
  I2C_SetCSFlag(ENUM_I2C_SSC);
  
  //  开始读取
  I2C_Start();
  I2C_SendHeader(1);  // 读取数据头
  while(I2C_ReceiveACK() == 1){printf("pressure ");}
//  printf("pressure ");
  
  for(i=0;i<4;i++){
    s_arrRawDataBuffer[i]=I2C_ReadByte();
//    printf("%x ",s_arrRawDataBuffer[i]);  
    I2C_SendACK(i/3); // 发送ACK
  }
  I2C_Stop();
  CalcPressureVal();
//  printf("\r\n");

  printf("[[4,%2f]]\r\n",s_arrSensorVal[ENUM_SENSOR_PRESSURE]);  
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void BumpTempMeature(unsigned short cnt)
{
  uint8_t i=0;
  static unsigned short s_iCnt=0;
  
  s_iCnt++;
  if(s_iCnt < cnt){
    return;
  }
  s_iCnt=0;
  
  
  //  片选到测压缩机温度传感器
  I2C_SetCSFlag(ENUM_I2C_BUMPTEMP);
  
  //  该传感器的开始信号：start+stop信号组合
  I2C_Start();
  I2C_Stop();
  I2C_WriteByte(0x03);  //  温度测量指令
  while(I2C_ReceiveACK() == 1){/*printf("temp wait ACK");*/}
  while(I2C_ReadSDA() == 1){/*printf("temp wait response");*/}  //  等待测量完毕
    
//  printf("bump temp");
  for(i=0;i<3;i++){
    s_arrRawDataBuffer[i]=I2C_ReadByte();
//    printf("%x  ",s_arrRawDataBuffer[i]);
    I2C_SendACK(i/2);
  }
//  printf("\r\n");
  CalcBumpTempVal();
  I2C_Stop();

  printf("[[5,%2f]]\r\n",s_arrSensorVal[ENUM_SENSOR_BUMPTEMP]);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void CalcBoardTempVal(void)
{
  unsigned short rawData=0;
  
  

  rawData = (s_arrRawDataBuffer[0]) << 8;
  rawData|= s_arrRawDataBuffer[1];
  
  // formula
  s_arrSensorVal[ENUM_SENSOR_BOARDTEMP]=-45.0+175.0*(rawData/65535.0);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void CalcO2_FlowVal(void)
{
  // 解包,算出O2浓度和流量
  int i=0;
  double temp1=0,temp2=0;;
  
  while(1){
    if(s_arrRawDataBuffer[i] == 0x16){  //  读取到数据头时，再计算
      temp1=s_arrRawDataBuffer[i+3];temp2=s_arrRawDataBuffer[i+4];
      temp1*=256;temp1+=temp2;
      temp1/=10.0;s_arrSensorVal[ENUM_SENSOR_O2]=temp1;
      
      if(s_iFlowFlag == 0){
        break;
      }
      temp1=s_arrRawDataBuffer[i+5];temp2=s_arrRawDataBuffer[i+6];
      temp1*=256;temp1+=temp2;
      temp1/=10.0;s_arrSensorVal[ENUM_SENSOR_FLOW]=temp1;
      
      break;
    }
    i++;
  }
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void CalcWaveVal(void)
{
  uint32_t rawData=0;   // 传感器的初值
  double pressure=0.0,pmax=498.16,pmin=-498.16,outmin=5033164.0,outmax=11744051.0;
  
  
//  pmax=0.036;pmin=-0.036;
  //  读出原始数据  24bit
  rawData = s_arrRawDataBuffer[1];
  rawData = (rawData << 8)|s_arrRawDataBuffer[2];
  rawData = (rawData << 8)|s_arrRawDataBuffer[3];
//  printf("raw=%d",rawData);
  
  
  pressure = (rawData - outmin)*(pmax-pmin);
//  printf("press=%f",pressure);
  pressure /=(outmax-outmin);
//    printf("press=%f",pressure);
  pressure +=pmin-9;
  
//  pressure *= 249.1;   // 计算压力，单位换算
  s_arrSensorVal[ENUM_SENSOR_WAVE] = pressure;
//  printf("press=%f",pressure);

}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void CalcPressureVal(void)
{
  uint32_t rawData=0;
  double pressure=0.0,outmin=1638,pmax=206842.77,pmin=0,outmax=14745;
  
  
  //  读出原始数据  14bit
  rawData = s_arrRawDataBuffer[0];
  if(rawData >= 0X40){
    return;
  }
  rawData = (rawData << 8)|s_arrRawDataBuffer[1];
//  printf("raw=%d",rawData);
  pressure = ((rawData - outmin)*(pmax-pmin))/(outmax-outmin)+pmin;   // 计算压力
  s_arrSensorVal[ENUM_SENSOR_PRESSURE] = pressure;
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void CalcBumpTempVal(void)
{
  double d1=-39.6,d2=0.01;
  short rawdata=0;
  
  rawdata = s_arrRawDataBuffer[0] & 0x3F;
  rawdata = (rawdata<<8)|s_arrRawDataBuffer[1];
  
  s_arrSensorVal[ENUM_SENSOR_BUMPTEMP]=d1+d2*rawdata;
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
void InitAllSensors(void)
{
  
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void Sensors_Update(unsigned int cnt)
{
  static uint32_t s_iCnt=0;
  
  s_iCnt++;
  if(s_iCnt <= cnt){
    return;
  }
  s_iCnt=0;
  

  
//  O2Meature(50);

//  BumpTempMeature(1250);  //  重映射JLINK接口后才可使用
//  
  // 数据展示
//      printf("[[1,%d]]\r\n",RVal);

//  printf("[[1,%f]]\r\n",s_arrSensorVal[ENUM_SENSOR_BOARDTEMP]);
//  printf("[[2,%f]]\r\n",s_arrSensorVal[ENUM_SENSOR_O2]);
//  printf("[[3,%f]]\r\n",s_arrSensorVal[ENUM_SENSOR_FLOW]);
//  
////  gpio_bit_write(GPIOC,GPIO_PIN_4,(1-gpio_output_bit_get(GPIOC, GPIO_PIN_4)));
//  printf("[[4,%f]]\r\n",s_arrSensorVal[ENUM_SENSOR_PRESSURE]);
////  printf("[[5,%f]]\r\n",s_arrSensorVal[ENUM_SENSOR_BUMPTEMP]);
          
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
double Sensors_GetVal(EnumSensorVal sensor)
{
  return s_arrSensorVal[sensor];
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void Senosrs_SwitchStatus(void)
{
  //  在两种状态中切换
  s_iSensorStatus = 1-s_iSensorStatus;
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void Sensor_Meature(void)
{
//  if(s_iSensorStatus == ENUM_SENSOR_STATUS_MEATUREING){ // 制氧时不测呼吸波形
//      WaveMeature(2);
//  }
//  
  WaveMeature(2);
  O2Meature(500);
  PressureMeature(250);
  BoardTempMeature(1000);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void MasterSendO2Cmd(void)
{
  unsigned char  command[]={0x11,0x01,0x01,0xed},num=0,i=0;
 

}  

void MasterGetData(void)
{
  uint8_t num=0,i=0;
   unsigned char data[50]={0};
  s_iFlowFlag=1;
  DelayNus(200); //  等待发回数据
  num=ReadO2Sensor(data);
  printf("master here: num=%d\r\n",num);
//  for(i=0;i<num;i++){
//    printf("%x  ",data[i]);
//  }
//  printf("\r\n"); 
  
  CalcO2_FlowVal();
 printf("[[3,%2f]]\r\n",s_arrSensorVal[ENUM_SENSOR_FLOW]);  
}