/*********************************************************************************************************
* 模块名称：
* 摘    要：
* 当前版本：
* 作    者：
* 完成日期：
* 内    容：
* 提 供API：
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
#include "DataBuffer.h"
#include <stdio.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：
* 函数功能： 
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：
* 注    意：
*********************************************************************************************************/

/*********************************************************************************************************
*                                              API函数实现
*********************************************************************************************************/
/*********************************************************************************************************
* 函数名称：
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：
* 注    意：
*********************************************************************************************************/
void InitDataBuffer(StructDataBuffer* pDaraBuffer,BUFFER_TYPE* arr,unsigned short alen)
{
  pDaraBuffer->len=alen;    //初始化容量
  pDaraBuffer->currIndex=0; //尾索引
  pDaraBuffer->pBuffer=arr; //给定缓冲区内存
  pDaraBuffer->front=0;     //头索引
  pDaraBuffer->currlen=0;   //数据现有长度
  pDaraBuffer->maxVal=0;
  pDaraBuffer->minVal=0XFFF;
}

/*********************************************************************************************************
* 函数名称：
* 函数功能：
* 输入参数：
* 输出参数：
* 返 回 值：
* 创建日期：
* 注    意：
*********************************************************************************************************/
void DataAppend(StructDataBuffer* pStrucBuffer,BUFFER_TYPE data) //末尾添加一个数据
{
  pStrucBuffer->pBuffer[pStrucBuffer->currIndex] = data;
  //printf("%d\r\n",data);
  if(pStrucBuffer->currlen < pStrucBuffer->len){  //如果现在长度小于了总容量长度
    pStrucBuffer->currlen++;
  }
//  printf("len:%d\r\n",pStrucBuffer->len);
//  printf("curr:%d\r\n",pStrucBuffer->currlen);
  pStrucBuffer->currIndex++;           //尾索引增加
  if(pStrucBuffer->currIndex >= pStrucBuffer->len){   //如果索引大于等于了容量长度
    pStrucBuffer->currIndex = 0;   //索引变为0
  }
  
  if(pStrucBuffer->currIndex < pStrucBuffer->front){ //如果尾索引小于等于了现在的头
    pStrucBuffer->front++;                     //头也增加
    if(pStrucBuffer->front >= pStrucBuffer->len){   //如果头超出了容量长度
      pStrucBuffer->front=0;           //重置
    }
  }
  
  if(data > pStrucBuffer->maxVal){
    pStrucBuffer->maxVal = data;
  }
  if(data < pStrucBuffer->minVal){
    pStrucBuffer->minVal = data;
  }
  
  if(pStrucBuffer ->currIndex == 0){
    pStrucBuffer->maxVal=0;
    pStrucBuffer->minVal=0XFFF;
  }
}

BUFFER_TYPE GetData(StructDataBuffer* pDataBuffer,int index)  //读取索引处的数据
{
  if(index < 0){    //索引小于0时
    if ((pDataBuffer->currIndex) + index < 0 ){
      return pDataBuffer->pBuffer[(pDataBuffer->currIndex) + index + pDataBuffer->len];
    }
    return pDataBuffer->pBuffer[(pDataBuffer->currIndex) + index];  //返回倒数的数据
  }
  if(index >= pDataBuffer->len){        //索引大于容量长度
    return pDataBuffer->pBuffer[index-(pDataBuffer->len)];  //返回多出位置的数据
  }
  return pDataBuffer->pBuffer[pDataBuffer->front+index];
}

void SetData(StructDataBuffer* pDataBuffer,int index,BUFFER_TYPE newData)  // 修改索引处的数据
{
  if(index < 0){    //索引小于0时
    if ((pDataBuffer->currIndex) + index < 0 ){
      pDataBuffer->pBuffer[(pDataBuffer->currIndex) + index + pDataBuffer->len]=newData;
      return;
    }
      pDataBuffer->pBuffer[(pDataBuffer->currIndex) + index]=newData;  //返回倒数的数据
  }
  if(index >= pDataBuffer->len)
  {
    pDataBuffer->pBuffer[index-(pDataBuffer->len)]=newData;
  }
  pDataBuffer->pBuffer[pDataBuffer->front+index]=newData;
}

void ClearBuffer(StructDataBuffer* pBuffer)   //清空缓冲区
{
  pBuffer->front=0;
  pBuffer->currIndex=0;
  pBuffer->currlen=0;
}

