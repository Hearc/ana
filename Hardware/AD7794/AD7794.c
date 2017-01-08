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

//ADC�Ĵ���ֵ
volatile signed long AdData[6][3]={0}, AdDataTmp = 0, AdDataLast[6] = {0};
//����ͨ��
unsigned char CurrentCh = 1, CurrentChCnt = 0;
unsigned char addata[3];

//�л�ͨ������Ĵ���
//ȫ0��ȫ1�Ĵ���
//δ�����Ĵ���
long ErrCntSwCh = 0, ErrCnt0F = 0, ErrCntReady = 0, ErrCntHdret = 0;    

/*
********************************************************************************
**�������ƣ�Reset_AD7794
**�������ܣ���λAD7794
**��ڲ�������
**���ڲ�������
**˵    ������
********************************************************************************
*/
void Reset_AD7794(void)
{
    unsigned char i;
    Board_GPIO_Set(0,false);                //SPI��Ƭѡ   false ��0 ����Ч 
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
**�������ƣ�Select_Channel
**�������ܣ�ѡ��AD7794ͨ��
**��ڲ�����Channel Ҫʹ�ܵ�ͨ�� (1~5)
**���ڲ�������
**˵    ������
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
**�������ƣ�ReadAd
**�������ܣ���ȡADֵ
**��ڲ�������
**���ڲ�����-1��ͨ������   -2��ȫ0��1����   -3�����ζ�ȡֵ��ƫ��   0��δ����
**˵    ����������ADֵ�浽AdData������
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
        return (-1);    //ͨ������ 
    }
    //�����Ƿ�׼����
    if ((rcv & 0x80)==0)      
    {
        ErrCntReady = 0;
        if(rcv & 0x40)              //�Ƿ�ȫ1 ��ȫ0����
        {
            ErrCnt0F++;
            Board_GPIO_Set(0,true);
            return (-2);            //ȫ0��1���� 
        }
        SPI_ReadWriteByte(0x58);    //write communication Register. Next is to read Data register
        AdDataTmp = SPI_ReadWriteByte(0xff);
        AdDataTmp <<= 8;
        AdDataTmp |= SPI_ReadWriteByte(0xff);
        AdDataTmp <<= 8;
        AdDataTmp |= SPI_ReadWriteByte(0xff);

        //�ж����ζ�ȡֵ�Ƿ�������
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
            return (-3);        //���ζ�ȡֵ��ƫ��  
        }
    }
    ErrCntReady++;              //δ����
    Board_GPIO_Set(0,true);
    return (0);	
}

/*
********************************************************************************
**�������ƣ�UartAddata
**�������ܣ�������ADֵ�Ӵ������ ���ڲ���
**��ڲ�������
**���ڲ�������
**˵    ������
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
