/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          AD7794.c
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

#define APP_AD7794_EXT_DEF

#include "board.h"
#include "AD7794.h"
#include "spi.h"

//ADC寄存器值
volatile signed long AdData[6][3]={0}, AdDataTmp = 0, AdDataLast[6] = {0};
//工作通道
unsigned char CurrentCh = 1, CurrentChCnt = 0;
unsigned char addata[3];

//切换通道错误的次数
//全0或全1的次数
//未就绪的次数
long ErrCntSwCh = 0, ErrCnt0F = 0, ErrCntReady = 0, ErrCntHdret = 0;    

/*
********************************************************************************
**函数名称：Reset_AD7794
**函数功能：复位AD7794
**入口参数：无
**出口参数：无
**说    明：无
********************************************************************************
*/
void Reset_AD7794(void)
{
    unsigned char i;
    Board_GPIO_Set(0,false);                //SPI的片选   false ：0 ：有效 
    for (i = 0; i < 4; i++) 
    {
        SPI_ReadWriteByte(0xFF);            //write 32 consecutive 1
    }
    Board_GPIO_Set(0,true);
    
    CurrentCh = 1;
    CurrentChCnt = 0;
    Select_Channel(1);
}

/*
********************************************************************************
**函数名称：Select_Channel
**函数功能：选择AD7794通道
**入口参数：Channel 要使能的通道 (1~5)
**出口参数：无
**说    明：无
********************************************************************************
*/
void Select_Channel(unsigned char Channel)
{
    Board_GPIO_Set(0,false);
    switch (Channel) 
    {
        case Channel_UF :                               //UF Pressure channel = 2    
            SPI_ReadWriteByte (0x10);                   //write communication register. Next is to write Configuration register
            SPI_ReadWriteByte (Config_UF_MSB);          //write configuration register MSB 8
            SPI_ReadWriteByte (Config_UF_LSB);          //write configuration register LSB 8
            SPI_ReadWriteByte (0x08);                   //write communication Register.Next is to write Mode register
            SPI_ReadWriteByte (Mode_MSB);               // write mode MSB 8
            SPI_ReadWriteByte (Mode_LSB);               // write mode LSB 8
            Board_GPIO_Set(0,true);
            break;
            
        case Channel_Weighting :                        //weighting channel = 1
            SPI_ReadWriteByte (0x10);                   //write communication register. Next is to write Configuration register
            SPI_ReadWriteByte (Config_Weighting_MSB);   //write configuration register MSB 8
            SPI_ReadWriteByte (Config_Weighting_LSB);   //write configuration register LSB 8    
            SPI_ReadWriteByte (0x08);                   //write communication Register. Register.Next is to write Mode register
            SPI_ReadWriteByte (Mode_MSB);               // write mode MSB 8
            SPI_ReadWriteByte (Mode_LSB);               // write mode LSB 8
            Board_GPIO_Set(0,true);
            break;
            
        case Channel_PreFlt :                           //Prefilter pressure channel =3 
            SPI_ReadWriteByte (0x10);                   //write communication register. Next is to write Configuration register
            SPI_ReadWriteByte (Config_Prefilter_MSB);   //write configuration register MSB 8
            SPI_ReadWriteByte (Config_Prefilter_LSB);   //write configuration register LSB 8
            SPI_ReadWriteByte (0x08);                   //write communication Register. Register.Next is to write Mode register
            SPI_ReadWriteByte (Mode_MSB);               // write mode MSB 8
            SPI_ReadWriteByte (Mode_LSB);               // write mode LSB 8
            Board_GPIO_Set(0,true);
            break;
            
        case Channel_Artery :                           //withdral pressure channel =5
            SPI_ReadWriteByte (0x10);                   //write communication register. Next is to write Configuration register
            SPI_ReadWriteByte (Config_Artery_MSB);      //write configuration register MSB 8
            SPI_ReadWriteByte (Config_Artery_LSB);      //write configuration register LSB 8
            SPI_ReadWriteByte (0x08);                   //write communication Register. Register.Next is to write Mode register
            SPI_ReadWriteByte (Mode_MSB);               // write mode MSB 8
            SPI_ReadWriteByte (Mode_LSB);               // write mode LSB 8
            Board_GPIO_Set(0,true);
            break;
            
        case Channel_Vein :                             //infusion pressure channel =4   
            SPI_ReadWriteByte (0x10);                   //write communication register. Next is to write Configuration register
            SPI_ReadWriteByte (Config_Vein_MSB);        //write configuration register MSB 8
            SPI_ReadWriteByte (Config_Vein_LSB);        //write configuration register LSB 8
            SPI_ReadWriteByte (0x08);                   //write communication Register. Register.Next is to write Mode register
            SPI_ReadWriteByte (Mode_MSB);               // write mode MSB 8
            SPI_ReadWriteByte (Mode_LSB);               // write mode LSB 8
            Board_GPIO_Set(0,true) ;
            break;
            
        default:                                        //Default                
            Board_GPIO_Set(0,true)  ;
            break;
    }
}

