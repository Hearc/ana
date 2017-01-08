/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          analog.c
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

#include "AD7794.h"
#include "analog.h"
#include "uart.h"
#include "Glb.h"

//读取AD7794时返回量
signed char ReadAdReturn;
// 计算称重计校准值;
volatile long WeightCoefficientX1;
//称重计第一次自检，称重计第二次自检，两次自检后比较自检结果
unsigned char WeightSelttestFirst=0,WeightSelttestSecond=0,StartToTest=0;
// 各项自检结果
unsigned char TestResult[13] = {0};
// 自检的各位保存在如下，
unsigned short SelfTestResult=0xffff;
// 称重稳定参数
unsigned long DiffWeightBuf = 0, DiffWeightBufLast = 0;
unsigned long DiffWeightADBuf = 0, DiffWeightADBufLast = 0;
         long MeasureUFRateEndTick = 0;  
// 执行结束预冲操作时不检测预冲结束；
unsigned char EndPrimeFlig = FALSE; 
//每几秒钟更新一次超滤；
unsigned char UpdateUfStartFlag=0,UpdateUfEndFlag=0;
// 更新超滤量后计算超滤率
volatile unsigned char gUfVolUpdata = 0;  
// 重新计算超滤率
volatile unsigned char gUfVolReset  = 0;  

// 压力(称重)偏移值，Y=ax+b中的b;
YaLi_t	YaLiOffset;  
// 压力值
YaLi_t	YaLi, YaLi_tmp, YaLi_last; 
// 使用的分钟数
UsedMinutes_t  UsedMinute;

extern YaLi_t  YaLiPRESS_COEFF;
extern unsigned ReceiveSelfCheck;
extern ERR_t    ErrFlag;
// 设备参数临时值
extern DevParameter_t DevPar_tmp;
extern DevParameter_t DevPar;
//记录各个状态
extern STATE_t   StateFlag;

extern unsigned char UartSendBuffer[8];
extern unsigned char Flashaddress[120];
extern unsigned long ANA_ERR1;
extern unsigned char CalibrationFor0kg, CalibrationFor1kg, RECalibration;
extern unsigned long SetUFRate,ActualUFRate;
extern volatile long AdData[6][3];

//计算数组的数的平均值
signed long CalcAverage(signed long * buf, unsigned char num)
{
    signed long sum = 0;
    unsigned char i = 0;
    
    for(i=0;i<num;i++)
    {
        sum += buf[i];
    }
    
    return (sum / num);
}

/*! \brief      求数组buf中 Num个数的最大值与最小值之差
 *
 *  \param      buf    待求数组
 *  \param      num    数组个数
 *  \return     最大值与最小值之差
 */
signed long DiffOfData(signed long * buf, unsigned char num)
{
    signed long min = buf[0];
    signed long max = buf[0];
    unsigned char i = 1;
    for(i=1;i<num;i++)
    {
        if(buf[i] > max)
        {
            max = buf[i];
        }
        else if(buf[i] < min)
        {
            min = buf[i];
        }
    }
    
    return (max-min);
}

/*****************************************************************************
 * 系统自检
 * 自检两次，收到上位机自检指令自检第一次，3S后自检第二次；
 * 比较自检结果，结果写入到SelfTestResult；
 ****************************************************************************/
unsigned char AnyErrPress   = FALSE;
unsigned char ErrTemp;
    
YaLi_t YaLiOffset1, YaLiOffset2;
    
