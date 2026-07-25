/*********************************************************************************************************
*
*   @author   Chanlin
*   @version  1.0
*   @update   none
*********************************************************************************************************/
/*********************************************************************************************************
*                                              包含头文件
*********************************************************************************************************/
#include "ADC.h"
#include "gd32f30x_conf.h"

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
static unsigned short s_arrADCVal[ENUM_NAME_ADC_MAX]={0};

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void ConfigGPIO(void);
static void ConfigADC(void);
static void ConfigDMA(void);

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
static void ConfigGPIO(void)
{
  rcu_periph_clock_enable(RCU_GPIOC);
  
  gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, BITS(0,3));
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
static void ConfigADC(void)
{
  static const uint8_t ADCChannelList[ENUM_NAME_ADC_MAX]={ADC_CHANNEL_10,ADC_CHANNEL_11,
                                         ADC_CHANNEL_12,ADC_CHANNEL_13};
  int i=0;
  // 配置ADC时钟源
  rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV6);  // 120M / 6 = 20MHz                                         
  
  adc_mode_config(ADC_MODE_FREE);
  
  //配置ADC0
  rcu_periph_clock_enable(RCU_ADC0);                                                            //使能ADC0的时钟
  adc_deinit(ADC0);                                                                             //复位ADC0
  adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);                                     //使能ADC扫描，即开启多通道转换
  adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);                               //使能连续采样
  adc_resolution_config(ADC0, ADC_RESOLUTION_12B);                                              //规则组配置，12位分辨率
  adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);                                         //右对齐
  adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, ENUM_NAME_ADC_MAX);                          //规则组长度，即ADC通道数量
  adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);                               //规则组使能外部触发
  adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); //规则组使用软件触发
  adc_oversample_mode_disable(ADC0);                                                            //禁用过采样
  
  // 通道采样顺序配置，采样顺序从0开始
  for(i=0;i<ENUM_NAME_ADC_MAX;i++){
    adc_regular_channel_config(ADC0, i, ADCChannelList[i], ADC_SAMPLETIME_239POINT5); // 1/20M * (239.5 + 12.5) = 12.6 us
  }    
//    adc_regular_channel_config(ADC0, 0, ADCChannelList[3], ADC_SAMPLETIME_239POINT5); // 1/20M * (239.5 + 12.5) = 12.6 us

  //ADC0使能
  adc_enable(ADC0);

  //使能ADC0校准
  adc_calibration_enable(ADC0);

  //使能DMA
  adc_dma_mode_enable(ADC0);

  //规则组软件触发
  adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, DISABLE);                               //规则组使能外部触发

}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
  static void ConfigDMA(void)
{
    //DMA初始化结构体
  dma_parameter_struct dma_init_struct;

  //DMA配置
  rcu_periph_clock_enable(RCU_DMA0);                           //使能DMA0时钟
  dma_deinit(DMA0, DMA_CH0);                                   //初始化结构体设置默认值
  dma_init_struct.direction  = DMA_PERIPHERAL_TO_MEMORY;       //设置DMA数据传输方向
  dma_init_struct.memory_addr  = (uint32_t)s_arrADCVal;        //内存地址设置
  dma_init_struct.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;   //内存增长使能
  dma_init_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;       //内存数据位数设置
  dma_init_struct.number       = ENUM_NAME_ADC_MAX * 1;            //内存数据量设置
  dma_init_struct.periph_addr  = (uint32_t)&(ADC_RDATA(ADC0)); //外设地址设置
  dma_init_struct.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;  //外设地址增长失能
  dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;   //外设数据位数设置
  dma_init_struct.priority   = DMA_PRIORITY_ULTRA_HIGH;        //优先级设置
  dma_init(DMA0, DMA_CH0, &dma_init_struct);                   //初始化结构体

  //DMA模式设置
  dma_circulation_enable(DMA0, DMA_CH0);             //使能循环
  dma_memory_to_memory_disable(DMA0, DMA_CH0);       //失能内存到内存

  //使能DMA
  dma_channel_enable(DMA0, DMA_CH0);
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
void InitADC(void)
{
  ConfigGPIO();
  ConfigDMA();
  ConfigADC();
}

/*************************************************
*
*
*   @param  void
*   @return void
*   @author Chanlin
**************************************************/
unsigned short  GetADCVal(EnumADCName ADCName)
{
  return s_arrADCVal[ADCName];
}

