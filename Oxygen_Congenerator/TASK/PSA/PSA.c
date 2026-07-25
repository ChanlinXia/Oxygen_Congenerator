/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "PSA.h"
//#include "PWM.h"
#include "PSA_HW.h"
#include "LCD.h"
#include "ALL_SENSORS.h"
#include "KEY_HW.h"
#include "DataBuffer.h"
#include <stdio.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define PSA_MODE_CONTINUOUS 0
#define PSA_MODE_PULSE 1

//  档位系统阀持续时间 (ms)
#define PSA_LEVEL5_SYSVALV_HIGH 3060
#define PSA_LEVEL5_SYSVALV_LOW  3760  

#define PSA_LEVEL4_SYSVALV_HIGH 2860
#define PSA_LEVEL4_SYSVALV_LOW  3500

//#define PSA_LEVEL5_SYSVALV_EAGE (PSA_LEVEL5_SYSVALV_LOW-PSA_LEVEL5_SYSVALV_HIGH)/2

//  均压阀开启时间
#define PSA_AVERVALV_OPEN_CNT_5     50
#define PSA_VALVSWITCH_EADGE_CNT_5  350
#define PSA_AVERVALVE_EADGE_CNT_5    (PSA_VALVSWITCH_EADGE_CNT_5-PSA_AVERVALV_OPEN_CNT_5)/2

#define PSA_AVERVALV_OPEN_CNT_4     35
#define PSA_VALVSWITCH_EADGE_CNT_4  320
#define PSA_AVERVALVE_EADGE_CNT_4    (PSA_VALVSWITCH_EADGE_CNT_4-PSA_AVERVALV_OPEN_CNT_4)/2

#define WAVE_BUFFER_LEN 1000  //  至少要采集到两个周期以上的数据

typedef enum{
  //  连续制氧模式状态
  ENUM_PSA_STATUS_IDLE=0,
  ENUM_PSA_STATUS_PRESSUP,
  ENUM_PSA_STATUS_VALVESWITCH,
  ENUM_PSA_STATUS_PRESSAVER,
  ENUM_PSA_STATUS_PRESSSTAY,
  
  //  脉冲制氧状态
  ENUM_PSA_STATUS_CALCTIME,   //  计算供氧时间
  ENUM_PSA_STATUS_MEATURE,    //  检测供氧点，打开了压差计
  ENUM_PSA_STATUS_SUPPLYING,  //  关闭关闭压差计读数
  ENUM_PSA_STATUS_MAX
}EnumPSAStatus;

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static unsigned char s_iPSAMode=PSA_MODE_CONTINUOUS;
static unsigned char s_iPSAStatus=0;
static unsigned char s_iPulseStatus=ENUM_PSA_STATUS_CALCTIME;
static unsigned char s_iLevelFlag=0;
static unsigned short s_iHighCntNum=0,s_iLowCntNum=0;
static unsigned short s_iAverOpenCntNum=0;
static unsigned short s_iValvSwitchEadgeCntNum=0,s_iAverValveEadgeCntNum;

static BUFFER_TYPE s_arrWave[WAVE_BUFFER_LEN]={0};

static StructDataBuffer s_WaveBuffer;


/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void SwitchValve(void);

static void PSAContinuous(void);
static void PSAPulseFlow(void);
static void PSAStart(void);
static void PSAAbort(void);
//static void PSATest(void);
static void AverValveSwitch(void);  // 均压阀开关
static void SetLevel(void);

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
//static void Switch_YASUOJI(void)
//{
//  static int i=0;
//  if(i<1){
//    SetPWMDuty(ENUM_COM_PWM,0);
//      i++;

//  }
//  else{
//    SetPWMDuty(ENUM_COM_PWM,0.5);
//        i=0;
//  }
//}