long YaLiOffsetMax;
long YaLiOffsetMin;
void HardwareSelfTest(void)
{

     YaLiOffsetMax =   8;
     YaLiOffsetMin =   -8;
    
    // 第一次自检
    if((StateFlag.MainState == STATE_SELFTEST)&&(WeightSelttestFirst==1))
    {
        WeightSelttestFirst=0;
        YaLiOffset1.Arterial = (((CalcAverage((signed long *)AdData[Channel_Artery], 3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);
                                      
        YaLiOffset1.Venous =   (((CalcAverage((signed long *)AdData[Channel_Vein],   3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);
            
        YaLiOffset1.UF =       (((CalcAverage((signed long *)AdData[Channel_UF],     3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);
        
        YaLiOffset1.PreFlt =   (((CalcAverage((signed long *)AdData[Channel_PreFlt], 3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);    
        
        //for 5kg
        YaLiOffset1.Weight =   (((CalcAverage((signed long *)AdData[Channel_Weighting], 3) >>6) * WeightCoefficient) / 100000)       \
                                - ((((8388608)>>6) * WeightCoefficient) / 100000);
        
        WeightCoefficientX1 = CalcAverage((signed long *)AdData[Channel_Weighting], 3) ;
        ReceiveSelfCheck=1;
    }
    
    // 第二次自检
    if(WeightSelttestSecond==1)
    {
        WeightSelttestSecond=0;
        YaLiOffset2.Arterial = (((CalcAverage((signed long *)AdData[Channel_Artery], 3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);                              
       
        YaLiOffset2.Venous =   (((CalcAverage((signed long *)AdData[Channel_Vein],   3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);
        
        YaLiOffset2.UF =       (((CalcAverage((signed long *)AdData[Channel_UF],     3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);
        
        YaLiOffset2.PreFlt =   (((CalcAverage((signed long *)AdData[Channel_PreFlt], 3) >>8) * 21795) / 1000000)       \
                                - ((((8388608)>>8) * 21795) / 1000000);
        //
        YaLiOffset2.Weight =   (((CalcAverage((signed long *)AdData[Channel_Weighting], 3) >>6) * WeightCoefficient) / 100000)       \
                                - ((((8388608)>>6) * WeightCoefficient) / 100000);
        WeightCoefficientX1 = CalcAverage((signed long *)AdData[Channel_Weighting], 3);
        StartToTest=1;
    }
    
    // 比较两次自检的结果
    if(StartToTest==1)
    {
        StartToTest=0;
        if(YaLiOffset2.Arterial == YaLiOffset1.Arterial)
        {
            YaLiOffset.Arterial = YaLiOffset2.Arterial;
            Flashaddress[8*8+4]=(YaLiOffset.Arterial>>8)&0xff;      Flashaddress[8*8+5]=(YaLiOffset.Arterial)&0xff;
            ErrTemp =  ((YaLiOffset.Arterial > YaLiOffsetMin)&&(YaLiOffset.Arterial < YaLiOffsetMax)) ? FALSE : TRUE;        
        }
        else
        {
            ErrTemp = TRUE;
        }
        AnyErrPress = AnyErrPress||ErrTemp;
        TestResult[1] = ErrTemp;
        SelfTestResult |=ErrTemp<<2;
           
        if(YaLiOffset2.Venous == YaLiOffset1.Venous)
        {
            YaLiOffset.Venous = YaLiOffset2.Venous;
            Flashaddress[8*9+2]=(YaLiOffset.Venous>>8)&0xff;      Flashaddress[8*9+3]=(YaLiOffset.Venous)&0xff;
            ErrTemp = ((YaLiOffset.Venous > YaLiOffsetMin)&&(YaLiOffset.Venous < YaLiOffsetMax)) ? FALSE : TRUE;
        }
        else
        {
            ErrTemp = TRUE;
        }
        AnyErrPress = AnyErrPress||ErrTemp;
        TestResult[2] = ErrTemp;
        SelfTestResult |=ErrTemp<<3;
        
        if(YaLiOffset2.UF == YaLiOffset1.UF)
        {
            YaLiOffset.UF = YaLiOffset2.UF;
            Flashaddress[8*9+6]=(YaLiOffset.UF>>8)&0xff;        Flashaddress[8*9+7]=(YaLiOffset.UF)&0xff;
            ErrTemp = ((YaLiOffset.UF > YaLiOffsetMin)&&(YaLiOffset.UF < YaLiOffsetMax)) ? FALSE : TRUE;
        }
        else
        {
            ErrTemp = TRUE;
        }    
        AnyErrPress = AnyErrPress||ErrTemp;
        TestResult[3] = ErrTemp;
        SelfTestResult |=ErrTemp<<4;
        
        if(YaLiOffset2.PreFlt == YaLiOffset1.PreFlt)
        {
            YaLiOffset.PreFlt = YaLiOffset2.PreFlt;
            Flashaddress[8*10+4]=(YaLiOffset.PreFlt>>8)&0xff;      Flashaddress[8*10+5]=(YaLiOffset.PreFlt)&0xff;
            ErrTemp = ((YaLiOffset.PreFlt > YaLiOffsetMin)&&(YaLiOffset.PreFlt < YaLiOffsetMax)) ? FALSE : TRUE;
        }
        else
        {
            ErrTemp = TRUE;
        }    
        AnyErrPress = AnyErrPress||ErrTemp; 
        TestResult[4] = ErrTemp;
        SelfTestResult |=ErrTemp<<5;
        
        if((ABS_DIFF(YaLiOffset2.Weight, YaLiOffset1.Weight) < 10)&&(ABS_DIFF(YaLiOffset1.Weight, YaLiOffset.Weight)<WEIGHT_TEST_ERR))
        {
            YaLiOffset.Weight = YaLiOffset2.Weight;
            Flashaddress[8*11+2]=(YaLiOffset.Weight>>8)&0xff;      Flashaddress[8*11+3]=(YaLiOffset.Weight)&0xff;  
            ErrTemp = ((YaLiOffset2.Weight > -200)&&(YaLiOffset2.Weight < 200)) ? FALSE : TRUE;
        }
        else
        {
            ErrTemp = TRUE;
        }
        AnyErrPress = AnyErrPress||ErrTemp;
        TestResult[5] = ErrTemp;  
        SelfTestResult |=ErrTemp<<6;  
         
        if(ErrTemp==TRUE)
        {
             SelfTestResult &=0xfc;
             SelfTestResult |=0x02;
        } 
        else
        {SelfTestResult=0;}
        //  AnyErrSelfTest = AnyErrSelfTest || AnyErrPress;  
    }
}

/*
********************************************************************************
* File name   : static long GetUFVolumeInterval(unsigned char UFMotorSpeed)
* Usage       : get total UF volume update ticks interval based on UF speed. 200 ticks = 1 second
* Description :  
*             :  
* Programmer  : FENG XQ
* Version     : v1.0    
* Created Date: 2010-05-15	  
*                                      
********************************************************************************
*/
static long GetUFVolumeInterval(unsigned char UFMotorSpeed)
{
    //     VufDisTab[14] = {0,50,100,150,200,250,300,350,400,450,500,550,600, 0}
    //200 ticks = 1 sec,    0  1  2   3   4   5   6   7   8   9  10  11  12  13
    
    long interval;
    
    switch (UFMotorSpeed)
    {
        case 1:                //// rate 50ml/h, 50ml/3600 sec
            interval= 43200;   //// UFVolume should be 3 ml in 216 sec
            break;	
        case 2:                //// rate 100ml/h, 100ml/3600 sec
            interval= 21600;   //// UFVolume should be 3 ml in 108 sec
            break;	
        case 3:                //// rate 150ml/h, 150ml/3600 sec
            interval= 24000;   //// UFVolume should be 5 ml in 120 sec
            break;
        case 4:                //// rate 200ml/h, 200ml/3600 sec
            interval= 18000;   //// UFVolume should be 5 ml in 90  sec
            break;
        case 5:                //// rate 250ml/h, 250ml/3600 sec
            interval= 14400;   //// UFVolume should be 5 ml in 72 sec
            break;
        case 6:                //// rate 300ml/h, 300ml/3600 sec
            interval= 12000;   //// UFVolume should be 5 ml in 60 sec
            break;
        case 7:                //// rate 350ml/h, 350ml/3600 sec
            interval= 10286;   //// UFVolume should be 5 ml in 51.43 sec
            break;
        case 8:                //// rate 400ml/h, 400ml/3600 sec
            interval= 9000;    //// UFVolume should be 5 ml in 45 sec
            break;
        case 9:                //// rate 450ml/h, 450ml/3600 sec
            interval= 8000;    //// UFVolume should be 5 ml in 40 sec
            break;
        case 10:               //// rate 500ml/h, 500ml/3600 sec
            interval= 7200;    //// UFVolume should be 5 ml in 36 sec
            break;
        case 11:               //// rate 550ml/h, 550ml/3600 sec
            interval= 6546;    //// UFVolume should be 5 ml in 32.73 sec
            break;
        case 12:               //// rate 600ml/h, 600ml/3600 sec
            interval= 6000;    //// UFVolume should be 5 ml in 30 sec
            break;
        default:
            interval= 0;
            break;
    }
    return  interval;
}
/*****************************************************************************
 * 判断各个压力是不是在设定的范围内
 * 记录称重计不稳状态，预冲结束标识，是否需要换袋，超滤率是否异常
 * 
 ****************************************************************************/
unsigned long ANA_ERR_CHECK(void)
{
    unsigned long temp=0;
//    if(YaLi.Arterial > Artery_max)    //动脉压大于动脉压最高限
//    {
//         temp |=1<<0;
//    }
//    if(YaLi.Arterial < Artery_min)    //动脉压小于动脉压最低限
//    {
//         temp |=1<<1;
//    }
//    if(YaLi.Venous > Vein_max)        //静脉压大于静脉压最高限
//    {
//         temp |=1<<2;
//    }
//    if(YaLi.Venous < Vein_min)         //静脉压小于静脉压最低限
//    {
//         temp |=1<<3;
//    }
//    if(YaLi.UF > UF_max)               //超滤压大于超滤压最高限        
//    {
//         temp |=1<<4;
//    }
//    if(YaLi.UF < UF_min)               //超滤压小于超滤压最低限   
//    {
//         temp |=1<<5;
//    }
//     if(YaLi.PreFlt >Preflt_max)       //滤器前压大于滤器前压最高限  
//    {
//         temp |=1<<6;
//    }
//    if(YaLi.Venous <Preflt_min)        //滤器前压小于滤器前压最低限
//    {
//         temp |=1<<7;
//    }
//    if(YaLi.Weight >UFBagWeight_max)   //称重超过最高限
//    {
//         temp |=1<<8;
//    }
//    if(YaLi.Weight <UFBagWeight_max)   //称重小于最低限
//    {
//         temp |=1<<9;
//    }
     if(YaLi.Arterial >DevPar.MaxArterialPress*Max_Arterial_Step)   //动脉压超过设定高限
    {
         temp |=1<<10;
    }
    else 
    {
         temp &=~(1<<10);
    }
    
    if(YaLi.Arterial <DevPar.MinArterialPress*Min_Arterial_Step)    //动脉压超过设定低限
    {
         temp |=1<<11;
    }
    else 
    {
         temp &=~(1<<11);
    }
    
    if(YaLi.Venous >DevPar.MaxVenousPress*Max_Venous_Step)          //静脉压超过设定高限
    {
         temp |=1<<12;
    }
    else 
    {
         temp &=~(1<<12);
    }
    
    if(YaLi.Venous <DevPar.MinVenousPress*Min_Venous_Step)          //静脉压超过设定低限       
    {
         temp |=1<<13;
    }
     else 
    {
         temp &=~(1<<13);
    }
    
    if(YaLi.UF <DevPar.MinUFPress*Min_UFPress_Step)                 //超滤压超过设定低限
    {
         temp |=1<<15;
    }
     else 
    {
         temp &=~(1<<15);
    }
    
    if(YaLi.TMP <DevPar.MinTMP*Max_TMP_Step)                        //跨模压超过设定低限
    {
         temp |=1<<18;
    }  
     else 
    {
         temp &=~(1<<18);
    }
    
     if(ErrFlag.Balance == TRUE)                                    //需要换袋
    {
         temp |=1<<19;
    }
     else 
    {
         temp &=~(1<<19);
    }

    if(StateFlag.BagChange == TRUE)                                 //需要换袋
    {
         temp |=1<<21;
    }
     else 
    {
         temp &=~(1<<21);
    }
    
    if(ErrFlag.EndPrime == TRUE)
    {
         temp |=1<<20;
    }
     else 
    {
         temp &=~(1<<20);
    }
    return temp;
}


unsigned char LastUfSpeed = 0;  
static unsigned char PrimingEndCnt = 0,  BagChangeCnt = 0, NotBagChangeCnt = 0;
static unsigned char WeightErrCnt = 0, WeightOkCnt = 0;
signed short WeightTmp = 0;      // change raw value to weight (g)
signed long  WeightBuf[8];       // raw value from AD7794
    
long TotalVolumeIncreaseInterval;
long TotalVolumeIncreaseStartTicks;
long BagChangeStartTicks;                       // beginning ticks before bag change alarm enabled

long Reset7794Ticks;
    
signed char ReadAdReturn = 0;
    
signed short LastWeight    = 0;  
signed short Currentweight = 0; 
unsigned char i = 0;
unsigned char calibrationtimes = 0;


 /*! \brief      压力检测总超率量计算
 *
 *  \param      None
 *  \return     None
 */   
void vTaskPress(void )
{
    if(RECalibration==1)
    {
         calibrationtimes++;
         if(calibrationtimes>=30)
         {
              calibrationtimes=0;
              RECalibration=0;
              CalibrationFor1kg=1;
         }
    }
    // read AD7794
    myDelay(10);
    ReadAdReturn = ReadAd(); 
    if(ReadAdReturn > 0) // ????   
    {
        switch(ReadAdReturn)
        {
            case Channel_Weighting:
                WeightBuf[i] = (((CalcAverage((signed long *)AdData[Channel_Weighting], 3) >>6) * YaLiPRESS_COEFF.Weight) / 100000)       \
                        - ((((8388608)>>6) * YaLiPRESS_COEFF.Weight) / 100000) - YaLiOffset.Weight;    
                 
                i++;
                if (i > 7) 
                { 
                    i = 0; 
                }
                DiffWeightBufLast = DiffWeightBuf;
                DiffWeightADBufLast = DiffWeightADBuf;
                DiffWeightADBuf = DiffOfData((signed long *)AdData[Channel_Weighting], 3);        // AD值的最大与最小之差
                DiffWeightBuf = DiffOfData((signed long *)WeightBuf, 8);                          // 数组中最大值与最小值之差                
                YaLi_tmp.Weight = CalcAverage(WeightBuf, 8);                                      // 计算重力平均值

                if(ABS_DIFF(YaLi_last.Weight, YaLi_tmp.Weight) < 50)
                {
                    YaLi.Weight = YaLi_tmp.Weight;
                    // 判断称重计是否平稳
                    if ((StateFlag.MainState >= STATE_PRIMING)&&(StateFlag.MainState <= STATE_WEIGHING))        //在05-10状态下判断
                    {
                        //if(((YaLi.Weight>WeightTmp)&&((YaLi.Weight-WeightTmp)>DevPar.ScaleErr))||((YaLi.Weight<WeightTmp)&&((WeightTmp-YaLi.Weight)>DevPar.ScaleErr)))   ////modified by FENG   DevPar.ScaleErr
                        if((DiffWeightBuf>DiffWeight)||(DiffWeightADBuf>DiffWeightAD))
                        {   
                            WeightErrCnt++;   
                        }
                        else
                        {  
                            WeightOkCnt++;
                        }

                        if(WeightErrCnt > 12)
                        {
                            ErrFlag.Balance = TRUE;   
                            WeightErrCnt = 0;        
                            WeightOkCnt  = 0;
                        }
                        if(WeightOkCnt > 3)
                       // if(WeightOkCnt > 16)
                        {
                            ErrFlag.Balance = FALSE;
                            WeightErrCnt =0;
                            WeightOkCnt  = 0;
                        }
                    }
    
                    WeightTmp = YaLi.Weight;
                }
                else
                {
                    YaLi_last.Weight = YaLi_tmp.Weight;
                }
                break;
                    
            case Channel_UF:
                YaLi.UF = -((((CalcAverage((signed long *)AdData[Channel_UF], 3) >>8) * YaLiPRESS_COEFF.UF) / 1000000) - ((((8388608)>>8) * YaLiPRESS_COEFF.UF) / 1000000) \
                    - YaLiOffset.UF       - YaLi.UF)       / 2;
                break;
                   
            case Channel_PreFlt:
                YaLi.PreFlt = ((((CalcAverage((signed long *)AdData[Channel_PreFlt], 3) >>8) * YaLiPRESS_COEFF.PreFlt) / 1000000) - ((((8388608)>>8) * YaLiPRESS_COEFF.PreFlt) / 1000000) \
                    - YaLiOffset.PreFlt   + YaLi.PreFlt)   / 2;
                break;
            case Channel_Vein:    
                YaLi.Venous =  ((((CalcAverage((signed long *)AdData[Channel_Vein], 3) >>8) * YaLiPRESS_COEFF.Venous) / 1000000) - ((((8388608)>>8) * YaLiPRESS_COEFF.Venous) / 1000000) \
                    - YaLiOffset.Venous + YaLi.Venous)   / 2;
                break;
                    
            case Channel_Artery:
                YaLi.Arterial   = -((((CalcAverage((signed long *)AdData[Channel_Artery], 3) >>8) * YaLiPRESS_COEFF.Arterial) / 1000000) - ((((8388608)>>8) * YaLiPRESS_COEFF.Arterial) / 1000000) \
                    - YaLiOffset.Arterial - YaLi.Arterial) / 2;
                break;
            default:
                break;
        }

        // 计算跨模压
        YaLi.TMP = ((YaLi.PreFlt + YaLi.Venous)/2) - YaLi.UF;
          
        //  更新总超滤量，根据当前超滤速度计算总超滤率更新时间间隔更改超滤速度或启停超滤泵时 重设参数
        if (((LastUfSpeed!=DevPar_tmp.TherapyUFSpeed)||(FALSE == StateFlag.UfMotorRun)) && (StateFlag.MainState == STATE_THERAPY))
        {                
            LastUfSpeed                   = DevPar_tmp.TherapyUFSpeed;
            TotalVolumeIncreaseInterval   = GetUFVolumeInterval(LastUfSpeed);
        }
//      if ((StateFlag.MainState == STATE_THERAPY)&&(TRUE == StateFlag.UfMotorRun)&&(LastUfSpeed)&&(DiffWeightADBuf < WEIGHT_STABLE_BUF)&&(DiffWeightBuf < WEIGHT_STABLE_VAL)) 
        if ((StateFlag.MainState == STATE_THERAPY)&&(DiffWeightADBuf < WEIGHT_STABLE_BUF)&&(DiffWeightBuf < WEIGHT_STABLE_VAL)) 
        {
            UpdateUfStartFlag=1;
            if (UpdateUfEndFlag==1)
            {
                UpdateUfEndFlag=0;
                LastWeight    = Currentweight;
                Currentweight = YaLi.Weight;
                // TotalVolumeIncreaseStartTicks = xTaskGetTickCount();

                if((Currentweight >= LastWeight)&&((Currentweight - LastWeight)<20))
                {
                    UsedMinute.TotalUFVolume  += (Currentweight - LastWeight);
                    gUfVolUpdata = TRUE;
                }
                else  //重新计算超滤率
                {
                    gUfVolReset = TRUE; 
                    gUfVolUpdata = FALSE;
                }                   
            }             
        }
            
        // 判断是否需要换袋
//            if (! StateFlag.BagChangeDelay)
//            {  
//            //    BagChangeStartTicks = xTaskGetTickCount();  
//            }  
//            else
//            {  
//             //   StateFlag.BagChangeDelay = !(DelayUntil(BagChangeStartTicks, BAG_CHANGE_TICKS)); 
//            }            

        if(((StateFlag.MainState == STATE_THERAPY)||(StateFlag.MainState == STATE_THERAPY))&&(!ErrFlag.Balance))           //&&(!StateFlag.BagChangeDelay))  
        {               
            if((YaLi.Weight) >= ((DevPar.BagChangeVolume * BagChangeVolume_Step) + 1))                
            {  
                BagChangeCnt ++;         
                NotBagChangeCnt = 0;
            }
            else
            {  
                NotBagChangeCnt ++;      
                BagChangeCnt = 0;
            }
            if(NotBagChangeCnt > 16)
            {
                NotBagChangeCnt = 0;
                StateFlag.BagChange = FALSE;
            }
                
            if(BagChangeCnt > 24)
            {
                BagChangeCnt    = 0;
                NotBagChangeCnt = 0;
                ErrFlag.BagChange   = TRUE;
                StateFlag.BagChange = TRUE;
            }
        }
        
        // 判断预冲是否结束
        if((StateFlag.MainState == STATE_PRIMING)&&(!ErrFlag.Balance))                   //&&(!EndPrimeFlig))
        {
            if(YaLi.Weight >= (DevPar.TargetPrimingVolume * TargetPrimingVolume_Step) + 1)
            { 
                PrimingEndCnt++;
            }
            else
            {
                PrimingEndCnt--;
            }

            if(PrimingEndCnt > 24)
            {
                PrimingEndCnt = 24;
                ErrFlag.EndPrime = TRUE; 
                ErrFlag.YclBz = FALSE;
                // ANA_ERR1 |=1<<20;
            }
            if(PrimingEndCnt == 0)
            {
                PrimingEndCnt = 1;
                ErrFlag.EndPrime = FALSE;
                StateFlag.PrimingEndDelay = FALSE;
            }
        }
    }
    else  // AD7794读数据不正常
    {
        // 有误时复位AD7794
        if((ErrCntSwCh > 20)||(ErrCnt0F > 20)||(ErrCntReady > 20))
        {
            ErrCntHdret++;
            Reset_AD7794();
            ErrCnt0F = 0;
            ErrCntSwCh = 0;
            ErrCntReady = 0;
            //   vTaskDelay( 100 );
        }
        if(ErrCntHdret > 3)
        {
            Reset_AD7794(); 
            ErrCntHdret = 0;
            ErrCnt0F = 0;
            ErrCntSwCh = 0;
            ErrCntReady = 0;
        }
    } 
    ANA_ERR1=ANA_ERR_CHECK();
}

/*****************************************************************************
 * 称重校准，
 * 在称重计悬空状态下和悬挂1KG砝码两种状态下计算a,b;
 * 
 ****************************************************************************/
void Calibration(void)
{
    if(CalibrationFor0kg==1)
    {
        CalibrationFor0kg=0;
        YaLiOffset.Weight =   (((CalcAverage((signed long *)AdData[Channel_Weighting], 3) >>6) * WeightCoefficient) / 100000)       \
                                - ((((8388608)>>6) * WeightCoefficient) / 100000);
    }
    
    if(CalibrationFor1kg==1)
    {
        CalibrationFor1kg=0;
        YaLiPRESS_COEFF.Weight=  ((1000+YaLiOffset.Weight)*100000) /  ((CalcAverage((signed long *)AdData[Channel_Weighting], 3) >>6) - (8388608>>6));
    }
} 

long UFRateStartWeight = 0;
static long UFRateEndWeight = 0;
unsigned char ufrateflag;
/*****************************************************************************
 * 计算超滤率；
 * 每3min计算一次；
 * 
 ****************************************************************************/
void CheckUFRate(void)
{   
    //  实际超滤速度检测，超滤泵运转中，且超滤速度不变，超滤速度大于0，称重计稳定，治疗中，计算实际超滤速度；
    //  超滤量更新后计算
//    if((!gUfVolReset)&&(TRUE == StateFlag.UfMotorRun)&&(LastTherapyUFSpeed==DevPar_tmp.TherapyUFSpeed )&&(DevPar_tmp.TherapyUFSpeed) \                                     &&(MeasureUFRateTickInterval)&&(StateFlag.MainState == STATE_THERAPY))                     
    if(StateFlag.MainState == STATE_THERAPY)
    {   
        if(ufrateflag==1)
        {
            ufrateflag=0;
            UFRateEndWeight = YaLi.Weight;  
            if((DiffWeightADBuf < WEIGHT_STABLE_BUF)&&(DiffWeightBuf < WEIGHT_STABLE_VAL)
               &&(DiffWeightADBufLast < WEIGHT_STABLE_BUF)&&(DiffWeightBufLast < WEIGHT_STABLE_VAL)
               &&(UFRateEndWeight >= UFRateStartWeight))
            {
                // ErrFlag.UfRate = GetUFRateErr( VufDisTab[LastTherapyUFSpeed], (MeasureUFRateEndTick - MeasureUFRateStartTick), UFRateStartWeight, UFRateEndWeight);
                ActualUFRate= ( UFRateEndWeight - UFRateStartWeight)*60*60/180;               
                UFRateStartWeight = UFRateEndWeight;    
            }
        }
    }  
}

// end of the file
