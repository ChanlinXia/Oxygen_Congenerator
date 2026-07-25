/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "LCD.h"
#include "LCD_LIB.h"
#include "KEY_HW.h"
#include "ALL_SENSORS.h"
#include <stdio.h>
#include <string.h>
//#include "BEEP_HW.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define SPILIT_LOW_LINE_Y 207
#define SPILIT_HIGH_LINE_Y 33

#define STRING_SIZE 32
#define SLECTED_COLOR YELLOW

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static u8 s_iCurPageFlag=ENUM_LCD_PAGE_HOME;
static StruLCDItem s_arrItem[ENUM_LCD_PAGE_MAX][20]={0};
static StruLCDPage s_arrPage[ENUM_LCD_PAGE_MAX]={0};
static StruLCDItem* s_arrReDrawItem[20]={0};
static StruLCDPage* s_ReDrawPage=0;
static u8 s_iKeyProcFlag=255;
static double s_arrLCDSensorVal[ENUM_SENSOR_MAX]={0};
static StruLCDItem  s_arrWarningItem[2]={{501,{0,80},{240,34},GRAY,RED,0,0,0,"Low O2 Level",32},
                                         {502,{0,80},{240,34},GRAY,RED,0,0,0,"Pmax Alert",32}};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void LCDDrawItem(StruLCDItem* item);
static void LCDDrawPage(StruLCDPage* page);
static void LCDKeyProc(int key_name); 
static void SetKeyFlag(int key_name);
static void LCDValUpadate(unsigned short cnt);


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
static void LCDPageInit(void)
{
  //  HOME页控件BRRED
  s_arrItem[ENUM_LCD_PAGE_HOME][0]=(StruLCDItem){000,{0,0},{240,34},LBBLUE,BLACK,0,BLACK,0,"Mode Selection",32};
  s_arrItem[ENUM_LCD_PAGE_HOME][1]=(StruLCDItem){001,{0,34},{240,80},LGRAY,RED,1,BLACK,(void*)&s_arrPage[ENUM_LCD_PAGE_FLOWSET],"Continuous",32};
  s_arrItem[ENUM_LCD_PAGE_HOME][2]=(StruLCDItem){002,{0,114},{240,80},LGRAY,RED,0,BLACK,(void*)&s_arrPage[ENUM_LCD_PAGE_PULSE],"Pulse Flow",32};
  s_arrPage[ENUM_LCD_PAGE_HOME]=(StruLCDPage){BLACK,3,1,0,s_arrItem[ENUM_LCD_PAGE_HOME]};
  
  //  流量选择页控件
  s_arrItem[ENUM_LCD_PAGE_FLOWSET][0]=(StruLCDItem){100,{0,0},{240,34},LBBLUE,BLACK,0,BLACK,0,"Rate Selection",32};
  s_arrItem[ENUM_LCD_PAGE_FLOWSET][1]=(StruLCDItem){101,{0,34},{240,80},LGRAY,RED,1,BLACK,0,"1L/min",32};
  s_arrItem[ENUM_LCD_PAGE_FLOWSET][2]=(StruLCDItem){102,{0,114},{240,80},LGRAY,RED,0,BLACK,0,"2L/min",32};
  s_arrItem[ENUM_LCD_PAGE_FLOWSET][3]=(StruLCDItem){103,{0,194},{240,80},LGRAY,RED,0,BLACK,0,"3L/min",32};
  s_arrPage[ENUM_LCD_PAGE_FLOWSET]=(StruLCDPage){BLACK,4,1,0,s_arrItem[ENUM_LCD_PAGE_FLOWSET]};
  
  //  
  s_arrItem[ENUM_LCD_PAGE_PULSE][0]=(StruLCDItem){200,{0,0},{240,34},LBBLUE,BLACK,0,BLACK,0,"Pulse Mode",32};
  s_arrItem[ENUM_LCD_PAGE_PULSE][1]=(StruLCDItem){201,{0,160},{105,60},LGRAY,RED,0,BLACK,0,"\0",32};
  s_arrItem[ENUM_LCD_PAGE_PULSE][2]=(StruLCDItem){202,{5,150},{40,28},LGRAY,RED,0,0,0,"O2%",24};
  s_arrItem[ENUM_LCD_PAGE_PULSE][3]=(StruLCDItem){203,{10,180},{10,28},LGRAY,RED,0,0,0,"90.12",32};
  
  s_arrItem[ENUM_LCD_PAGE_PULSE][4]=(StruLCDItem){204,{105,160},{135,60},LGRAY,RED,0,BLACK,0,"\0",32};
  s_arrItem[ENUM_LCD_PAGE_PULSE][5]=(StruLCDItem){205,{110,150},{40,28},LGRAY,RED,0,0,0,"EQF/min",24};
  s_arrItem[ENUM_LCD_PAGE_PULSE][6]=(StruLCDItem){206,{130,180},{40,28},LGRAY,RED,0,0,0,"2.15L",32};
  s_arrItem[ENUM_LCD_PAGE_PULSE][7]=(StruLCDItem){207,{0,40},{240,34},LGRAY,RED,0,0,0,"O2 Supplying",32};

  
//  s_arrItem[ENUM_LCD_PAGE_PULSE][6]=(StruLCDItem){203,{100,160},{140,80},LGRAY,RED,0,BLACK,0,"EqF:3L/m",32};
  s_arrPage[ENUM_LCD_PAGE_PULSE]=(StruLCDPage){LGRAY,8,0,0,s_arrItem[ENUM_LCD_PAGE_PULSE]};
  
}

