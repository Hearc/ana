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
//  ��������
/******************************************************************************/
//Ԥ��ʱѹ������ֵ
#define  MinArterialPriming          (-200) 
#define  MaxArterialPriming          ( 200) 
#define  MinVenousPriming            (-200) 
#define  MaxVenousPriming            ( 200) 
#define  MaxTMPPriming               ( 200) 
#define  MinUFPressPriming           (-100) 

/******************************************************************************/
#define  TargetPrimingVolume_Step    ( 10)   //Ŀ��Ԥ��������   
#define  DefaultTargetPrimingVolume  ( 10)   //Ŀ��Ԥ���� = ��ֵ X ���� 
#define  TargetUFVolume_Step         (100)   //Ŀ�곬��������  
#define  DefaultTargetUFVolume       ( 8 )   //Ŀ�곬���� = ��ֵ X ���� 
#define  Scale_Err_Step              ( 1 )   //���ؼ�����  
#define  DefaultScaleErr             ( 5 )   //���ؼ���� = ��ֵ X ����
#define  BagChangeVolume_Step        (500)   //����������  
#define  DefaultBagChangeVolume      ( 4 )   //������     = ��ֵ X ����
/******************************************************************************/
#define  MinArterial_min             (-30)   //����ѹ������Сֵ
#define  MinArterial_max             (-5 )   //����ѹ�������ֵ
#define  MaxArterial_min             (-5 )   //����ѹ������Сֵ
#define  MaxArterial_max             ( 15)   //����ѹ�������ֵ
#define  MinVenous_min               ( 1 )   //����ѹ������Сֵ
#define  MinVenous_max               ( 8 )   //����ѹ�������ֵ
#define  MaxVenous_min               ( 5 )   //����ѹ������Сֵ
#define  MaxVenous_max               ( 10)   //����ѹ�������ֵ
#define  MinUF_min                   (-40)   //����ѹ������Сֵ
#define  MinUF_max                   ( 2 )   //����ѹ�������ֵ
#define  MaxTMP_min                  ( 0 )   //��Ĥѹ������Сֵ
#define  MaxTMP_max                  ( 25)   //��Ĥѹ�������ֵ

//��ѹ���ߵ���Ĭ��ֵ
#define  DefaultMinArterialPress     (-20) 
#define  DefaultMaxArterialPress     ( 15)   
#define  DefaultMinVenousPress       ( 1 ) 
#define  DefaultMaxVenousPress       ( 10) 
#define  DefaultMinUFPress           (-30) 
#define  DefaultMaxUFPress           ( 5 ) 
#define  DefaultMinTMP               ( 0 ) 
#define  DefaultMaxTMP               ( 20) 

#define  Min_Arterial_Step           ( 10 )  //����ѹ���޲���
#define  Max_Arterial_Step           ( 10 )  //����ѹ���޲���
#define  Min_Venous_Step             ( 10 )  //����ѹ���޲��� 
#define  Max_Venous_Step             ( 20 )  //����ѹ���޲���
#define  Min_UFPress_Step            ( 10 )  //����ѹ���޲���
#define  Max_TMP_Step                ( 20 )  //��Ĥѹ���޲���
/******************************************************************************/
//��СԤ��������
#define  DefaultTargetPrimingVolumeMin_Step       ( 10 )
//��СԤ���� = ��ֵ X ����
#define  DefaultTargetPrimingVolumeMin            ( 5 )   
//Ԥ��������ʱ����ѹ������Сֵ
#define  DefaultVenousIncreaseMin                 ( 5 )
//������ϵͳά����ťҪ��Կ������ֵ���� 88��93g��������
#define  DefaultSerMin               ( 88 )
#define  DefaultSerMax               ( 93 )
/******************************************************************************/
//��״̬����
#define  STATE_NULL             0       // ��
#define  STATE_STARTANIMAL      1       // ��������
#define  STATE_SELFTEST         2       // ϵͳ�Լ�
#define  STATE_WEICALIBRATE     3       // ���ض���
#define  STATE_PREPRIMING       4       // ׼��Ԥ��
#define  STATE_PRIMING          5       // ����Ԥ��
#define  STATE_THERAPY          6       // ��������
#define  STATE_FINISHED         7       // ȷ�Ͻ���
#define  STATE_FINISH           8       // ���ƽ���
#define  STATE_MAINTANCE        9       // ϵͳά��
#define  STATE_WEIGHING         10      // ����У׼
#define  STATE_PARAMSET         11      // ���Ʋ�������
#define  STATE_SETTING1         12      // ϵͳ����1
#define  STATE_SETTING2         13      // ϵͳ����2