/*************************************************
*   
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void AverValveSwitch(void)
{
  static unsigned char s_iFlag=0;
  
  //  第一次打开阀，第二次关闭阀
  if(s_iFlag == 0){ //  背面阀->正面阀
    PSA_PinCtr(ENUM_PSA_PIN_AVER,1);
  }
  else{//  正面阀->背面阀
    PSA_PinCtr(ENUM_PSA_PIN_AVER,0);
  }
  s_iFlag =1-s_iFlag;
}

/*************************************************
*   
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void SwitchValve(void)
{
  static unsigned char s_iFlag=0;
  static  unsigned short s_iCnt=0;
    
//      PSA_PinCtr(ENUM_PSA_PIN_SYS1,s_iFlag);
  
  
  //  先关阀
  if(s_iFlag == 0){ //  背面阀->正面阀
    PSA_PinCtr(ENUM_PSA_PIN_SYS1,0);
  }
  else{//  正面阀->背面阀
    PSA_PinCtr(ENUM_PSA_PIN_SYS2,0);
  }
  
  //  进行计时
  s_iCnt++;
  
  //  中途开关均压阀
  if(s_iCnt == s_iAverValveEadgeCntNum || 
     s_iCnt ==s_iValvSwitchEadgeCntNum-s_iAverValveEadgeCntNum){ //  均压阀开关时机
    AverValveSwitch();
  }
  
  if(s_iCnt<s_iValvSwitchEadgeCntNum){  //  边缘持续时间
    return;
  }
  s_iCnt=0;
  
  //  到时间后打开另一系统阀
  if(s_iFlag == 0){ //  背面阀->正面阀PSA_VALVSWITCH_EADGE_CNT_5
    PSA_PinCtr(ENUM_PSA_PIN_SYS2,1);
  }
  else{//  正面阀->背面阀
    PSA_PinCtr(ENUM_PSA_PIN_SYS1,1);//  开正面阀
  }
  
  s_iPSAStatus=ENUM_PSA_STATUS_PRESSAVER; //  换完阀继续加气
  s_iFlag=1-s_iFlag;
}

//static void Switch_sys(void)
//{
//  static int i=0;
//  if(i<1){
//    printf("2");
//    PSA_PinCtr(ENUM_PSA_PIN_SYS1,0);
//    PSA_PinCtr(ENUM_PSA_PIN_SYS2,1);
//      i++;

//  }
//  else{
//        printf("1");
//    PSA_PinCtr(ENUM_PSA_PIN_SYS1,1);
//    PSA_PinCtr(ENUM_PSA_PIN_SYS2,0);
//    i=0;
//  }
//}

static void PSAStart(void)
{
  PSA_PinCtr(ENUM_PSA_PIN_FAN,1);  // 打开风扇
  PSA_PinCtr(ENUM_PSA_PIN_SYS1,1);  //背面阀
  PSA_PinCtr(ENUM_PSA_PIN_SYS2,0);
  PSA_PinCtr(ENUM_PSA_PIN_AVER,0);//1为开
  if(s_iPSAMode == PSA_MODE_CONTINUOUS)
  {
    PSA_PWMCtr(ENUM_O2_PWM,1);  //  打开氧气阀
  }
  else{
    PSA_PWMCtr(ENUM_O2_PWM,0);//  关闭氧气阀
  }
  s_iPSAStatus=ENUM_PSA_STATUS_PRESSUP;
    s_iPulseStatus=ENUM_PSA_STATUS_CALCTIME;

  PSA_PWMCtr(ENUM_COM_PWM,0.96);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void PSAAbort(void)
{
  PSA_PinCtr(ENUM_PSA_PIN_FAN,0);
  PSA_PinCtr(ENUM_PSA_PIN_SYS1,0);//背面阀
  PSA_PinCtr(ENUM_PSA_PIN_SYS2,0);
  PSA_PinCtr(ENUM_PSA_PIN_AVER,0);//1为开
  PSA_PWMCtr(ENUM_O2_PWM,0);//  打开氧气阀泄气
//  PSA_PinCtr(ENUM_PSA_PIN_O2,1);
  s_iPSAStatus=ENUM_PSA_STATUS_IDLE;//  返回空闲状态
  PSA_PWMCtr(ENUM_COM_PWM,0);
  s_iPulseStatus=ENUM_PSA_STATUS_IDLE;
  s_iPSAMode =255;  //  改成mode未定
}

/*************************************************
*   ENUM_PSA_STATUS_IDLE=0,
*   ENUM_PSA_STATUS_PRESSUP,
*   ENUM_PSA_STATUS_VALVESWITCH,
*   ENUM_PSA_STATUS_PRESSAVER,
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void PSAContinuous(void)
{
  static double s_PressVal=0;
  static unsigned short s_iCnt=0;
  
  
  s_PressVal=Sensors_GetVal(ENUM_SENSOR_WAVE);
  if(s_iPSAStatus == ENUM_PSA_STATUS_IDLE){ //  空闲状态
    return;
  }
  
  if(ENUM_PSA_STATUS_PRESSUP == s_iPSAStatus){  //  加压状态
//    if(s_PressVal >= 80000){  //  过压保持
//      PSA_PWMCtr(ENUM_COM_PWM,0); //  关闭压缩机
//      s_iPSAStatus=ENUM_PSA_STATUS_PRESSSTAY;
//    }
    s_iCnt++;
    if(s_iCnt >= s_iHighCntNum){  //  超时换筛
      s_iCnt=0;
//      PSA_PWMCtr(ENUM_O2_PWM,1);  // 打开氧气阀放氧
      s_iPSAStatus=ENUM_PSA_STATUS_VALVESWITCH;
    }
  }
  
  if(ENUM_PSA_STATUS_PRESSSTAY == s_iPSAStatus){  //  保持状态
    if(s_PressVal < 72000){
      PSA_PWMCtr(ENUM_COM_PWM,1);
      s_iPSAStatus=ENUM_PSA_STATUS_PRESSUP;
    }
  }
  
  if(ENUM_PSA_STATUS_VALVESWITCH== s_iPSAStatus){ //  换筛状态
    SwitchValve();
     // 换筛后开始均压
    s_iCnt=0;
  }
  
  if(ENUM_PSA_STATUS_PRESSAVER== s_iPSAStatus){   //  均压状态
    if(s_iCnt == 0){  //  均压
//      PSA_PinCtr(ENUM_PSA_PIN_AVER,1);
      s_iCnt=1;
    }
    else{ //关闭均压,开始加压
      PSA_PinCtr(ENUM_PSA_PIN_AVER,0);
      s_iPSAStatus=ENUM_PSA_STATUS_PRESSUP;
//      PSA_PWMCtr(ENUM_O2_PWM,0);  //  关闭氧气阀
      s_iCnt=0;
    }
  }
    
  //  压力监测，如果足够，进入保持状态，否则继续PSA
  
}

/*************************************************
*   
*   
*   
*   
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void SetLevel(void)
{  
  if(s_iLevelFlag == 1){ //  高档位
    s_iHighCntNum = PSA_LEVEL5_SYSVALV_HIGH;  //  系统阀A打开时间
    s_iLowCntNum  = PSA_LEVEL5_SYSVALV_LOW;   //  系统阀B关闭时间
    
    s_iAverOpenCntNum=PSA_AVERVALV_OPEN_CNT_5;  //  均压阀打开时间
    s_iValvSwitchEadgeCntNum=PSA_VALVSWITCH_EADGE_CNT_5;  //  阀切换时边缘时长
    s_iAverValveEadgeCntNum=PSA_AVERVALVE_EADGE_CNT_5;    //  均压阀的边缘时长
  }
  else{
    s_iHighCntNum = PSA_LEVEL4_SYSVALV_HIGH;  //  系统阀A打开时间
    s_iLowCntNum  = PSA_LEVEL4_SYSVALV_LOW;   //  系统阀B关闭时间
    
    s_iAverOpenCntNum=PSA_AVERVALV_OPEN_CNT_4;  //  均压阀打开时间
    s_iValvSwitchEadgeCntNum=PSA_VALVSWITCH_EADGE_CNT_4;  //  阀切换时边缘时长
    s_iAverValveEadgeCntNum=PSA_AVERVALVE_EADGE_CNT_4;    //  均压阀的边缘时长
  }
  s_iLevelFlag=1-s_iLevelFlag;
}

/*************************************************
*   ENUM_PSA_STATUS_CALCTIME,   
*   ENUM_PSA_STATUS_MEATURE,  
*   ENUM_PSA_STATUS_SUPPLYING,
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void PSAPulseFlow(void)
{
  static unsigned short s_iSupplyTime=50,s_iCnt=0;
  double wave_val=0;
  PSAContinuous();  //  持续制取氧气
  
  wave_val=Sensors_GetVal(ENUM_SENSOR_WAVE);
  //  脉冲供氧
  if(s_iPulseStatus ==  ENUM_PSA_STATUS_CALCTIME){  //  计算供氧时长状态
    if(wave_val > 100 || wave_val < -100){
      printf("breathing");
      LCD_SetItemText(207,"Breathing");
    }
    s_iCnt++;
    if(s_iCnt >= 2000){
      s_iCnt=0;
      s_iPulseStatus=ENUM_PSA_STATUS_MEATURE;
    }
  }
  
  if(s_iPulseStatus ==  ENUM_PSA_STATUS_MEATURE){  //  供氧点检测
    
    s_iCnt++;
    if(s_iCnt >= 3000){
      s_iCnt=0;
      s_iPulseStatus=ENUM_PSA_STATUS_SUPPLYING;
    }
  }
  
  if(s_iPulseStatus ==  ENUM_PSA_STATUS_SUPPLYING){  //  供氧时间
    if(s_iCnt == 0){
//        MasterSendO2Cmd(); 
    }
    
    s_iCnt++;
    
//    if(Sensors_GetVal(ENUM_SENSOR_PRESSURE) <=  60000){  //  当压力过低时会提前结束供氧
//      
//    }
        
    
    PSA_PinCtr(ENUM_PSA_PIN_O2,1);
    
//    MasterGetO2Data();
//    if(s_iCnt == s_iSupplyTime/2){  //  在正中段主动发送指令读取氧气数据
//      MasterGetO2Data();
////      LCD_Update(0);
//    }
//    
    if(s_iCnt <s_iSupplyTime){
      return;
    }
//    MasterGetData();
    PSA_PinCtr(ENUM_PSA_PIN_O2,0);
    s_iPulseStatus=ENUM_PSA_STATUS_MEATURE;
    s_iCnt=0; 
  }
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
void InitPSA(void)
{
  PSA_PinCtr(ENUM_PSA_PIN_FAN,0);
  PSA_PinCtr(ENUM_PSA_PIN_SYS1,0);//背面阀
  PSA_PinCtr(ENUM_PSA_PIN_SYS2,0);
  PSA_PinCtr(ENUM_PSA_PIN_AVER,0);//1为开
//  PSA_PWMCtr(ENUM_O2_PWM,1);  //  关闭氧气阀
  PSA_PinCtr(ENUM_PSA_PIN_O2,0);
  PSA_PWMCtr(ENUM_COM_PWM,0);
  
  s_iPSAStatus=ENUM_PSA_STATUS_IDLE;  //全部置于空闲状态
  s_iPulseStatus =  ENUM_PSA_STATUS_CALCTIME;
  SetLevel();
//  SetLevel();
  InitDataBuffer(&s_WaveBuffer,s_arrWave,WAVE_BUFFER_LEN);
//////    PSATest();

//  PSA_PWMCtr(ENUM_O2_PWM,0);
//  SetPWMDuty(ENUM_COM_PWM,0.5);
//  WriteChannelDuty(ENUM_COM_PWM,500);
//  SetPWMDuty(ENUM_SYS1_PWM,1);  //出气，背面为SYS1
//  SetPWMDuty(ENUM_SYS2_PWM,0);
//  SetPWMDuty(ENUM_CON_PWM,1);
//    SetPWMDuty(ENUM_FAN_PWM,1);

//  SetKEYCallBack(Switch_sys,1,ENUM_NAME_KEY2);

//  SetKEYCallBack(Switch_YASUOJI,1,ENUM_NAME_KEY1);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void PSA_Task(unsigned short cnt)
{
  static unsigned short s_iCnt=0;
  static unsigned char s_LCDPage=0;
  unsigned char  cur_page=LCD_GetPage();
  
  
  if(s_LCDPage != cur_page){
    s_LCDPage = cur_page;
    
    switch(s_LCDPage){
      case ENUM_LCD_PAGE_HOME:
        s_iCnt=0;
        PSAAbort();
        return;
        break;
      case ENUM_LCD_PAGE_PULSE:
        s_iPSAMode = PSA_MODE_PULSE;
        PSAStart();
        break;
      case ENUM_LCD_PAGE_CONTINUOUS:
        s_iPSAMode = PSA_MODE_CONTINUOUS;
        PSAStart();
        break;
      default:
        return;
        break;
      }
  }
  
  
  s_iCnt++;
  if(s_iCnt < cnt){
    return;
  }
  s_iCnt=0;
  
//  PSATest();

  if(s_iPSAMode == PSA_MODE_CONTINUOUS){  // 持续状态
    PSAContinuous();
  }
  else{
    PSAPulseFlow();
  }
  
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
