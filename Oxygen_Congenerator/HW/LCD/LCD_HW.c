/*********************************************************************************************************
*   此文件为对厂家（汉昇）例程的移植
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "LCD_HW.h"
#include "gd32f30x_conf.h"
#include "systick.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/


// 设置引脚控制宏
#define LCD_SCLK_Clr() { gpio_bit_write(GPIOA,GPIO_PIN_12,RESET); }//SCL=SCLK
#define LCD_SCLK_Set() { gpio_bit_write(GPIOA,GPIO_PIN_12,SET); }

#define LCD_MOSI_Clr() { gpio_bit_write(GPIOA,GPIO_PIN_11,RESET); }//SDA=MOSI
#define LCD_MOSI_Set() { gpio_bit_write(GPIOA,GPIO_PIN_11,SET); }

#define LCD_RES_Clr()  { gpio_bit_write(GPIOA,GPIO_PIN_8,RESET); }//RES
#define LCD_RES_Set()  { gpio_bit_write(GPIOA,GPIO_PIN_8,SET); }

#define LCD_DC_Clr()   { gpio_bit_write(GPIOB,GPIO_PIN_12,RESET); }//DC
#define LCD_DC_Set()   { gpio_bit_write(GPIOB,GPIO_PIN_12,SET); }
 		     
#define LCD_CS_Clr()   { gpio_bit_write(GPIOA,GPIO_PIN_15,RESET); }//CS
#define LCD_CS_Set()   { gpio_bit_write(GPIOA,GPIO_PIN_15,SET); }

#define LCD_BLK_Clr()  { gpio_bit_write(GPIOB,GPIO_PIN_1,RESET); }//BLK
#define LCD_BLK_Set()  { gpio_bit_write(GPIOB,GPIO_PIN_1,SET); }

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void GPIOConfig(void);
  
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
  rcu_periph_clock_enable(RCU_GPIOA|RCU_GPIOB|RCU_AF);   //打开端口时钟
  gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);    // 重映射恢复PA15 PB3 PB4为IO 
  
  gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15);
  gpio_bit_set(GPIOA,GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15);
  gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_1|GPIO_PIN_12);
  gpio_bit_set(GPIOB,GPIO_PIN_1|GPIO_PIN_12);
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void LCD_Writ_Bus(uint8_t dat) 
{	
	uint8_t i;
	LCD_CS_Clr();
	for(i=0;i<8;i++)
	{			  
		LCD_SCLK_Clr();
    //Delay1us();
        //Delay1us();

		if(dat&0x80) { LCD_MOSI_Set(); }
		else { LCD_MOSI_Clr();}
        //Delay1us();
    //Delay1us();

		LCD_SCLK_Set();
    //Delay1us();
        //Delay1us();

		dat<<=1;
	}	
  LCD_CS_Set();	
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
void LCD_WR_DATA8(uint8_t dat)
{
	LCD_Writ_Bus(dat);
}


void LCD_WR_DATA(uint16_t dat)
{
	LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
}


void LCD_WR_REG(uint8_t dat)
{
	LCD_DC_Clr();//写命令
	LCD_Writ_Bus(dat);
	LCD_DC_Set();//写数据
}

void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
	}
	else
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1);
		LCD_WR_DATA(x2);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1);
		LCD_WR_DATA(y2);
		LCD_WR_REG(0x2c);//储存器写
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
void InitLCDHW(void)
{
	GPIOConfig();//初始化GPIO
	
	LCD_RES_Clr();//??
	DelayNms(100);
	LCD_RES_Set();
	DelayNms(100);
	
	LCD_BLK_Set();//????
//  LCD_BLK_Clr();
    DelayNms(100);
	
//************* Start Initial Sequence **********//
	LCD_WR_REG(0x11); //Sleep out 
	DelayNms(120);              //Delay 120ms 
	//************* Start Initial Sequence **********// 
	LCD_WR_REG(0x36);
	if(USE_HORIZONTAL==0)LCD_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)LCD_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)LCD_WR_DATA8(0x70);
	else LCD_WR_DATA8(0xA0);
	
	/*
	LCD_WR_REG(0x3A);			
	LCD_WR_DATA8(0x05);

	LCD_WR_REG(0xB2);			
	LCD_WR_DATA8(0x0C);
	LCD_WR_DATA8(0x0C); 
	LCD_WR_DATA8(0x00); 
	LCD_WR_DATA8(0x33); 
	LCD_WR_DATA8(0x33); 			

	LCD_WR_REG(0xB7);			
	LCD_WR_DATA8(0x35);
//---------------------------------ST7789V Power
	LCD_WR_REG(0xBB);
	//LCD_WR_DATA8(0x32); //Vcom=1.35V
	LCD_WR_DATA8(0x20); //Vcom=1.35V			
	

	LCD_WR_REG(0xC0);
	LCD_WR_DATA8(0x2C);
					
	LCD_WR_REG(0xC2);
	LCD_WR_DATA8(0x01);

	LCD_WR_REG(0xC3);			
	//LCD_WR_DATA8(0x15); //GVDD=4.8V  ????
	LCD_WR_DATA8(0x0B); //GVDD=4.8V  ????
				
	LCD_WR_REG(0xC4);			
	LCD_WR_DATA8(0x20); //VDV, 0x20:0v

	LCD_WR_REG(0xC6);			
	LCD_WR_DATA8(0x0F); //0x0F:60Hz        	

	LCD_WR_REG(0xD0);			
	LCD_WR_DATA8(0xA4);
	LCD_WR_DATA8(0xA1); 

//---------ST7789V gamma setting--------//
	LCD_WR_REG(0xE0);
	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x05);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x48);   
	LCD_WR_DATA8(0x17);   
	LCD_WR_DATA8(0x14);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x34);   

	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x03);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x11);   
	LCD_WR_DATA8(0x3D);   
	LCD_WR_DATA8(0x47);   
	LCD_WR_DATA8(0x55);   
	LCD_WR_DATA8(0x53);   
	LCD_WR_DATA8(0x1A);   
	LCD_WR_DATA8(0x16);   
	LCD_WR_DATA8(0x14);   
	LCD_WR_DATA8(0x1F);   
	LCD_WR_DATA8(0x22); 

	LCD_WR_REG(0xE1);     
    LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x0E);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x09);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x33);   
	LCD_WR_DATA8(0x48);   
	LCD_WR_DATA8(0x17);   
	LCD_WR_DATA8(0x14);   
	LCD_WR_DATA8(0x15);   
	LCD_WR_DATA8(0x31);   
	LCD_WR_DATA8(0x34);	

	LCD_WR_DATA8(0xD0);   
	LCD_WR_DATA8(0x02);   
	LCD_WR_DATA8(0x08);   
	LCD_WR_DATA8(0x0D);   
	LCD_WR_DATA8(0x12);   
	LCD_WR_DATA8(0x2C);   
	LCD_WR_DATA8(0x43);   
	LCD_WR_DATA8(0x55);   
	LCD_WR_DATA8(0x53);   
	LCD_WR_DATA8(0x1E);   
	LCD_WR_DATA8(0x1B);   
	LCD_WR_DATA8(0x19);   
	LCD_WR_DATA8(0x20);   
	LCD_WR_DATA8(0x22);


	//LCD_WR_REG(0x21); 
	LCD_WR_REG(0x29);	
	 */
	 //*
