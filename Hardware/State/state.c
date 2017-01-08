/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          state.c
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

#include "state.h"
#include "Glb.h"
#include "AD7794.h"

STATE_t  StateFlag;
ERR_t        ErrFlag;                         // err and alarm flag struct
DevParameter_t DevPar;
DevParameter_t DevPar_tmp;
//y=ax+b的a;
YaLi_t  YaLiPRESS_COEFF;       
// 血泵速度，超滤泵速度调整数值时的临时值
unsigned char BlSpeed, UfSpeed;
//存储参数的缓存
unsigned char Flashaddress[120]={0};
extern YaLi_t YaLiOffset;
unsigned long SetUFRate=0,ActualUFRate=0;
signed short    Artery_max=500,               Artery_min=-500,                             //动脉压最大值     最小值
                Vein_max=500,                 Vein_min=-500,                               //静脉压最大值     最小值
                UF_max=500,                   UF_min=-500,                                 //超滤压最大值     最小值   
                Preflt_max =500,              Preflt_min =-500,                            //滤器前压最大值   最小值 
                TMP_max=500,                  TMP_min=-500,                                //跨模压最大值     最小值
                UFBagWeight_max=2500,         UFBagWeight_min=-2500,                       //超滤袋重量最大值 最小值
                Artery_Set_High=150,          Artery_Set_Low=-200,                         //动脉压设定高限， 低限
                Vein_Set_High=200,            Vein_Set_Low=10,                             //静脉压设定高限   低限
                TMP_Set_High=400,                                                          //跨膜压设定高限   低限
                                              UF_Set_Low=-300,                             //超滤压设定高限   低限
                Artery_Set_Param_A=20924,     Artery_Set_Param_B=0,                        //动脉压设定参数  a  b
                Vein_Set_Param_A=20924,       Vein_Set_Param_B=0,                          //静脉压设定参数  a  b
                UF_Set_Param_A=20924,         UF_Set_Param_B=0,                            //超滤压设定参数  a  b
                Preflt_Set_Param_A=20924,     Preflt_Set_Param_B=0,                        //滤器前压设定参数  a  b
                Weighting_Set_Param_A=3730,   Weighting_Set_Param_B=0,                     //称重设定参数  a 
                UF_Speed_Set=0,                                                            //设定超滤速度
                Preflt_End_Set=100,                                                        //设定预冲结束量
                BagChange_weight_Set=2000;    
