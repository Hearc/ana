/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          timer.c
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

#include "timer.h"
#include "timer_11xx.h"
#include "analog.h"
#include "uart.h"
#include "Glb.h"

#define TICKRATE_HZ1 (1)        /* 1 ticks per second   */
#define TICKRATE_HZ2 (100)      /* 100 ticks per second */

//��¼�����ʶ
unsigned long ANA_ERR1=0;
extern volatile long AdData[6][3];
extern unsigned int  TotalTimeAPU;
extern unsigned char ReceiveSelfCheck,WeightSelttestSecond;
//ÿ�����Ӹ���һ�γ��ˣ�
extern unsigned char UpdateUfStartFlag,UpdateUfEndFlag;
extern YaLi_t YaLi;
extern UsedMinutes_t  UsedMinute;
extern unsigned short SelfTestResult;
extern unsigned long SetUFRate,ActualUFRate;

/*****************************************************************************
 * LPC_TIMER32_0�жϳ�ʼ��
 * ÿ1S���ж�
 ****************************************************************************/
void timer_init(void)
{
    uint32_t timerFreq;
    /* Enable and setup SysTick Timer at a periodic rate */
    SysTick_Config(SystemCoreClock / TICKRATE_HZ1);

    /* Enable timer 1 clock */
    Chip_TIMER_Init(LPC_TIMER32_0);     

    /* Timer rate is system clock rate */
    timerFreq = Chip_Clock_GetSystemClockRate();

    /* Timer setup for match and interrupt at TICKRATE_HZ */
    Chip_TIMER_Reset(LPC_TIMER32_0);
    Chip_TIMER_MatchEnableInt(LPC_TIMER32_0, 1);
    Chip_TIMER_SetMatch(LPC_TIMER32_0, 1, (timerFreq / TICKRATE_HZ1));
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 1);
    Chip_TIMER_Enable(LPC_TIMER32_0);

    /* Enable timer interrupt */
    NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
    NVIC_EnableIRQ(TIMER_32_0_IRQn);
}

/*****************************************************************************
 * LPC_TIMER32_0��ʱ���ж�
 * ָʾ�Ʒ�ת
 * ����A0--A5����
 * ���¿����ϵ�ʱ��
 ****************************************************************************/
bool led_state;
char UartBufferA0[8], UartBufferA1[8], UartBufferA2[8], UartBufferA3[8], UartBufferA4[8];
extern STATE_t  StateFlag;