//LCD_WR_REG(0x36);     
//LCD_WR_DATA8(0x00);   
//LCD_WR_REG(0x21); 



LCD_WR_REG(0x3A);			
LCD_WR_DATA8(0x05);
	    
LCD_WR_REG(0xB2);     
LCD_WR_DATA8(0x0c);   // 05 0C
LCD_WR_DATA8(0x0c);   // 05 OC
LCD_WR_DATA8(0x00);   
LCD_WR_DATA8(0x33);   
LCD_WR_DATA8(0x33);   

LCD_WR_REG(0xB7);     
LCD_WR_DATA8(0x75);   

LCD_WR_REG(0xBB);     
LCD_WR_DATA8(0x22);   

LCD_WR_REG(0xC0);     
LCD_WR_DATA8(0x2C);   

LCD_WR_REG(0xC2);     
LCD_WR_DATA8(0x01);   

LCD_WR_REG(0xC3);     
LCD_WR_DATA8(0x13);   

LCD_WR_REG(0xC4);     
LCD_WR_DATA8(0x20);   

LCD_WR_REG(0xC6);     
LCD_WR_DATA8(0x11);   

LCD_WR_REG(0xD0);     
LCD_WR_DATA8(0xA4);   
LCD_WR_DATA8(0xA1);   

LCD_WR_REG(0xD6);     
LCD_WR_DATA8(0xA1);   

LCD_WR_REG(0xE0);     
LCD_WR_DATA8(0xD0);   
LCD_WR_DATA8(0x05);   
LCD_WR_DATA8(0x0A);   
LCD_WR_DATA8(0x09);   
LCD_WR_DATA8(0x08);   
LCD_WR_DATA8(0x05);   
LCD_WR_DATA8(0x2E);   
LCD_WR_DATA8(0x44);   
LCD_WR_DATA8(0x45);   
LCD_WR_DATA8(0x0F);   
LCD_WR_DATA8(0x17);   
LCD_WR_DATA8(0x16);   
LCD_WR_DATA8(0x2B);   
LCD_WR_DATA8(0x33);   

LCD_WR_REG(0xE1);     
LCD_WR_DATA8(0xD0);   
LCD_WR_DATA8(0x05);   
LCD_WR_DATA8(0x0A);   
LCD_WR_DATA8(0x09);   
LCD_WR_DATA8(0x08);   
LCD_WR_DATA8(0x05);   
LCD_WR_DATA8(0x2E);   
LCD_WR_DATA8(0x43);   
LCD_WR_DATA8(0x45);   
LCD_WR_DATA8(0x0F);   
LCD_WR_DATA8(0x16);   
LCD_WR_DATA8(0x16);   
LCD_WR_DATA8(0x2B);   
LCD_WR_DATA8(0x33);   
//LCD_WR_REG(0x21);     //颜色反向显示
LCD_WR_REG(0x29);     //Display on	*/
	
} 

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