/*************************************************
*   
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void LCDDrawItem(StruLCDItem* item)
{
  unsigned short s_x=item->pos[0],s_y=item->pos[1],
                  x_size=item->size[0],y_size=item->size[1];
  
  unsigned short back_color=0;
  
  //  绘制背景色
  if(item->selected_flag == 1){
    back_color = SLECTED_COLOR;
//    LCD_Fill(s_x,s_y,s_x+x_size,s_y+y_size,SLECTED_COLOR);
  }
  else{
    back_color = item->back_color;
        // 背景色item->back_color
  }
  LCD_Fill(s_x,s_y,s_x+x_size,s_y+y_size,back_color);
  
  if(item->border_color != 0){  //  设置了边框颜色，则绘制边框&& item->selected_flag !=0
    LCD_DrawLine(s_x,s_y,s_x,s_y+y_size,item->border_color);
    LCD_DrawLine(s_x+x_size,s_y,s_x+x_size,s_y+y_size,item->border_color);
    
    LCD_DrawLine(s_x,s_y,s_x+x_size,s_y,item->border_color);
    LCD_DrawLine(s_x,s_y+y_size,s_x+x_size,s_y+y_size,item->border_color);
  }
  LCD_ShowString(s_x+5,1+s_y+(y_size-STRING_SIZE)/2,item->Text,item->front_color,
      back_color,item->text_size,0);
  
}

/*************************************************
*   
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void LCDKeyProc(int key_name)
{
  unsigned short temp=0,i=0;
  StruLCDPage* curpage=&s_arrPage[s_iCurPageFlag];
  StruLCDItem* item;
  
  switch(s_iKeyProcFlag){
    case ENUM_NAME_PWR:
//      printf("BACK");
      //  返回到HOME页
      if(s_iCurPageFlag !=ENUM_LCD_PAGE_HOME){
        s_ReDrawPage = &s_arrPage[ENUM_LCD_PAGE_HOME];
        s_iCurPageFlag=ENUM_LCD_PAGE_HOME;
      }
      
      break;
    case ENUM_NAME_KEY2:
      temp=curpage->selected_item_number;
      item=&curpage->page_item_list[temp];
      s_arrReDrawItem[0]=item;  //  将需要重绘的放进数组
      item->selected_flag=0;    //  取消选中
      
      temp++;
      if(temp >= curpage->item_num){
        temp=1;
      }
      printf("temp=%d",temp);
      item=&(curpage->page_item_list[temp]);
      curpage->selected_item_number=temp;
      item->selected_flag=1;
      s_arrReDrawItem[1]=item;
      curpage->redraw_num=2;
//      s_arrPage[s_iCurPageFlag]=curpage;
      break;
    case ENUM_NAME_KEY1:
      printf("key1Q2");
      temp=curpage->selected_item_number;
      item=&curpage->page_item_list[temp];
        
      switch(item->item_number){
        // case  item指针:
        
        // page 指针:
        default:s_ReDrawPage=(StruLCDPage*)item->ui_pointer;
          for(i=0;i<ENUM_LCD_PAGE_MAX;i++){ //  更新标志位
            if(&s_arrPage[i] == s_ReDrawPage){
              s_iCurPageFlag=i;
              break;
            }
          }
          break;
      }
      
      break;
    default:
      break;
  }
  s_iKeyProcFlag=255;
}

/*************************************************
*   
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void LCDDrawPage(StruLCDPage* page)
{
  int i=0;
  
  //  绘制背景颜色
  LCD_Fill(0,0,LCD_W,LCD_H,page->back_color);
  
  for(i=0;i<page->item_num;i++){
    LCDDrawItem(page->page_item_list+i);
  }
  
  //  选中第一个为
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void SetKeyFlag(int key_name)
{
  s_iKeyProcFlag  = key_name;
}

/*************************************************
*   更新LCD数值显示
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void LCDValUpadate(unsigned short cnt)
{
  static unsigned short s_iCnt=0;
  char str[5]={0},str1[6]={0};
  
  s_iCnt++;
  if(s_iCnt<cnt){
    return;
  }
  s_iCnt=0;
  
  if(s_iCurPageFlag == ENUM_LCD_PAGE_CONTINUOUS){ //  持续供氧模式的数值更新
    
  }
  
  if(s_iCurPageFlag == ENUM_LCD_PAGE_PULSE){  //  脉冲供氧的数值更新
    //  氧浓度更新
    if(s_arrLCDSensorVal[ENUM_SENSOR_O2] != Sensors_GetVal(ENUM_SENSOR_O2)){
      s_arrLCDSensorVal[ENUM_SENSOR_O2]=Sensors_GetVal(ENUM_SENSOR_O2);
      sprintf(str,"%.2f",s_arrLCDSensorVal[ENUM_SENSOR_O2]);
      LCD_SetItemText(203,str);
      
    }
    if(s_arrLCDSensorVal[ENUM_SENSOR_FLOW]!=Sensors_GetVal(ENUM_SENSOR_FLOW)){
      printf("LCD");
      s_arrLCDSensorVal[ENUM_SENSOR_FLOW]=Sensors_GetVal(ENUM_SENSOR_FLOW);  //  更新一次
      sprintf(str1,"%.2f",s_arrLCDSensorVal[ENUM_SENSOR_FLOW]);
      strcat(str1,"L");
      LCD_SetItemText(206,str1);
    }
    
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
void InitLCD(void)
{
  LCDPageInit();
//    LCD_Fill(0,0,LCD_W,LCD_H,LGRAY);  // 绘制窗体背景
//  
//  // 绘制参数和波形分界线
//  LCD_DrawLine(0,SPILIT_LOW_LINE_Y,LCD_W,SPILIT_LOW_LINE_Y,BLACK);  // 在指定位置画一条线
//  LCD_DrawLine(0,SPILIT_HIGH_LINE_Y,LCD_W,SPILIT_HIGH_LINE_Y,BLACK);  // 在指定位置画一条线
//  LCD_DrawLine(271,0,271,SPILIT_HIGH_LINE_Y,BLACK);  // 在指定位置画一条线
//  LCD_DrawLine(114,0,114,SPILIT_HIGH_LINE_Y,BLACK);  // 在指定位置画一条线
//  LCD_DrawLine(97,SPILIT_LOW_LINE_Y,97,280,BLACK);
//  LCD_DrawLine(206,SPILIT_LOW_LINE_Y,206,280,BLACK);
////    LCD_ShowChinese32x32(180,180,"夏",RED,WHITE,32,0);
//  
//  LCD_ShowString(207,208,"ChanLin",RED,GRAY,32,0);
//  
//  LCD_ShowString(0,208," 100% ",BRRED,YELLOW,32,0);
//  LCD_ShowString(100,208,"BT-OFF",BRRED,LGRAY,32,0);
//  
//  LCD_ShowString(0,0,"MONITOR",WHITE,LGRAYBLUE,32,0);
//    LCD_ShowString(117,0,"SPO2:100%",LIGHTGREEN,LGRAY,32,0);
////  LCD_ShowString(150,0,"ECG:98",LIGHTGREEN,LGRAY,32,0);
//  
//  LCD_ShowString(272,0,"OFF",LBBLUE,WHITE,32,0);
  
//  SetKEYCallBack(&LCD_SetPage,);
  
  SetKEYCallBack(SetKeyFlag,ENUM_NAME_KEY1,ENUM_NAME_KEY1);
  SetKEYCallBack(SetKeyFlag,ENUM_NAME_KEY2,ENUM_NAME_KEY2);
  SetKEYCallBack(SetKeyFlag,ENUM_NAME_PWR,ENUM_NAME_PWR);

  LCDDrawPage(&s_arrPage[ENUM_LCD_PAGE_HOME]);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void LCD_SetPage(EnumLCDPage page_name)
{
  
  s_iCurPageFlag = page_name;
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void LCD_Update(unsigned int cnt)
{
  static unsigned int s_iCnt=0;
  StruLCDPage* page=&s_arrPage[s_iCurPageFlag];
  int i=0;
  
  s_iCnt++;
  if(s_iCnt <= cnt){
    return;
  }
  s_iCnt=0;
  
  
  LCDKeyProc(0);  //  处理完上个key
//  LCDValUpadate(10);
//  LCD_AlertShow(1);
  
  if(page->redraw_num !=0 ){
    for(i=0;i<page->redraw_num;i++){
      LCDDrawItem(s_arrReDrawItem[i]);
    }
    page->redraw_num =0;
  }
  
  if(s_ReDrawPage !=0 ){
    LCDDrawPage(s_ReDrawPage);
    s_ReDrawPage=0;
  }
  
  
  
//  LCDDrawPage(&s_arrPage[s_iCurPageFlag]);
//  switch(s_iCurPageFlag){
//    case ENUM_LCD_PAGE_HOME:
//      break;
//    case ENUM_LCD_PAGE_FLOWSET:
//      break;
//    case ENUM_LCD_PAGE_PULSE:
//      break;
//    case ENUM_LCD_PAGE_CONTINUOUS:
//      break;
//    case ENUM_LCD_PAGE_MAX:
//      break;
//    default:
//      break;
//  }
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void LCD_SetItemText(unsigned short item_num,const char* text)
{
  StruLCDItem* item=&s_arrItem[item_num/100][item_num%100];
  StruLCDPage* page=&s_arrPage[item_num/100];
  
//  printf("%s",text);
  strcpy(item->Text,text);
//    printf("%s",item->Text);

  if(item_num/100 == s_iCurPageFlag){ // 是否要在下次刷新
    s_arrReDrawItem[page->redraw_num]=item;
    page->redraw_num++;
  }
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
unsigned char LCD_GetPage(void)
{
  return s_iCurPageFlag;
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void LCD_AlertShow(unsigned short warning)
{
  if(s_iCurPageFlag != ENUM_LCD_PAGE_CONTINUOUS && s_iCurPageFlag != ENUM_LCD_PAGE_PULSE){
    return;
  }
  s_arrReDrawItem[s_arrPage[s_iCurPageFlag].redraw_num]=&s_arrWarningItem[warning];
  s_arrPage[s_iCurPageFlag].redraw_num++;

}
