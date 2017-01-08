/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          uart.c
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

#include "uart.h"
#include "analog.h"
#include "Glb.h"

/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 128   /* Send */
#define UART_RRB_SIZE 32    /* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

//�յ��������͵��Լ��ʶ����һ���Լ���ɣ�ReceiveSelfCheck��1.���Կ�����ʱ����3S�󣬿�ʼ�ڶ����Լ죻
unsigned char ReceiveSelfCheck=0;       
//����У׼ʱ�����ؼ�����ʱ�յ��������͵�ָ��ʱCalibrationFor0kg��1���յ�����1KG������͵�ָ��RECalibration��1����������е�����ֵ��CalibrationFor1kg��1��
unsigned char CalibrationFor0kg=0,    CalibrationFor1kg=0,   RECalibration=0;
//���ݷ��ͻ����� ,�ο����ݸ�ʽ
uint8_t UartSendBuffer[8];
////�ϵ�����ʱ��
unsigned int  TotalTimeAPU=0;
//Ӳ��������汾��;
unsigned char H_version=0x01,S_version=0x01 ;
//���ؼƲ��������趨
int Weigh_Sensitivity_Set;

extern unsigned char Flashaddress[120];
extern unsigned short SelfTestResult;
extern DevParameter_t DevPar;
extern YaLi_t YaLiPRESS_COEFF,YaLiOffset,YaLi;
extern unsigned char BlSpeed, UfSpeed;
extern long UFRateStartWeight;
extern STATE_t  StateFlag;  
extern unsigned char WeightSelttestFirst;
/*****************************************************************************
 * ��ʼ���������ţ�
 * ʹ��PIO1_6��PIO1_7��
 ****************************************************************************/
static void Init_UART_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 18, IOCON_FUNC1 | IOCON_MODE_INACT);	/* PIO0_18 used for RXD */
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 19, IOCON_FUNC1 | IOCON_MODE_INACT);	/* PIO0_19 used for TXD */
#elif (defined(BOARD_NXP_XPRESSO_11C24) || defined(BOARD_MCORE48_1125))
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT));/* RXD */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT));/* TXD */
#else
#error "No Pin muxing defined for UART operation"
#endif
}

/*****************************************************************************
 * �����ж�
 * ֻʹ���˽����ж�
 * ָ���ʽΪ8���ֽڣ��ο�<ģ���ͨѶЭ��>
 * ��϶�ʱ������ֹ�ֽڸ��������Ĵ���
 ****************************************************************************/
void UART_IRQHandler(void)
{
    /* Want to handle any errors? Do it here. */

    /* Use default ring buffer handler. Override this with your own
        code if you need more capability. */
    Chip_UART_IRQRBHandler(LPC_USART, &rxring, &txring);
}

/*****************************************************************************
 * ���ڳ�ʼ��
 * Setup UART for 115.2K8N1
 ****************************************************************************/
void uart_init()
{
    Init_UART_PinMux();

    /* Setup UART for 115.2K8N1 */
    Chip_UART_Init(LPC_USART);
    Chip_UART_SetBaud(LPC_USART, 115200);
    Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));
    Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
    Chip_UART_TXEnable(LPC_USART);

    /* Before using the ring buffers, initialize them using the ring
        buffer init function */
    RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
    RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

    /* Enable receive data and line status interrupt */
    Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART0_IRQn, 1);
    NVIC_EnableIRQ(UART0_IRQn); 
}

/*****************************************************************************
 * ִ����λ�����͵�ָ�
 * ͨѶЭ��ο�����������������ģ��沿�֣�
 ****************************************************************************/
