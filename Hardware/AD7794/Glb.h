/*******************************************************************************
*
*       Copyright(c) 2008-2017; Beijing HeartCare Medical Co. LTD.
*
*       All rights reserved.  Protected by international copyright laws.
*       Knowledge of the source code may NOT be used to develop a similar product.
*
* File:          glb.h
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

#ifndef	__GLB_H__
#define	__GLB_H__

#ifndef NULL
#define NULL  0L
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif
/******************************************************************************/
//  常量定义
/******************************************************************************/
//预冲时压力极限值
#define  MinArterialPriming          (-200) 
#define  MaxArterialPriming          ( 200) 
#define  MinVenousPriming            (-200) 
#define  MaxVenousPriming            ( 200) 
#define  MaxTMPPriming               ( 200) 
#define  MinUFPressPriming           (-100) 

/******************************************************************************/
#define  TargetPrimingVolume_Step    ( 10)   //目标预冲量步长   
#define  DefaultTargetPrimingVolume  ( 10)   //目标预冲量 = 此值 X 步长 
#define  TargetUFVolume_Step         (100)   //目标超滤量步长  
#define  DefaultTargetUFVolume       ( 8 )   //目标超滤量 = 此值 X 步长 
#define  Scale_Err_Step              ( 1 )   //称重计误差步长  
#define  DefaultScaleErr             ( 5 )   //称重计误差 = 此值 X 步长
#define  BagChangeVolume_Step        (500)   //换袋量步长  
#define  DefaultBagChangeVolume      ( 4 )   //换袋量     = 此值 X 步长
/******************************************************************************/
#define  MinArterial_min             (-30)   //动脉压低限最小值
#define  MinArterial_max             (-5 )   //动脉压低限最大值
#define  MaxArterial_min             (-5 )   //动脉压高限最小值
#define  MaxArterial_max             ( 15)   //动脉压高限最大值
#define  MinVenous_min               ( 1 )   //静脉压低限最小值
#define  MinVenous_max               ( 8 )   //静脉压低限最大值
#define  MaxVenous_min               ( 5 )   //静脉压高限最小值
#define  MaxVenous_max               ( 10)   //静脉压高限最大值
#define  MinUF_min                   (-40)   //超滤压低限最小值
#define  MinUF_max                   ( 2 )   //超滤压低限最大值
#define  MaxTMP_min                  ( 0 )   //跨膜压高限最小值
#define  MaxTMP_max                  ( 25)   //跨膜压高限最大值

//各压力高低限默认值
#define  DefaultMinArterialPress     (-20) 
#define  DefaultMaxArterialPress     ( 15)   
#define  DefaultMinVenousPress       ( 1 ) 
#define  DefaultMaxVenousPress       ( 10) 
#define  DefaultMinUFPress           (-30) 
#define  DefaultMaxUFPress           ( 5 ) 
#define  DefaultMinTMP               ( 0 ) 
#define  DefaultMaxTMP               ( 20) 

#define  Min_Arterial_Step           ( 10 )  //动脉压低限步长
#define  Max_Arterial_Step           ( 10 )  //动脉压高限步长
#define  Min_Venous_Step             ( 10 )  //静脉压低限步长 
#define  Max_Venous_Step             ( 20 )  //静脉压高限步长
#define  Min_UFPress_Step            ( 10 )  //超滤压低限步长
#define  Max_TMP_Step                ( 20 )  //跨膜压高限步长
/******************************************************************************/
//最小预冲量步长
#define  DefaultTargetPrimingVolumeMin_Step       ( 10 )
//最小预冲量 = 此值 X 步长
#define  DefaultTargetPrimingVolumeMin            ( 5 )   
//预冲结束检测时静脉压上升最小值
#define  DefaultVenousIncreaseMin                 ( 5 )
//开机出系统维护按钮要求钥匙重量值区间 88～93g（不含）
#define  DefaultSerMin               ( 88 )
#define  DefaultSerMax               ( 93 )
/******************************************************************************/
//主状态定义
#define  STATE_NULL             0       // 无
#define  STATE_STARTANIMAL      1       // 启动动画
#define  STATE_SELFTEST         2       // 系统自检
#define  STATE_WEICALIBRATE     3       // 称重定标
#define  STATE_PREPRIMING       4       // 准备预冲
#define  STATE_PRIMING          5       // 进行预冲
#define  STATE_THERAPY          6       // 进行治疗
#define  STATE_FINISHED         7       // 确认结束
#define  STATE_FINISH           8       // 治疗结束
#define  STATE_MAINTANCE        9       // 系统维护
#define  STATE_WEIGHING         10      // 称重校准
#define  STATE_PARAMSET         11      // 治疗参数设置
#define  STATE_SETTING1         12      // 系统设置1
#define  STATE_SETTING2         13      // 系统设置2