/*
********************************************************************************
**函数名称：ReadAd
**函数功能：读取AD值
**入口参数：无
**出口参数：-1：通道有误   -2：全0、1错误   -3：两次读取值差偏大   0：未就绪
**说    明：读出的AD值存到AdData数组中
********************************************************************************
*/
signed char ReadAd(void)
{
    signed char rcv; 
    Board_GPIO_Set(0,false);
    SPI_ReadWriteByte(0X40);
    rcv = SPI_ReadWriteByte(0xff);
    if(CurrentCh != ((rcv&0x07)+1))
    {
        ErrCntSwCh++;
        CurrentChCnt = 0;
        Select_Channel(CurrentCh); 
        Board_GPIO_Set(0,true);
        return (-1);    //通道有误 
    }
    //数据是否准备好
    if ((rcv & 0x80)==0)      
    {
        ErrCntReady = 0;
        if(rcv & 0x40)              //是否全1 或全0错误
        {
            ErrCnt0F++;
            Board_GPIO_Set(0,true);
            return (-2);            //全0、1错误 
        }
        SPI_ReadWriteByte(0x58);    //write communication Register. Next is to read Data register
        AdDataTmp = SPI_ReadWriteByte(0xff);
        AdDataTmp <<= 8;
        AdDataTmp |= SPI_ReadWriteByte(0xff);
        AdDataTmp <<= 8;
        AdDataTmp |= SPI_ReadWriteByte(0xff);

        //判断两次读取值是否相差过大
        if(ABS_DIFF(AdDataLast[CurrentCh], AdDataTmp) < PRESS_1MMHG)
        {
            AdData[CurrentCh][CurrentChCnt] = AdDataTmp;
            AdDataLast[CurrentCh] = AdDataTmp;
            if(CurrentChCnt < 2) 
            {
                CurrentChCnt++;
            }
            else
            {
                CurrentChCnt = 0;
                if(CurrentCh < 5)
                { 
                    CurrentCh++;  
                }
                else
                { 
                    CurrentCh = 1; 
                }
                Select_Channel(CurrentCh);
            }
            Board_GPIO_Set(0,true);
            
            return (CurrentCh);
        }
        else
        {
            AdDataLast[CurrentCh] = AdDataTmp;
            Board_GPIO_Set(0,true);
            return (-3);        //两次读取值差偏大  
        }
    }
    ErrCntReady++;              //未就绪
    Board_GPIO_Set(0,true);
    return (0);	
}

/*
********************************************************************************
**函数名称：UartAddata
**函数功能：读出的AD值从串口输出 用于测试
**入口参数：无
**出口参数：无
**说    明：无
********************************************************************************
*/
void UartAddata(void)
{
    Chip_UART_SendByte(LPC_USART,CurrentCh+'0');
    Chip_UART_SendByte(LPC_USART,'\t');
    Chip_UART_SendByte(LPC_USART,CurrentChCnt+'0');
    Chip_UART_SendByte(LPC_USART,'\t');
    //  UartLong(AdData[CurrentCh][CurrentChCnt]);
    Chip_UART_SendByte(LPC_USART,'\t');
    
    if((5 == CurrentCh)&&(2 == CurrentChCnt))
    {
        Chip_UART_SendByte(LPC_USART,0x0d);
        Chip_UART_SendByte(LPC_USART,0x0a);
    }
}
/*
The end of the file
*/
