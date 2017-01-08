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
unsigned char UartReceiveBuf[11];                           //用于保存从上位机收到的指令；

unsigned char ReadTime=0   ;         
int main(void)
{
    SystemCoreClockUpdate();
    Board_Init();
    Spi_init();                                                  //初始化SPI接口
    uart_init();                                                 //初始化串口

    timer_init();                                                //初始化定时器
    timer32_1_init();                                            //定时器1初始化
    InitializeDeviceParameters();                                //初始化需要的参数
    Reset_AD7794();                                              //复位AD7794
    for(ReadTime=0;ReadTime<30;ReadTime++)
    {
        ReadAd();   
        myDelay(10);
    }
    
    while (1)
    {
         uart_cmd();                   //处理串口接收到的指令
         vTaskPress();                 //读取计算压力值，判断称重是否稳定，计算总超滤量，判断是否换袋，判断预冲是否结束，AD7794错误重启；
         HardwareSelfTest();           //执行系统自检
         Calibration();                //执行称重校准  
         CheckUFRate();                //计算超滤率
    }
}

// end of the file