/******************************************************************************/
//  数据结构定义
/******************************************************************************/
// 设备参数
typedef struct __DevParameter
{
    volatile unsigned char PrimingUFSpeed;        //预冲时超滤泵速度 0～12
    volatile unsigned char TherapyUFSpeed;        //治疗时超滤泵速度 0～12
    volatile unsigned char PrimingBloodSpeed;     //预冲时血泵速度 0～10
    volatile unsigned char TherapyBloodSpeed;     //治疗时血泵速度 0～10
 
    volatile unsigned char TargetPrimingVolume;   // 目标预冲量 = 此值 X 步长  
    volatile unsigned char TargetUFVolume;        // 目标超滤量 = 此值 X 步长
    volatile unsigned long ScaleErr;              // 称重计误差 = 此值 X 步长
    volatile unsigned char BagChangeVolume;       // 换袋量     = 此值 X 步长  

    volatile   signed char MinArterialPress;      // 动脉压低限
    volatile   signed char MaxArterialPress;      // 动脉压高限
    volatile   signed char MinVenousPress;        // 静脉压低限
    volatile   signed char MaxVenousPress;        // 静脉压高限
    
    volatile   signed char MinUFPress;            // 超滤压低限
    volatile   signed char MaxUFPress;            // 超滤压高限
    volatile   signed char MinTMP;                // 跨膜压低限
    volatile   signed char MaxTMP;                // 跨膜压高限
        
    volatile unsigned char TargetPrimingVolumeMin;   // 最小预冲量
} DevParameter_t; 
/******************************************************************************/
//////异常报警标志结构体  此顺序不能变
typedef struct err_type
{
    volatile unsigned char  PumpCover;     //0  泵头盖未关
    volatile unsigned char  ClipNoOpen;    //1  静脉夹未开
    volatile unsigned char  AirBubble;     //2  检测到气泡 
    volatile unsigned char  BloodLeak;     //3  检测到漏血

    volatile unsigned char  ArterialPress; //4  动脉压异常
    volatile unsigned char  VenousPress;   //5  静脉压异常
    volatile unsigned char  Tmp;           //6  跨膜压异常
    volatile unsigned char  UfPress;       //7  超滤压异常

    volatile unsigned char  Balance;       //8  称重计不稳
    volatile unsigned char  BagChange;     //9  需要换袋
    volatile unsigned char  EndPrime;      //10 预冲结束
    volatile unsigned char  Motor;         //11 电机故障

    volatile unsigned char  BldStop;       //12 血泵未运行
    volatile unsigned char  Picc;          //13 识别卡错误
    volatile unsigned char  Clip;          //14 静脉夹故障
    volatile unsigned char  YclBz;         //15  预冲量不足
    
    volatile unsigned char  PreArterialPress;  //16  动脉压预警	
    volatile unsigned char  PreVenousPress;    //17  静脉压预警
    volatile unsigned char  PreTmp;            //18  跨膜压预警
    volatile unsigned char  PreUfPress;        //19  超滤压预警
    
    volatile unsigned char  UfRate;            //20  超滤率异常
    volatile unsigned char  Zero;              //21  误差超限
    volatile unsigned char  SelfTest;          //22  自检未通过    
} ERR_t;

/******************************************************************************/
////压力
typedef struct __YaLi
{
    volatile signed long Venous;    //静脉压
    volatile signed long Arterial;  //动脉压
    volatile signed long UF;        //超滤压
    volatile signed long PreFlt;    //滤器前压
    volatile signed long TMP;       //跨膜压
    volatile signed long Weight;	//称重
} YaLi_t;

/******************************************************************************/
////使用时间分钟
typedef struct __UsedMinute
{
    volatile unsigned long  TotalMinutes;  //启动时间
    volatile unsigned long  TreatMinutes;  //治疗时间
    volatile unsigned long  TotalUFVolume; //总超滤量
} UsedMinutes_t;  
/******************************************************************************/
//设备状态
typedef struct stateflag_type
{
    volatile unsigned char  MainState;       // system status
    volatile unsigned char  Emergency;       // EmergencyFlag 
    volatile unsigned char  LcdRefreshing;   // LcdRefreshing 
    volatile unsigned char  GUIChanged;      // StatusChanged
    volatile unsigned char  KeyReady;        // KeyAvailable
    volatile unsigned char  KeyPressed;      // KeyPressed
    volatile unsigned char  UfMotorRun;      // UfRunFlag
    volatile unsigned char  BldMotorRun;     // BlRunFlag
    volatile unsigned char  LedAlarm;        // Led Alarm Enable
    volatile unsigned char  BuzzAlarm;       // Buzz Alarm Enable
    volatile unsigned char  BuzzAlarmDelay;  // Buzz Alarm Delay Flag
    volatile unsigned char  VenousLowDelay;  // Venous low Alarm Delay Flag
    volatile unsigned char  BagChangeDelay;  // Bag change Alarm Delay Flag
    volatile unsigned char  BagChange;       // Bag change Flag
    volatile unsigned char  Clip;            // ClipFlag:  open=0,  close=1
    volatile unsigned char  Runing;          // 
    volatile unsigned char  LedAlarmMaxMin;      
    volatile unsigned char  BuzzAlarmMaxMin;  
    volatile unsigned char  BuzzAlarmPrmingLong;  
    volatile unsigned char  PrimingEndDelay;
} STATE_t;

/******************************************************************************/
#endif

//// 亚洲  非洲  西欧  东欧  大洋洲   北美洲   南美洲 

////// End of file