/******************************************************************************/
//  ���ݽṹ����
/******************************************************************************/
// �豸����
typedef struct __DevParameter
{
    volatile unsigned char PrimingUFSpeed;        //Ԥ��ʱ���˱��ٶ� 0��12
    volatile unsigned char TherapyUFSpeed;        //����ʱ���˱��ٶ� 0��12
    volatile unsigned char PrimingBloodSpeed;     //Ԥ��ʱѪ���ٶ� 0��10
    volatile unsigned char TherapyBloodSpeed;     //����ʱѪ���ٶ� 0��10
 
    volatile unsigned char TargetPrimingVolume;   // Ŀ��Ԥ���� = ��ֵ X ����  
    volatile unsigned char TargetUFVolume;        // Ŀ�곬���� = ��ֵ X ����
    volatile unsigned long ScaleErr;              // ���ؼ���� = ��ֵ X ����
    volatile unsigned char BagChangeVolume;       // ������     = ��ֵ X ����  

    volatile   signed char MinArterialPress;      // ����ѹ����
    volatile   signed char MaxArterialPress;      // ����ѹ����
    volatile   signed char MinVenousPress;        // ����ѹ����
    volatile   signed char MaxVenousPress;        // ����ѹ����
    
    volatile   signed char MinUFPress;            // ����ѹ����
    volatile   signed char MaxUFPress;            // ����ѹ����
    volatile   signed char MinTMP;                // ��Ĥѹ����
    volatile   signed char MaxTMP;                // ��Ĥѹ����
        
    volatile unsigned char TargetPrimingVolumeMin;   // ��СԤ����
} DevParameter_t; 
/******************************************************************************/
//////�쳣������־�ṹ��  ��˳���ܱ�
typedef struct err_type
{
    volatile unsigned char  PumpCover;     //0  ��ͷ��δ��
    volatile unsigned char  ClipNoOpen;    //1  ������δ��
    volatile unsigned char  AirBubble;     //2  ��⵽���� 
    volatile unsigned char  BloodLeak;     //3  ��⵽©Ѫ

    volatile unsigned char  ArterialPress; //4  ����ѹ�쳣
    volatile unsigned char  VenousPress;   //5  ����ѹ�쳣
    volatile unsigned char  Tmp;           //6  ��Ĥѹ�쳣
    volatile unsigned char  UfPress;       //7  ����ѹ�쳣

    volatile unsigned char  Balance;       //8  ���ؼƲ���
    volatile unsigned char  BagChange;     //9  ��Ҫ����
    volatile unsigned char  EndPrime;      //10 Ԥ�����
    volatile unsigned char  Motor;         //11 �������

    volatile unsigned char  BldStop;       //12 Ѫ��δ����
    volatile unsigned char  Picc;          //13 ʶ�𿨴���
    volatile unsigned char  Clip;          //14 �����й���
    volatile unsigned char  YclBz;         //15  Ԥ��������
    
    volatile unsigned char  PreArterialPress;  //16  ����ѹԤ��	
    volatile unsigned char  PreVenousPress;    //17  ����ѹԤ��
    volatile unsigned char  PreTmp;            //18  ��ĤѹԤ��
    volatile unsigned char  PreUfPress;        //19  ����ѹԤ��
    
    volatile unsigned char  UfRate;            //20  �������쳣
    volatile unsigned char  Zero;              //21  ����
    volatile unsigned char  SelfTest;          //22  �Լ�δͨ��    
} ERR_t;

/******************************************************************************/
////ѹ��
typedef struct __YaLi
{
    volatile signed long Venous;    //����ѹ
    volatile signed long Arterial;  //����ѹ
    volatile signed long UF;        //����ѹ
    volatile signed long PreFlt;    //����ǰѹ
    volatile signed long TMP;       //��Ĥѹ
    volatile signed long Weight;	//����
} YaLi_t;

/******************************************************************************/
////ʹ��ʱ�����
typedef struct __UsedMinute
{
    volatile unsigned long  TotalMinutes;  //����ʱ��
    volatile unsigned long  TreatMinutes;  //����ʱ��
    volatile unsigned long  TotalUFVolume; //�ܳ�����
} UsedMinutes_t;  
/******************************************************************************/
//�豸״̬
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

//// ����  ����  ��ŷ  ��ŷ  ������   ������   ������ 

////// End of file

