/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          analog.h
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

#ifndef __ANALOG_H__
#define	__ANALOG_H__

#define DiffWeightAD        12000           //称重读取数组中的最大值和最小值之差
#define DiffWeight          7               //换算为AD后最大值与最小值之差
typedef struct
{
    signed short weighing;
    signed short UFValue;        //超滤亚
    signed short PreFltValue;    //滤器前压
    signed short VeinValue;      //静脉压
    signed short ArteryValue;    //动脉压
    signed short TMP;            //跨模压
}Press_t;

signed long CalcAverage(signed long * buf, unsigned char num);
void Calc_Offset(void);
void  UF_Calc_Offset(void);
void calc_Pressue(void);
void vTaskPress(void );
void HardwareSelfTest(void);
void Calibration(void);
void CheckUFRate(void);

#endif
//end of the file
