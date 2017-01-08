/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          AD7794.h
* Compiler :     
* Revision :     Revision
* Date :         2017-01-08
* Updated by :   
* Description :  
*                
* 
* LPC11C14 sytem clock: 48Mhz
* system clock: 48MHz
********************************************************************************
* This edition can only be used in DCBoard V6.0 using MornSun DC/DC Module
*******************************************************************************/

#include "stdlib.h"
#ifndef __AD7794_H__
#define	__AD7794_H__

#ifdef  APP_AD7794_EXT_DEF
#define APP_AD7794_EXT 	
#else
#define APP_AD7794_EXT	extern
#endif

//measurement channel
#define Channel_Weighting       0x01
#define Channel_UF              0x02
#define Channel_PreFlt          0x03
#define Channel_Vein            0x04
#define Channel_Artery          0x05

//mode register
#define Mode_MSB                0x00      // 0000 0000  0000 1001 
#define Mode_LSB                0x09      // continuous conversion, internal clock, AMP_CM=0, Chop_Dis=0, 16.7Hz
 
//configuration register
#define Config_Weighting_MSB    0x07      // 07 Gain=128 // 0000 0111     1011 0000   channel 1
#define Config_Weighting_LSB    0xB0      // Weighting  Channel_1: bipolar, Gain=128, internal Ref 1.17V,REF_DET

#define Config_UF_MSB           0x06      // 0000 0110     1011 0001   channel 2 
#define Config_UF_LSB           0xB1      // UF  Channel_2: bipolar, Gain=64, internal 1.17V Ref,REF_DET 

#define Config_Prefilter_MSB    0x06      // 0000 0110     1011 0010   channel 3
#define Config_Prefilter_LSB    0xB2      // PreFilter Channel_3: bipolar, Gain=64, internal Ref 1.17V,REF_DET

#define Config_Vein_MSB         0x06      // 0000 0110     1011 0011   channel 4 
#define Config_Vein_LSB         0xB3      // Vein Channel_4: bipolar, Gain=64, internal Ref 1.17V,REF_DET

#define Config_Artery_MSB       0x06      // 0000 0110     1011 0100   channel 5
#define Config_Artery_LSB       0xB4      // Artery    Channel_5: bipolar, Gain=64, internal Ref 1.17V,REF_DET

//Constant 
#define WeightCoefficient       (3730)
#define PRESS_COEFF_DEF         (20924)
#define PRESS_1MMHG             (12235)
#define VENOUS_PRESS_COEFF      (20)      //mmHg 治疗时静脉压判断与实际压力差值
#define PRESS_ERR_PRE           (30)      //mmHg 预报警差值
#define PRESS_TRBR_MAX          ( 500)    //mmHg 跨膜压压力高限
#define PRESS_MAX               ( 400)    //mmHg 压力高限
#define PRESS_MIN               (-400)    //mmHg 压力低限
#define WEIGHT_MAX              ( 2500)   //g 称重高限
#define WEIGHT_MIN              (-2500)   //g 称重低限
#define WEIGHT_TEST_ERR         ( 600)     //g 称重自检限值
#define WEIGHT_COF_MIN          ( 1000)   // 称重校准值低限值
#define WEIGHT_COF_MAX          ( 5000)   // 称重校准值高限值
#define WEIGHT_COF_DEF          ( 3730)   // 称重校准值默认值
#define WEIGHT_STABLE_BUF       ( 5000)   // 称重稳定AD值数组偏差阀值
#define WEIGHT_STABLE_VAL       ( 3 )     // 称重稳定称重值数组偏差阀值
//切换通道错误的次数
//全0或全1的次数
//未就绪的次数
APP_AD7794_EXT long ErrCntSwCh, ErrCnt0F, ErrCntReady, ErrCntHdret; 

/*
********************************************************************************
*  FUNCTION PROTOTYPES
********************************************************************************
*/
APP_AD7794_EXT void Reset_AD7794  (void);                                     // 复位AD7794
APP_AD7794_EXT void Select_Channel(unsigned char Channel);                    // 选择AD7794通道
APP_AD7794_EXT signed char ReadAd (void);                                     // 读取AD值
APP_AD7794_EXT void UartAddata(void);                                         // 读出的AD值从串口输出 用于测试

#define ABS_DIFF(A, B)    (((A) > (B)) ? ((A) - (B)):((B) - (A)))     //求两值差
#define ABS(A)            (((A) > 0) ? (A):( - (A)))                  //求绝对值

#endif   //#ifndef __AD7794_H

/*
The end of the file
*/
