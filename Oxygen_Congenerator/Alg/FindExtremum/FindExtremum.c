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
#include "FindExtremum.h"
#include <stdio.h>

/*********************************************************************************************************
*                                              宏定义
*********************************************************************************************************/
#define MAX_DATAIN_LEN 1800

/*********************************************************************************************************
*                                              枚举结构体
*********************************************************************************************************/

/*********************************************************************************************************
*                                              内部变量
*********************************************************************************************************/
static short s_arrSumTmep[MAX_DATAIN_LEN/2+1]={0},s_arrExtrePosTemp[MAX_DATAIN_LEN]={0};

/*********************************************************************************************************
*                                              内部函数声明
*********************************************************************************************************/
static void BufferClear(int dataLen);

static int max(DATA_TYPE* rawData,int dataLen); //返回最大值的索引
static int min(DATA_TYPE* rawData,int dataLen); //返回最小值的索引


static int FindPeaks(DATA_TYPE* rawData,int dataLen,DATA_TYPE* peaksPos,int startPos);
static int FindTroughs(DATA_TYPE* rawData,int dataLen,DATA_TYPE* troughsPos,int startPos);
static int SimpleFindPeaks(DATA_TYPE* rawData,int dataLen,DATA_TYPE* peaksPos,int startPos);
static int SimpleFindTroughs(DATA_TYPE* rawData,int dataLen,DATA_TYPE* troughsPos,int startPos);
static int SimpleFindTroughs2(DATA_TYPE* rawData,int dataLen,DATA_TYPE* troughsPos,int startPos);

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
static void BufferClear(int dataLen)
{
  int i=0;
  
  for(i=0;i<dataLen;i++){
    s_arrExtrePosTemp[i]=0;
    
    if(i< dataLen/2){
      s_arrSumTmep[i]=0;
    }
  }
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
static int FindPeaks(DATA_TYPE* rawData, int dataLen, DATA_TYPE* peaksPos, int startPos)
{
  int i = 0, k = 0, j = 0;
  int maxWinLen = 0, sumTemp = 0;

  for (k = 1; k < dataLen / 2 + 1; k++) {
    sumTemp = 0;

    for (i = k; i < dataLen - k; i++) {
      if (rawData[i] > rawData[i - k] && rawData[i] >= rawData[i + k]) {
        sumTemp++;
      }
    }

    s_arrSumTmep[k] = sumTemp;
  }

  maxWinLen = max(s_arrSumTmep, dataLen / 2);  //找出最大窗的长度

  printf("[[1,%d]]\r\n", maxWinLen);
  for (k = 1; k < maxWinLen + 1; k++) {
    for (i = k; i < dataLen - k; i++) {
      if (rawData[i] > rawData[i - k] && rawData[i] >= rawData[i + k]) {
        s_arrExtrePosTemp[i]++;
      }
    }
  }

  for (i = startPos; i < dataLen; i++) {
    if (s_arrExtrePosTemp[i] == maxWinLen) {
      peaksPos[j] = i;
      j++;
    }
  }

  for (i = 0; i < dataLen; i++) {
    if (s_arrExtrePosTemp[i] == maxWinLen) {
      peaksPos[j] = i;
      j++;
    }
  }
  
  //  printf("j:%d\r\n",j);
  return j;
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
static int FindTroughs(DATA_TYPE* rawData,int dataLen,DATA_TYPE* troughsPos,int startPos)
{
  int i=0,k=0,j=0;
  int maxWinLen=0,sumTemp=0;
  
  for(k=1;k<dataLen/2+1;k++){
    sumTemp=0;
    
    for(i=k;i<dataLen-k;i++){
      if(rawData[i] < rawData[i-k] && rawData[i] <= rawData[i+k]){
        sumTemp++;
      }
    }
    
    s_arrSumTmep[k]=sumTemp;
  }
  
  maxWinLen=max(s_arrSumTmep,dataLen/2);  //找出最大窗的长度
  
  for(k=1;k<maxWinLen+1;k++){
    for(i=k;i<dataLen-k;i++){
      if(rawData[i] < rawData[i-k] && rawData[i] <= rawData[i+k]){
        s_arrExtrePosTemp[i]++;
      }
    }
  }
  
  for(i=startPos;i<dataLen;i++){
    if(s_arrExtrePosTemp[i] == maxWinLen){
      troughsPos[j]=i;
      j++;
    }
  }
  
  for(i=0;i<startPos;i++){
    if(s_arrExtrePosTemp[i] == maxWinLen){
      troughsPos[j]=i;
      j++;
    }
  }
  
//  printf("j:%d\r\n",j);
  return j;
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
static int SimpleFindPeaks(DATA_TYPE* rawData, int dataLen, DATA_TYPE* peaksPos, int startPos)
{
  int i = 0, j = 0, k = 0, startIndex = 0;
  int refPeakVal=0,refPeakPos=0;
  
  for (i = startPos; i < dataLen; i++) {
    if (rawData[i - 1] < rawData[i] && rawData[i] >= rawData[i + 1]) {
      k = 0;
      while (1) {
        if (rawData[i + k] > rawData[i]) {
          break;
        }

        if (rawData[i + k] < rawData[i]) {
          peaksPos[j] = i;
          j++;
          break;
        }
        k++;
      }
      i += k;
    }
  }

  for (i = 0; i < startPos; i++) {
    if (rawData[i - 1] < rawData[i] && rawData[i] >= rawData[i + 1]) {
      k = 0;
      while (1) {
        if (rawData[i + k] > rawData[i]) {
          break;
        }

        if (rawData[i + k] < rawData[i]) {
          peaksPos[j] = i;
          j++;
          break;
        }

        k++;
      }
     i += k;
    }
  }
  
  refPeakPos=max(rawData+startPos,400);
  refPeakVal=rawData[refPeakPos];
//  
//  for(i=0;i<j;i++){
//    if()
//  }
//  

  return j;
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
static int max(DATA_TYPE* rawData,int dataLen)
{
  int i=0,maxPos=0,maxTemp=rawData[0];
  
  for(i=0;i<dataLen;i++){
    if(rawData[i] > maxTemp){
      maxTemp=rawData[i];
      maxPos=i;
    }
  }
  
  return maxPos;
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
static int min(DATA_TYPE* rawData,int dataLen)
{
  int i=0,minPos=0,minTemp=rawData[0];
  
  for(i=0;i<dataLen;i++){
    if(rawData[i] < minTemp){
      minTemp=rawData[i];
      minPos=i;
    }
  }
  
  return minPos;
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
static int SimpleFindTroughs(DATA_TYPE* rawData,int dataLen,DATA_TYPE* troughsPos,int startPos)
{
    int i = 0, j = 0, k = 0, startIndex = 0;
  //  int refPeakVal=0,refPeakPos=0;

  for (i = startPos + 1; i < dataLen - 1; i++) {
    if (rawData[i - 1] > rawData[i] && rawData[i] <= rawData[i + 1]) {
      k = 1;
      while (1) {
        if (i + k >= dataLen)
        {
          break;
        }

        if (rawData[i + k] < rawData[i]) {
          break;
        }

        if (rawData[i + k] > rawData[i]) {
          troughsPos[j] = i + k - 1;
          j++;
          break;
        }

        k++;
      }
           /*i += k*/;
    }
  }

  for (i = 1; i < startPos - 1; i++) {
    if (rawData[i - 1] > rawData[i] && rawData[i] <= rawData[i + 1]) {
      k = 0;
      while (1) {
        if (rawData[i + k] < rawData[i]) {
          break;
        }

        if (rawData[i + k] > rawData[i]) {
          troughsPos[j] = i + k - 1;
          j++;
          break;
        }

       k++;
      }
           /*i += k;*/
    }
  }
  //printf("j:%d",j);
  return j;
}

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
int FindExtremum(DATA_TYPE* rawData,int dataLen,DATA_TYPE* extremPos,
                  EnumFindExtermType PeakOrTrough,int startPos)
{
  int extremumNum=0;
  
  switch(PeakOrTrough){
    case ENUM_AMPD_PEAKS:
      extremumNum=FindPeaks(rawData,dataLen,extremPos,startPos);
      break;
    case ENUM_AMPD_TROUGH:
      extremumNum=FindTroughs(rawData,dataLen,extremPos,startPos);
      break;
    case ENUM_SIMPLE_FIND_PEAKS:
      extremumNum=SimpleFindPeaks(rawData,dataLen,extremPos,startPos);
    case ENUM_SIMPLE_FIND_THOUGHS:
      extremumNum=SimpleFindTroughs(rawData,dataLen,extremPos,startPos);
      break;
    case ENUM_FIND_MAXVAL:
      extremumNum=max(rawData,dataLen);
      break;
    case ENUM_FIND_MINVAL:
      extremumNum=min(rawData,dataLen);
      break;
    default:
      break;
  }
  
  BufferClear(dataLen);
  return extremumNum; 
}
