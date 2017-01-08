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

#define DiffWeightAD        12000           //���ض�ȡ�����е����ֵ����Сֵ֮��
#define DiffWeight          7               //����ΪAD�����ֵ����Сֵ֮��
typedef struct
{
    signed short weighing;
    signed short UFValue;        //������
    signed short PreFltValue;    //����ǰѹ
    signed short VeinValue;      //����ѹ
    signed short ArteryValue;    //����ѹ
    signed short TMP;            //��ģѹ
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