uint8_t m;
uint8_t updateUFtime=0;
void TIMER32_0_IRQHandler(void)
{
    uint8_t i;

    if(Chip_TIMER_MatchPending(LPC_TIMER32_0, 1)) 
    {
        Chip_TIMER_ClearMatch(LPC_TIMER32_0, 1);
        //ָʾ�Ʒ�ת
        led_state=!led_state;
        Board_LED_Set(0, led_state);
        // ���¿����ϵ�ʱ��
        TotalTimeAPU++;    
        //��ʱ3S��   �����Լ�ʱ����        
        if(ReceiveSelfCheck==1)
        {
             m++;
             if(m==3)
             {
                 WeightSelttestSecond=1;
                 ReceiveSelfCheck=0;
             }
        }
         //��ʱ5S��   ÿ5S����һ���ܳ�����
        if(UpdateUfStartFlag==1)
        {
            updateUFtime++;
            if(updateUFtime>=5)
            {
                updateUFtime=0;
                UpdateUfEndFlag=1;
            }
        }

        //calc_Pressue();
        //����A0--A5����
        UartBufferA0[0]=0xA0;                                                  //�����Լ����ʹ����ʶ
        UartBufferA0[1]=StateFlag.MainState;
        UartBufferA0[2]=(SelfTestResult>>8)&0xff;
        UartBufferA0[3]=SelfTestResult&0xff;
        UartBufferA0[4]=(ANA_ERR1>>24)&0xff;
        UartBufferA0[5]=(ANA_ERR1>>16)&0xff;
        UartBufferA0[6]=(ANA_ERR1>>8)&0xff;
        UartBufferA0[7]=(ANA_ERR1)&0xff;
        for(i=0;i<8;i++)
        {
            //printf("%c",UartBufferA0[i]);
            Chip_UART_SendByte(LPC_USART,UartBufferA0[i]);
            myDelay(2);
        }

        UartBufferA1[0]=0xA1;                                                 //���Ͷ���ѹ������ѹ�ͳ���ѹ
        UartBufferA1[1]=StateFlag.MainState;
        UartBufferA1[2]=(YaLi.Arterial>>8)&0xff;
        UartBufferA1[3]=YaLi.Arterial&0xff;
        UartBufferA1[4]=(YaLi.Venous>>8)&0xff;
        UartBufferA1[5]=YaLi.Venous&0xff;
        UartBufferA1[6]=(YaLi.UF>>8);
        UartBufferA1[7]=YaLi.UF;

        for(i=0;i<8;i++)
        {
            //	printf("%c",UartBufferA1[i]);
            Chip_UART_SendByte(LPC_USART,UartBufferA1[i]);
            myDelay(2);
        }

        UartBufferA2[0]=0xA2;                                                  //������ǰѹ����ģѹ�ͳ���ֵ
        UartBufferA2[1]=StateFlag.MainState;
        UartBufferA2[2]=(YaLi.PreFlt>>8)&0xff;
        UartBufferA2[3]=(YaLi.PreFlt)&0xff;
        UartBufferA2[4]=(YaLi.TMP>>8)&0xff;   
        UartBufferA2[5]=(YaLi.TMP)&0xff; 
        UartBufferA2[6]=(YaLi.Weight>>8)&0xff;
        UartBufferA2[7]=(YaLi.Weight)&0xff;

        for(i=0;i<8;i++)
        {
            //	printf("%c",UartBufferA2[i]);
            Chip_UART_SendByte(LPC_USART,UartBufferA2[i]);
            myDelay(2);
        }

        UartBufferA3[0]=0xA3;                                                   // �����ܳ�����
        UartBufferA3[1]=StateFlag.MainState;
        UartBufferA3[2]=0;
        UartBufferA3[3]=0;
        UartBufferA3[4]=(UsedMinute.TotalUFVolume>>24)&0xff;   
        UartBufferA3[5]=(UsedMinute.TotalUFVolume>>16)&0xff;
        UartBufferA3[6]=(UsedMinute.TotalUFVolume>>8)&0xff; 
        UartBufferA3[7]=(UsedMinute.TotalUFVolume)&0xff; 

        for(i=0;i<8;i++)
        {
            //	printf("%c",UartBufferA3[i]);
            Chip_UART_SendByte(LPC_USART,UartBufferA3[i]);
            myDelay(2);
        }

        UartBufferA4[0]=0xA4;                       //�����趨�ĳ����ʺ�ʵ�ʳ�����
        UartBufferA4[1]=StateFlag.MainState;
        UartBufferA4[2]=0;
        UartBufferA4[3]=0;
        UartBufferA4[4]=(SetUFRate>>8)&0xff;        //Byte4��5   �趨�ĳ�����  
        UartBufferA4[5]=SetUFRate;
        UartBufferA4[6]=(ActualUFRate>>8)&0xff;     //Byte6, 7   ʵ�ʵĳ�����
        UartBufferA4[7]=ActualUFRate;
        
        for(i=0;i<8;i++)
        {
            //	printf("%c",UartBufferA4[i]);
            Chip_UART_SendByte(LPC_USART,UartBufferA4[i]);
            myDelay(2);
        }
    }
}

/*****************************************************************************
 * timer32_1��ʼ����
 * ÿ1S�ж�100��
 ****************************************************************************/
void timer32_1_init(void)
{
    uint32_t timerFreq;
    /* Enable and setup SysTick Timer at a periodic rate */
    //SysTick_Config(SystemCoreClock / TICKRATE_HZ1);

    /* Enable timer 1 clock */
    Chip_TIMER_Init(LPC_TIMER32_1);     

    /* Timer rate is system clock rate */
    timerFreq = Chip_Clock_GetSystemClockRate();

    /* Timer setup for match and interrupt at TICKRATE_HZ */
    Chip_TIMER_Reset(LPC_TIMER32_1);
    Chip_TIMER_MatchEnableInt(LPC_TIMER32_1, 1);
    Chip_TIMER_SetMatch(LPC_TIMER32_1, 1, (timerFreq / TICKRATE_HZ2));   //ÿ�������TICKRATE_32_1��
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_1, 1);
    Chip_TIMER_Enable(LPC_TIMER32_1);

    /* Enable timer interrupt */
    NVIC_ClearPendingIRQ(TIMER_32_1_IRQn);
    NVIC_EnableIRQ(TIMER_32_1_IRQn);
}

/*****************************************************************************
 * ÿ10ms�������
 * ���ڽ���ָ��ʱ�յ���һ���ַ���ֻ��10ms����ȡ��
 * ��ʱ���㳬���ʵ�ʱ�䣬3min����һ�Σ�
 ****************************************************************************/
unsigned short Ufratecount=0;
extern unsigned char ufrateflag;
void TIMER32_1_IRQHandler(void)
{
    if(Chip_TIMER_MatchPending(LPC_TIMER32_1, 1)) 
    {
        Chip_TIMER_ClearMatch(LPC_TIMER32_1, 1);
        
        if(StateFlag.MainState == STATE_THERAPY)
        {
            Ufratecount++;
            
            if(Ufratecount==18000)
            {
                Ufratecount=0;
                ufrateflag=1;
            }
        } 
    }
}

//end of the file