/*
********************************************************************************
* Functions name  : InitializeDeviceParameters
* Usage           : initialize flags, states, errs, parameters and other
* Description     : 
* Versioon        : V1.0
* Created By      : FENG XQ
* Created date    : 2010-05-13
* ******************************************************************************
*/                                           
unsigned char InitializeDeviceParameters(void)
{
//    long UsedNumbers;
//    unsigned char DeviceErr = FALSE;
//    unsigned char TempErr   = FALSE;
   
    // reset device state ///////////////////////////////////
    StateFlag.MainState     =STATE_NULL;
    StateFlag.Emergency     =FALSE;
    StateFlag.LcdRefreshing =FALSE;
    StateFlag.GUIChanged    =FALSE;
    StateFlag.KeyPressed    =FALSE;
    StateFlag.UfMotorRun    =FALSE;
    StateFlag.BldMotorRun   =FALSE;
    StateFlag.LedAlarm      =FALSE;
    StateFlag.BuzzAlarm     =FALSE;
    StateFlag.BuzzAlarmDelay  = FALSE;
    StateFlag.VenousLowDelay  = FALSE;
    StateFlag.BagChangeDelay  = FALSE;
    StateFlag.PrimingEndDelay = FALSE; 
    StateFlag.BagChange     =FALSE;    
    StateFlag.KeyReady      =FALSE;   
    StateFlag.Runing        =FALSE;
    // clear all err
    ErrFlag.PumpCover       =FALSE;
    ErrFlag.ClipNoOpen      =FALSE;
    ErrFlag.AirBubble       =FALSE;
    ErrFlag.BloodLeak       =FALSE;
    ErrFlag.ArterialPress   =FALSE;   
    ErrFlag.VenousPress     =FALSE;    
    ErrFlag.Tmp             =FALSE;
    ErrFlag.UfPress         =FALSE; 
    ErrFlag.PreArterialPress=FALSE;   
    ErrFlag.PreVenousPress  =FALSE;    
    ErrFlag.PreTmp          =FALSE;   
    ErrFlag.PreUfPress      =FALSE; 
    ErrFlag.Balance         =FALSE;
    ErrFlag.BagChange       =FALSE;    
    ErrFlag.EndPrime        =FALSE;
    ErrFlag.Motor           =FALSE;
    ErrFlag.BldStop         =FALSE;   
    ErrFlag.Picc            =FALSE;
    ErrFlag.Clip            =FALSE;
    ErrFlag.UfRate          =FALSE; 
   
    if(DevPar.PrimingUFSpeed > 12)      { DevPar.PrimingUFSpeed = 6;        }
   
    if(DevPar.TherapyUFSpeed > 12)      { DevPar.TherapyUFSpeed = 6;        }
   
    if(DevPar.PrimingBloodSpeed > 10)   { DevPar.PrimingBloodSpeed = 4;     }
   
    if(DevPar.TherapyBloodSpeed > 10)   { DevPar.TherapyBloodSpeed = 4;     }

    if((DevPar.TargetPrimingVolume > 20) || (DevPar.TargetPrimingVolume < 1))   { DevPar.TargetPrimingVolume = 5;  }
   
    if((DevPar.TargetUFVolume > 99) || (DevPar.TargetUFVolume < 1))             { DevPar.TargetUFVolume = 40;      }
   
    if((DevPar.ScaleErr > 20) || (DevPar.ScaleErr < 1))                         { DevPar.ScaleErr = 5;             }
   
    if((DevPar.BagChangeVolume > 7) || (DevPar.BagChangeVolume < 1))            { DevPar.BagChangeVolume = 4;      }

    
    // if((DevPar.MinArterialPress < MinArterial_min)|| (DevPar.MinArterialPress > MinArterial_max)) 
    {DevPar.MinArterialPress = DefaultMinArterialPress;}  //动脉压设定低限
    // if((DevPar.MaxArterialPress < MaxArterial_min)|| (DevPar.MaxArterialPress > MaxArterial_max))
    {DevPar.MaxArterialPress = DefaultMaxArterialPress;}  //动脉压设定高限 
    //if((DevPar.MinVenousPress   < MinVenous_min  )|| (DevPar.MinVenousPress   > MinVenous_max  ))
    {DevPar.MinVenousPress   = DefaultMinVenousPress;}    //
    //if((DevPar.MaxVenousPress   < MaxVenous_min  )|| (DevPar.MaxVenousPress   > MaxVenous_max  ))
    {DevPar.MaxVenousPress   = DefaultMaxVenousPress;}
    //if((DevPar.MinUFPress       < MinUF_min      )|| (DevPar.MinUFPress       > MinUF_max      ))
    {DevPar.MinUFPress       = DefaultMinUFPress;}        //超滤压设定低限
    //if((DevPar.MaxTMP           < MaxTMP_min     )|| (DevPar.MaxTMP           > MaxTMP_max     ))
    {DevPar.MaxTMP           = DefaultMaxTMP;}
                   
    DevPar.TargetPrimingVolume     = DefaultTargetPrimingVolume;      // 100ml
    DevPar.TargetPrimingVolumeMin  = DefaultTargetPrimingVolumeMin;   //  50ml
    DevPar.ScaleErr                = DefaultScaleErr;                 // err + - 5 g
    DevPar_tmp.PrimingUFSpeed      = DevPar.PrimingUFSpeed;
    DevPar_tmp.PrimingBloodSpeed   = DevPar.PrimingBloodSpeed;
    DevPar_tmp.TherapyUFSpeed      = DevPar.TherapyUFSpeed;
    DevPar_tmp.TherapyBloodSpeed   = DevPar.TherapyBloodSpeed;
    DevPar_tmp.TargetPrimingVolume = DevPar.TargetPrimingVolume;
    DevPar_tmp.TargetUFVolume      = DevPar.TargetUFVolume;
    DevPar_tmp.ScaleErr            = DevPar.ScaleErr;
    DevPar_tmp.BagChangeVolume     = DevPar.BagChangeVolume;

    DevPar_tmp.MinArterialPress    = DevPar.MinArterialPress;
    DevPar_tmp.MaxArterialPress    = DevPar.MaxArterialPress;
    DevPar_tmp.MinVenousPress      = DevPar.MinVenousPress;
    DevPar_tmp.MaxVenousPress      = DevPar.MaxVenousPress;
    DevPar_tmp.MinUFPress          = DevPar.MinUFPress;
    DevPar_tmp.MaxUFPress          = DevPar.MaxUFPress;
    DevPar_tmp.MinTMP              = DevPar.MinTMP;
    DevPar_tmp.MaxTMP              = DevPar.MaxTMP;
   
    BlSpeed = DevPar_tmp.PrimingBloodSpeed;
    UfSpeed = DevPar_tmp.PrimingUFSpeed;
    
    YaLiPRESS_COEFF.Arterial=PRESS_COEFF_DEF;
    YaLiPRESS_COEFF.PreFlt=PRESS_COEFF_DEF;
    YaLiPRESS_COEFF.UF=PRESS_COEFF_DEF;
    YaLiPRESS_COEFF.Venous=PRESS_COEFF_DEF;
    YaLiPRESS_COEFF.Weight=WeightCoefficient;
    
    //少写一个称重计不稳灵敏度
    
    Flashaddress[8*2+2]=(Artery_max>>8)&0xff;      
    Flashaddress[8*2+3]=(Artery_max)&0xff;   
    Flashaddress[8*2+4]=(Vein_max>>8)&0xff;      
    Flashaddress[8*2+5]=(Vein_max)&0xff;  
    Flashaddress[8*2+6]=(UF_max>>8)&0xff;          
    Flashaddress[8*2+7]=(UF_max)&0xff;
    
    Flashaddress[8*3+2]=(Preflt_max>>8)&0xff;      
    Flashaddress[8*3+3]=(Preflt_max)&0xff;   
    Flashaddress[8*3+4]=(TMP_max>>8)&0xff;      
    Flashaddress[8*3+5]=(TMP_max)&0xff;  
    Flashaddress[8*3+6]=(UFBagWeight_max>>8)&0xff;          
    Flashaddress[8*3+7]=(UFBagWeight_max)&0xff;
    
    Flashaddress[8*4+2]=(Artery_min>>8)&0xff;      
    Flashaddress[8*4+3]=(Artery_min)&0xff;   
    Flashaddress[8*4+4]=(Vein_min>>8)&0xff;      
    Flashaddress[8*4+5]=(Vein_min)&0xff;  
    Flashaddress[8*4+6]=(UF_min>>8)&0xff;          
    Flashaddress[8*4+7]=(UF_min)&0xff;
    
    Flashaddress[8*5+2]=(Preflt_min>>8)&0xff;      
    Flashaddress[8*5+3]=(Preflt_min)&0xff;   
    Flashaddress[8*5+4]=(TMP_min>>8)&0xff;      
    Flashaddress[8*5+5]=(TMP_min)&0xff;  
    Flashaddress[8*5+6]=(UFBagWeight_min>>8)&0xff;          
    Flashaddress[8*5+7]=(UFBagWeight_min)&0xff;
    
    Flashaddress[8*6+2]=(DevPar.MaxArterialPress>>8)&0xff;      
    Flashaddress[8*6+3]=(DevPar.MaxArterialPress)&0xff;   
    Flashaddress[8*6+4]=(DevPar.MaxVenousPress>>8)&0xff;      
    Flashaddress[8*6+5]=(DevPar.MaxVenousPress)&0xff;  
    Flashaddress[8*6+6]=(DevPar.MaxTMP>>8)&0xff;          
    Flashaddress[8*6+7]=(DevPar.MaxTMP)&0xff;
    
    Flashaddress[8*7+2]=(DevPar.MinArterialPress>>8)&0xff;      
    Flashaddress[8*7+3]=(DevPar.MinArterialPress)&0xff;   
    Flashaddress[8*7+4]=(DevPar.MinVenousPress>>8)&0xff;      
    Flashaddress[8*7+5]=(DevPar.MinVenousPress)&0xff;  
    Flashaddress[8*7+6]=(UF_Set_Low>>8)&0xff;          
    Flashaddress[8*7+7]=(UF_Set_Low)&0xff;
    
    Flashaddress[8*8+2]=(YaLiPRESS_COEFF.Arterial>>8)&0xff;      
    Flashaddress[8*8+3]=(YaLiPRESS_COEFF.Arterial)&0xff;   
    Flashaddress[8*8+4]=(YaLiOffset.Arterial>>8)&0xff;      
    Flashaddress[8*8+5]=(YaLiOffset.Arterial)&0xff;  
    Flashaddress[8*8+6]=(YaLiPRESS_COEFF.Venous>>8)&0xff;          
    Flashaddress[8*8+7]=(YaLiPRESS_COEFF.Venous)&0xff;
    
    Flashaddress[8*9+2]=(YaLiOffset.Venous>>8)&0xff;      
    Flashaddress[8*9+3]=(YaLiOffset.Venous)&0xff;   
    Flashaddress[8*9+4]=(YaLiPRESS_COEFF.UF>>8)&0xff;      
    Flashaddress[8*9+5]=(YaLiPRESS_COEFF.UF)&0xff;  
    Flashaddress[8*9+6]=(YaLiOffset.UF>>8)&0xff;        
    Flashaddress[8*9+7]=(YaLiOffset.UF)&0xff;
     
    Flashaddress[8*10+2]=(YaLiPRESS_COEFF.PreFlt>>8)&0xff;      
    Flashaddress[8*10+3]=(YaLiPRESS_COEFF.PreFlt)&0xff;   
    Flashaddress[8*10+4]=(YaLiOffset.PreFlt>>8)&0xff;      
    Flashaddress[8*10+5]=(YaLiOffset.PreFlt)&0xff;  
    Flashaddress[8*10+6]=(YaLiPRESS_COEFF.Weight>>8)&0xff;          
    Flashaddress[8*10+7]=(YaLiPRESS_COEFF.Weight)&0xff;
     
    Flashaddress[8*11+2]=(YaLiOffset.Weight>>8)&0xff;      
    Flashaddress[8*11+3]=(YaLiOffset.Weight)&0xff;  
     
    Flashaddress[8*12+2]=(UfSpeed>>8)&0xff;      
    Flashaddress[8*12+3]=(UfSpeed)&0xff;   
    Flashaddress[8*12+4]=(DevPar.TargetPrimingVolume>>8)&0xff;      
    Flashaddress[8*12+5]=(DevPar.TargetPrimingVolume)&0xff;  
    Flashaddress[8*12+6]=(DevPar.BagChangeVolume>>8)&0xff;          
    Flashaddress[8*12+7]=(DevPar.BagChangeVolume)&0xff;

    return 1;
}

// END OF FILE
