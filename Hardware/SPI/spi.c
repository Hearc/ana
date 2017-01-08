/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          spi.c
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

#include "spi.h"
#include "board.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define LOOPBACK_TEST       1
#define SSP_MODE_TEST       1   /*1: Master, 0: Slave */
#define POLLING_MODE        1

#if POLLING_MODE
#define INTERRUPT_MODE      0
#else
#define INTERRUPT_MODE      1
#endif

#define BUFFER_SIZE                         (0x100)
#define SSP_DATA_BITS                       (SSP_BITS_8)
#define SSP_DATA_BIT_NUM(databits)          (databits + 1)
#define SSP_DATA_BYTES(databits)            (((databits) > SSP_BITS_8) ? 2 : 1)
#define SSP_LO_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? 0xFF : (0xFF >> \
                                                  (8 - SSP_DATA_BIT_NUM(databits))))
#define SSP_HI_BYTE_MSK(databits)           ((SSP_DATA_BYTES(databits) > 1) ? (0xFF >> \
                                                  (16 - SSP_DATA_BIT_NUM(databits))) : 0)
#define LPC_SSP           LPC_SSP0
#define SSP_IRQ           SSP0_IRQn
#define SSPIRQHANDLER     SSP0_IRQHandler

static SSP_ConfigFormat ssp_format;
//static Chip_SSP_DATA_SETUP_T xf_setup;
static volatile uint8_t  isXferCompleted = 0;
/* Set pin mux for SSP operation */

static void Init_SSP_PinMux(void)
{
#if (defined(BOARD_NXP_XPRESSO_11U14) || defined(BOARD_NGX_BLUEBOARD_11U24))
    /* Only SSP0 is supported */
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 2, (IOCON_FUNC1 | IOCON_MODE_PULLUP));   /* SSEL0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 29, (IOCON_FUNC1 | IOCON_MODE_PULLUP));  /* SCK0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8, (IOCON_FUNC1 | IOCON_MODE_PULLUP));   /* MISO0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 9, (IOCON_FUNC1 | IOCON_MODE_PULLUP));   /* MOSI0 */
#elif defined(BOARD_NXP_XPRESSO_11C24)               //  
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));    /* MISO0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));    /* MOSI0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO2_11, (IOCON_FUNC1 | IOCON_MODE_INACT));   /* SCK0 */
    Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO2_11);
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_2, (IOCON_FUNC0 | IOCON_MODE_INACT));    /* SSEL0 */
#elif defined(BOARD_MCORE48_1125)
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_8, (IOCON_FUNC1 | IOCON_MODE_INACT));    /* MISO0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_9, (IOCON_FUNC1 | IOCON_MODE_INACT));    /* MOSI0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_2, (IOCON_FUNC1 | IOCON_MODE_INACT));    /* SSEL0 */
    Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO0_6, (IOCON_FUNC2 | IOCON_MODE_INACT));    /* SCK0 */
    Chip_IOCON_PinLocSel(LPC_IOCON, IOCON_SCKLOC_PIO0_6);
#else
    #error "SSP Pin muxing not configured"
#endif
}

#if INTERRUPT_MODE
/**
 * @brief	SSP interrupt handler sub-routine
 * @return	Nothing
 */
void SSPIRQHANDLER(void)
{
    Chip_SSP_Int_Disable(LPC_SSP);	/* Disable all interrupt */
    if (SSP_DATA_BYTES(ssp_format.bits) == 1) {
        Chip_SSP_Int_RWFrames8Bits(LPC_SSP, &xf_setup);
    }
    else {
        Chip_SSP_Int_RWFrames16Bits(LPC_SSP, &xf_setup);
    }

    if ((xf_setup.rx_cnt != xf_setup.length) || (xf_setup.tx_cnt != xf_setup.length)) {
        Chip_SSP_Int_Enable(LPC_SSP);   /* enable all interrupts */
    }
    else {
        isXferCompleted = 1;
    }
}
#endif /*INTERRUPT_MODE*/

/*****************************************************************************
 * SPI初始化
 * 模式3
 ****************************************************************************/
void Spi_init(void)
{
    /* SSP initialization */
    Init_SSP_PinMux();
    Chip_SSP_Init(LPC_SSP);

    ssp_format.frameFormat = SSP_FRAMEFORMAT_SPI;
    ssp_format.bits = SSP_DATA_BITS;
    ssp_format.clockMode = SSP_CLOCK_MODE3;
    Chip_SSP_SetFormat(LPC_SSP, ssp_format.bits, ssp_format.frameFormat, ssp_format.clockMode);
    Chip_SSP_SetMaster(LPC_SSP, SSP_MODE_TEST);
    Chip_SSP_Enable(LPC_SSP);
}

uint8_t SPI_ReadWriteByte(uint8_t TxData)
{
    uint8_t retry=0;
    while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_TFE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        retry++;
        if(retry > 200)
        {
            return 0;
        }
    }
    Chip_SSP_SendFrame(LPC_SSP0, TxData); //通过外设SPIx发送一个数据
    retry=0;
    while (Chip_SSP_GetStatus(LPC_SSP0, SSP_STAT_RNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        retry++;
        if(retry > 200)
        {
            return 0;
        }
    }
    return Chip_SSP_ReceiveFrame(LPC_SSP0);                    //返回通过SPIx最近接收的数据	   
 }

//end of the file
