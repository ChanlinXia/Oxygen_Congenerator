/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
#ifndef _LCD_H_
#define _LCD_H_
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举
*********************************************************************************************************/
typedef enum
{
  ENUM_LCD_PAGE_HOME=0,
  ENUM_LCD_PAGE_FLOWSET,
  ENUM_LCD_PAGE_PULSE,
  ENUM_LCD_PAGE_CONTINUOUS,
  ENUM_LCD_PAGE_MAX
}EnumLCDPage;
/*********************************************************************************************************
*                                              结构体
*********************************************************************************************************/
typedef struct 
{
  unsigned short item_number;
  unsigned short pos[2];
  unsigned short size[2];
  unsigned short back_color;
  unsigned short front_color;
  unsigned char  selected_flag;
  unsigned short border_color;
  
  void*   ui_pointer;
  char Text[20]; 
  unsigned char text_size;
}StruLCDItem;

typedef struct 
{
  unsigned short back_color;
  unsigned short item_num;
  unsigned short selected_item_number;
  unsigned char  redraw_num;
//  unsigned short* redraw_item_list;
  StruLCDItem* page_item_list; 
}StruLCDPage;

/*********************************************************************************************************
*                                              API列表
*********************************************************************************************************/
void InitLCD(void);
void LCD_SetPage(EnumLCDPage page_name);
void LCD_Update(unsigned int cnt);
void LCD_SetItemText(unsigned short item_num,const char* text);
unsigned char LCD_GetPage(void);
void LCD_AlertShow(unsigned short warning);

#endif
