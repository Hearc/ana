/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          main.c
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

#include "board.h"
#include "AD7794.h"
#include "uart.h"
#include "chip.h"
#include "analog.h"
#include "timer.h"
#include "spi.h"
#include "state.h"

//.
unsigned char UartReceiveBuf[11];                           //���ڱ������λ���յ���ָ�

unsigned char ReadTime=0   ;         
int main(void)
{
    SystemCoreClockUpdate();
    Board_Init();
    Spi_init();                                                  //��ʼ��SPI�ӿ�
    uart_init();                                                 //��ʼ������

    timer_init();                                                //��ʼ����ʱ��
    timer32_1_init();                                            //��ʱ��1��ʼ��
    InitializeDeviceParameters();                                //��ʼ����Ҫ�Ĳ���
    Reset_AD7794();                                              //��λAD7794
    for(ReadTime=0;ReadTime<30;ReadTime++)
    {
        ReadAd();   
        myDelay(10);
    }
    
    while (1)
    {
         uart_cmd();                   //�����ڽ��յ���ָ��
         vTaskPress();                 //��ȡ����ѹ��ֵ���жϳ����Ƿ��ȶ��������ܳ��������ж��Ƿ񻻴����ж�Ԥ���Ƿ������AD7794����������
         HardwareSelfTest();           //ִ��ϵͳ�Լ�
         Calibration();                //ִ�г���У׼  
         CheckUFRate();                //���㳬����
    }
}

// end of the file