void uart_cmd(void)
{
    uint8_t i,temp;
    uint8_t bytes = 0;
    
    signed short tempsetting;
    //
    bytes = Chip_UART_ReadRB(LPC_USART, &rxring, UartReceiveBuf, 8);
    
    if(bytes > 7)
    {
        UartSendBuffer[0]=UartReceiveBuf[0]+0x60;
        UartSendBuffer[1]=UartReceiveBuf[1];
        StateFlag.MainState=UartReceiveBuf[1];                              //����˿�����״̬
        
        //�յ�"0x50"ʱ��������Ӳ���汾�ţ��ϵ�ʱ��        
        if(UartReceiveBuf[0]==0x50)
        {
            UartSendBuffer[2]=H_version;
            UartSendBuffer[3]=S_version;
            UartSendBuffer[4]=(TotalTimeAPU>>24)&0xff;
            UartSendBuffer[5]=(TotalTimeAPU>>16)&0xff;
            UartSendBuffer[6]=(TotalTimeAPU>>8)&0xff;
            UartSendBuffer[7]=(TotalTimeAPU)&0xff;
            for(i=0;i<8;i++)
            {
                Chip_UART_SendByte(LPC_USART,UartSendBuffer[i]);
                myDelay(2);
            }
        }
        //�յ�"0x51~~0x5c"��ĳ��ָ��ֵʱ,���ʹ洢�Ĳ���ֵ��
        else if((UartReceiveBuf[0]>=0x51)&&(UartReceiveBuf[0]<=0x5c))
        {
            temp=0x0f&UartReceiveBuf[0];
            for(i=2;i<=7;i++)
            {
                UartSendBuffer[i]=Flashaddress[8*temp+i]; 
            }
            for(i=0;i<8;i++)
            {
                Chip_UART_SendByte(LPC_USART,UartSendBuffer[i]);
                myDelay(2);
            }
        }
        //�յ�"0x60"ʱ������״̬�ı䣬ָ�����һλΪִ��ĳ����
        if(UartReceiveBuf[0]==0x60)
        {
            if(UartReceiveBuf[1]==0x02)                       //�յ�ϵͳ�Լ�
            {
                if(UartReceiveBuf[7]==0x01)                   //����01ʱ�������л����Լ���棬Ϊ01ʱ��ִ���Լ죻
                {
                    WeightSelttestFirst=1;                    //������һ���Լ��־
                    SelfTestResult=0x0001;                    //��¼�˿̵��Լ�״̬���ο�ͨ��Э��
                }
            }
            else if(UartReceiveBuf[1]==0x10)                  //�յ�����У׼
            {
                if(UartReceiveBuf[7]==0x01)                   //ж����Կ�ף����ؼ�����
                {
                    CalibrationFor0kg=1;
                } 
                else if(UartReceiveBuf[7]==0x02)              //����1kg����
                {
                    RECalibration=0x01;
                }
            }
            else if(UartReceiveBuf[1]==0x06)                  //�յ�"0x06",
            {
                UFRateStartWeight=YaLi.Weight;                //��������ʱ��ʼ���㳬����
            }
        }
        //�յ�"0x61~~0x6c"��ĳ��ָ��ֵʱ,�޸�EEPROM�д洢�Ĳ���ֵ
        else if((UartReceiveBuf[0]>=0x61)&&(UartReceiveBuf[0]<=0x6c))
        {
            temp=0x0f&UartReceiveBuf[0];
            for(i=2;i<=7;i++)
            {
                Flashaddress[8*temp+i]=UartReceiveBuf[i];
            }
            //��������
            switch (UartReceiveBuf[0])
            {
                case 0x61:
                    DevPar.ScaleErr = (UartReceiveBuf[4]<<24)+(UartReceiveBuf[5]<<16)+(UartReceiveBuf[6]<<8)+UartReceiveBuf[7];
                    break;
                      
                case 0x66:
                    tempsetting=(UartReceiveBuf[2]<<8)+(UartReceiveBuf[3]);
                    if ((tempsetting >= MaxArterial_min)&&(tempsetting <= MaxArterial_max))
                    {
                        DevPar.MaxArterialPress=tempsetting;
                    }
                      
                    tempsetting=(UartReceiveBuf[4]<<8)+(UartReceiveBuf[5]);
                    if ((tempsetting >= MaxVenous_min)&&(tempsetting <= MaxVenous_max))
                    {
                        DevPar.MaxVenousPress=tempsetting;
                    }
                      
                    tempsetting=(UartReceiveBuf[6]<<8)+(UartReceiveBuf[7]);
                    if ((tempsetting >= MaxTMP_min)&&(tempsetting <= MaxTMP_max))
                    {
                        DevPar.MaxTMP=tempsetting;
                    }
                    break;
                      
                case 0x67:
                    tempsetting=(UartReceiveBuf[2]<<8)+(UartReceiveBuf[3]);
                    if ((tempsetting >= MinArterial_min)&&(tempsetting <= MinArterial_max))
                    {
                         DevPar.MinArterialPress=tempsetting;
                    }
                      
                    tempsetting=(UartReceiveBuf[4]<<8)+(UartReceiveBuf[5]);
                    if ((tempsetting >= MinVenous_min)&&(tempsetting <= MinVenous_max))
                    {
                         DevPar.MinVenousPress=tempsetting;
                    }
                      
                    tempsetting=(UartReceiveBuf[6]<<8)+(UartReceiveBuf[7]);
                    if ((tempsetting >= MinUF_min)&&(tempsetting <= MinUF_max))
                    {
                         DevPar.MaxUFPress=tempsetting;
                    }
                    break;
                     
                case 0x68:
                    YaLiPRESS_COEFF.Arterial=(UartReceiveBuf[2]<<8)+(UartReceiveBuf[3]);
                    YaLiOffset.Arterial=(UartReceiveBuf[4]<<8)+(UartReceiveBuf[5]);
                    YaLiPRESS_COEFF.Venous=(UartReceiveBuf[6]<<8)+(UartReceiveBuf[7]);
                    break;
                      
                case 0x69:
                    YaLiOffset.Venous=(UartReceiveBuf[2]<<8)+(UartReceiveBuf[3]);
                    YaLiPRESS_COEFF.UF=(UartReceiveBuf[4]<<8)+(UartReceiveBuf[5]);
                    YaLiOffset.UF=(UartReceiveBuf[6]<<8)+(UartReceiveBuf[7]);
                    break;
                      
                case 0x6A:
                    YaLiPRESS_COEFF.PreFlt=(UartReceiveBuf[2]<<8)+(UartReceiveBuf[3]);
                    YaLiOffset.PreFlt=(UartReceiveBuf[4]<<8)+(UartReceiveBuf[5]);
                    YaLiPRESS_COEFF.Weight=(UartReceiveBuf[6]<<8)+(UartReceiveBuf[7]);
                    break;
                      
                case 0x6B:
                    YaLiOffset.Weight=(UartReceiveBuf[2]<<8)+(UartReceiveBuf[3]);
                    break;
                      
                case 0x6C:
                    UfSpeed=(UartReceiveBuf[2]<<8)+(UartReceiveBuf[3]);
                    DevPar.TargetPrimingVolume =   (UartReceiveBuf[4]<<8)+(UartReceiveBuf[5]);
                    DevPar.BagChangeVolume     =   (UartReceiveBuf[6]<<8)+(UartReceiveBuf[7]);     
                    break;
            }
        }
    }
}

//ms����ʱ
void myDelay (uint32_t ulTime)
{
    uint32_t i;

    i = 0;
    while (ulTime--) 
    {
        for (i = 0; i < 5000; i++);
    }
}

// END OF FILE
