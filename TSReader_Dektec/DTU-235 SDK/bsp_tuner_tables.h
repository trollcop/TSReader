
#ifndef __DRXBSP_TUNER_TABLES_H__
#define __DRXBSP_TUNER_TABLES_H__

/*------------------------------------------------------------------------------
INCLUDES
------------------------------------------------------------------------------*/

#include "bsp_tuner.h"
#include "tuner5byte.h"

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------
DEFINES
-------------------------------------------------------------------------*/

#define ARRAYREF(array) array, ( sizeof(array) / sizeof(array[0]) )
#define MAXFRQ  ((DRXFrequency_t)(0x7FFFFFFF))
// #define TUNER_I2C_ADDR ((0x60)<<1) /* default I2C address for tuners */
#define TUNER_I2C_ADDR (0xC0) /* default I2C address for tuners */
#define TUNER_DEV_ID   (2)         /* default tuner device id */

/*-------------------------------------------------------------------------
DEFINES FOR MODE CONTRACTION
-------------------------------------------------------------------------*/

#define TUNER_MODE_DIGITALANALOG   ( TUNER_MODE_DIGITAL | TUNER_MODE_ANALOG )
#define TUNER_MODE_SWITCHLOCK      ( TUNER_MODE_SWITCH  | TUNER_MODE_LOCK )
#define TUNER_MODE_67MHZ           ( TUNER_MODE_6MHZ    | TUNER_MODE_7MHZ )
#define TUNER_MODE_678MHZ          ( TUNER_MODE_6MHZ    | TUNER_MODE_7MHZ | TUNER_MODE_8MHZ )

/*------------------------------------------------------------------------------
TUNER SETTINGS
------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Thomson DTT 759X tuner configuration                                       */
/*----------------------------------------------------------------------------*/
#define PRE_CONF_TUNER    tunerThomsonDTT759X

static TUNER5BYTEControlUnit_t TCU_ThomsonDTT759X_AUX_SCAN[] =
{
   /* ATC = 1/010 */ { MAXFRQ , 0x9C , 0xA0 }
};

static TUNER5BYTEControlUnit_t TCU_ThomsonDTT759X_AUX_LOCK[] =
{
   /* ATC = 0/010 */ { MAXFRQ , 0x9C , 0x20 }
};

static TUNER5BYTEControlUnit_t TCU_ThomsonDTT759X_AUX_SUB1[] =
{
   /* ATC = 0/110 */ { MAXFRQ , 0x9C , 0x60 }
};

static TUNER5BYTEControlUnit_t TCU_ThomsonDTT759X_BS_7[] =
{
   /*     - 305 MHz: CP/T = 0/110, BS = 0x12 */ { 305000 , 0xB4 , 0x12 },
   /* 305 - 405 MHz: CP/T = 0/111, BS = 0x12 */ { 405000 , 0xBC , 0x12 },
   /* 405 - 445 MHz: CP/T = 1/110, BS = 0x12 */ { 445000 , 0xF4 , 0x12 },
   /* 445 - 465 MHz: CP/T = 1/111, BS = 0x12 */ { 465000 , 0xFC , 0x12 },
   /* 465 - 735 MHz: CP/T = 0/111, BS = 0x18 */ { 735000 , 0xBC , 0x18 },
   /* 735 - 835 MHz: CP/T = 1/110, BS = 0x18 */ { 835000 , 0xF4 , 0x18 },
   /* 835 - 896 MHz: CP/T = 1/111, BS = 0x18 */ { MAXFRQ , 0xFC , 0x18 }
};

static TUNER5BYTEControlUnit_t TCU_ThomsonDTT759X_BS_8[] =
{
   /*     - 305 MHz: CP/T = 0/110, BS = 0x02 */ { 305000 , 0xB4 , 0x02 },
   /* 305 - 405 MHz: CP/T = 0/111, BS = 0x02 */ { 405000 , 0xBC , 0x02 },
   /* 405 - 445 MHz: CP/T = 1/110, BS = 0x02 */ { 445000 , 0xF4 , 0x02 },
   /* 445 - 465 MHz: CP/T = 1/111, BS = 0x02 */ { 465000 , 0xFC , 0x02 },
   /* 465 - 735 MHz: CP/T = 0/111, BS = 0x08 */ { 735000 , 0xBC , 0x08 },
   /* 735 - 835 MHz: CP/T = 1/110, BS = 0x08 */ { 835000 , 0xF4 , 0x08 },
   /* 835 - 896 MHz: CP/T = 1/111, BS = 0x08 */ { MAXFRQ , 0xFC , 0x08 }
};

static TUNER5BYTEControlTable_t TCT_ThomsonDTT759X[] =
{
   {
      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_SWITCH | TUNER_MODE_678MHZ | TUNER_MODE_SUB0,
      /* description              */ "auxiliary-scan",
      /* refFreqNumerator   [kHz] */   500,
      /* refFreqDenominator       */     3,
      /* outputFrequency    [kHz] */ 36000,
      /* controlUnits             */ TCU_ThomsonDTT759X_AUX_SCAN
   },
   {
      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_LOCK | TUNER_MODE_678MHZ | TUNER_MODE_SUB0,
      /* description              */ "auxiliary-locked",
      /* refFreqNumerator   [kHz] */   500,
      /* refFreqDenominator       */     3,
      /* outputFrequency    [kHz] */ 36000,
      /* controlUnits             */ TCU_ThomsonDTT759X_AUX_LOCK
   },
   {
      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_SWITCH | TUNER_MODE_678MHZ | TUNER_MODE_SUB1,
      /* description              */ "auxiliary, external RF-AGC",
      /* refFreqNumerator   [kHz] */   500,
      /* refFreqDenominator       */     3,
      /* outputFrequency    [kHz] */ 36000,
      /* controlUnits             */ TCU_ThomsonDTT759X_AUX_SUB1
   },
   {
      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_SWITCHLOCK | TUNER_MODE_67MHZ | TUNER_MODE_SUBALL,
      /* description              */ "7 MHz SAW, bandswitch",
      /* refFreqNumerator   [kHz] */   500,
      /* refFreqDenominator       */     3,
      /* outputFrequency    [kHz] */ 36000,
      /* controlUnits             */ TCU_ThomsonDTT759X_BS_7
   },
   {
      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_SWITCHLOCK | TUNER_MODE_8MHZ | TUNER_MODE_SUBALL,
      /* description              */ "8 MHz SAW, bandswitch",
      /* refFreqNumerator   [kHz] */   500,
      /* refFreqDenominator       */     3,
      /* outputFrequency    [kHz] */ 36000,
      /* controlUnits             */ TCU_ThomsonDTT759X_BS_8
   }
};

static TUNERSubMode_t subModesThomsonDTT759X[] =
{
   "TOP 112dBuV",
   "External RF-AGC"
};

static TUNER5BYTEData_t dataThomsonDTT759X =
{
   /* lockMask     */ 0x40,
   /* lockValue    */ 0x40,
   /* controlTable */ ARRAYREF(TCT_ThomsonDTT759X)
};

static TUNERCommonAttr_t commonAttrThomsonDTT759X =
{
   /* name            */  "Thomson DTT 759X",
   /* minFreqRF [kHz] */     149000,
   /* maxFreqRF [kHz] */     896000,
   /* sub-mode        */          0,
   /* sub-modes       */  ARRAYREF(subModesThomsonDTT759X)
};

static TUNERInstance_t tunerThomsonDTT759X =
{
   /* I2C address  */ { TUNER_I2C_ADDR , TUNER_DEV_ID },
   /* myCommonAttr */ &commonAttrThomsonDTT759X,
   /* myExtAttr    */ &dataThomsonDTT759X,
   /* myFunct      */ &TUNER5BYTEFunctions_g
};

/*----------------------------------------------------------------------------*/
/*----end-Thomson-DTT-759X----------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Philips TD(M) 1316AL-mk2 tuner configuration  (preliminary settings)       */
/*----------------------------------------------------------------------------*/
//#define PRE_CONF_TUNER  tunerPhilipsTD1316AL2
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_DGTL_AUX[] =
//{
//   /* Full range: AL = 011 */ { MAXFRQ,  0x9C , 0xA0 }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_DGTL_SCAN_BS_7[] =
//{
//   /*     - 159 MHz: CP/SP = 0111/00001 , 7MHz SAW */ { 159000 , 0xBC , 0x01 },
//   /* 162 - 444 MHz: CP/SP = 0111/00010 , 7MHz SAW */ { 444000 , 0xBC , 0x02 },
//   /* 448 -     MHz: CP/SP = 0111/00100 , 7MHz SAW */ { MAXFRQ,  0xBC , 0x04 }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_DGTL_SCAN_BS_8[] =
//{
//   /*     - 159 MHz: CP/SP = 0111/01001 , 8MHz SAW */ { 159000 , 0xBC , 0x09 },
//   /* 162 - 444 MHz: CP/SP = 0111/01010 , 8MHz SAW */ { 444000 , 0xBC , 0x0A },
//   /* 448 -     MHz: CP/SP = 0111/01100 , 8MHz SAW */ { MAXFRQ,  0xBC , 0x0C }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_DGTL_LOCKED_BS_7[] =
//{
//   /*     - 180 MHz: CP/SP = 0111/00001 , 7MHz SAW */ { 180000 , 0xBC , 0x01 },
//   /* 180 - 197 MHz: CP/SP = 1110/00001 , 7MHz SAW */ { 197000 , 0xF4 , 0x01 },
//   /* 197 - 366 MHz: CP/SP = 0111/00010 , 7MHz SAW */ { 366000 , 0xBC , 0x02 },
//   /* 366 - 484 MHz: CP/SP = 1110/00010 , 7MHz SAW */ { 484000 , 0xF4 , 0x02 },
//   /* 484 - 646 MHz: CP/SP = 0111/00100 , 7MHz SAW */ { 646000,  0xBC , 0x04 },
//   /* 646 - 790 MHz: CP/SP = 1110/00100 , 7MHz SAW */ { 790000,  0xF4 , 0x04 },
//   /* 790 -     MHz: CP/SP = 1111/00100 , 7MHz SAW */ { MAXFRQ,  0xFC , 0x04 }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_DGTL_LOCKED_BS_8[] =
//{
//   /*     - 180 MHz: CP/SP = 0111/01001 , 8MHz SAW */ { 180000 , 0xBC , 0x09 },
//   /* 180 - 197 MHz: CP/SP = 1110/01001 , 8MHz SAW */ { 197000 , 0xF4 , 0x09 },
//   /* 197 - 366 MHz: CP/SP = 0111/01010 , 8MHz SAW */ { 366000 , 0xBC , 0x0A },
//   /* 366 - 484 MHz: CP/SP = 1110/01010 , 8MHz SAW */ { 484000 , 0xF4 , 0x0A },
//   /* 484 - 646 MHz: CP/SP = 0111/01100 , 8MHz SAW */ { 646000,  0xBC , 0x0C },
//   /* 646 - 790 MHz: CP/SP = 1110/01100 , 8MHz SAW */ { 790000,  0xF4 , 0x0C },
//   /* 790 -     MHz: CP/SP = 1111/01100 , 8MHz SAW */ { MAXFRQ,  0xFC , 0x0C }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_ANLG_AUX0[] =
//{
//   /* Full range: AL = 101 */ { MAXFRQ,  0x9E , 0xD0 }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_ANLG_AUX1[] =
//{
//   /* Full range: AL = 101 */ { MAXFRQ,  0x9E , 0xE0 }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_ANLG_SCAN[] =
//{
//   /*     - 159 MHz: CP/SP = 0111/00001 , 7MHz SAW */ { 159000 , 0xCE , 0x01 },
//   /* 162 - 444 MHz: CP/SP = 0111/00010 , 7MHz SAW */ { 444000 , 0xCE , 0x02 },
//   /* 448 -     MHz: CP/SP = 0111/00100 , 7MHz SAW */ { MAXFRQ,  0xCE , 0x04 }
//};
//
//static TUNER5BYTEControlUnit_t TCU_PhilipsTD1316AL2_ANLG_LOCKED[] =
//{
//   /*     - 159 MHz: CP/SP = 1110/01001 , 8MHz SAW */ { 159000 , 0xF6 , 0x09 },
//   /* 162 - 444 MHz: CP/SP = 1110/01010 , 8MHz SAW */ { 444000 , 0xF6 , 0x0A },
//   /* 448 -     MHz: CP/SP = 1110/01100 , 8MHz SAW */ { MAXFRQ,  0xF6 , 0x0C }
//};
//
//static TUNER5BYTEControlTable_t TCT_PhilipsTD1316AL2[] =
//{
//   {
//      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_SWITCHLOCK | TUNER_MODE_678MHZ | TUNER_MODE_SUBALL,
//      /* description              */ "digital, auxilary byte",
//      /* refFreqNumerator   [kHz] */   500,
//      /* refFreqDenominator       */     3,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_DGTL_AUX
//   },
//   {
//      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_SWITCH | TUNER_MODE_67MHZ | TUNER_MODE_SUBALL,
//      /* description              */ "search/scan, bandswitch byte, 7MHz Bandwidth",
//      /* refFreqNumerator   [kHz] */   500,
//      /* refFreqDenominator       */     3,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_DGTL_SCAN_BS_7
//   },
//   {
//      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_SWITCH | TUNER_MODE_8MHZ | TUNER_MODE_SUBALL,
//      /* description              */ "search/scan, bandswitch byte, 8MHz Bandwidth",
//      /* refFreqNumerator   [kHz] */   500,
//      /* refFreqDenominator       */     3,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_DGTL_SCAN_BS_8
//   },
//   {
//      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_LOCK | TUNER_MODE_67MHZ | TUNER_MODE_SUBALL,
//      /* description              */ "digital locked, bandswitch byte, 7MHz Bandwidth",
//      /* refFreqNumerator   [kHz] */   500,
//      /* refFreqDenominator       */     3,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_DGTL_LOCKED_BS_7
//   },
//   {
//      /* modes                    */ TUNER_MODE_DIGITAL | TUNER_MODE_LOCK | TUNER_MODE_8MHZ | TUNER_MODE_SUBALL,
//      /* description              */ "digital locked, bandswitch byte, 8MHz Bandwidth",
//      /* refFreqNumerator   [kHz] */   500,
//      /* refFreqDenominator       */     3,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_DGTL_LOCKED_BS_8
//   },
//   {
//      /* modes                    */ TUNER_MODE_ANALOG | TUNER_MODE_SWITCHLOCK | TUNER_MODE_678MHZ | TUNER_MODE_SUB0,
//      /* description              */ "analogue, TOP -12",
//      /* refFreqNumerator   [kHz] */   125,
//      /* refFreqDenominator       */     2,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_ANLG_AUX0
//   },
//   {
//      /* modes                    */ TUNER_MODE_ANALOG | TUNER_MODE_SWITCHLOCK | TUNER_MODE_678MHZ | TUNER_MODE_SUB1,
//      /* description              */ "analogue, external RF-AGC",
//      /* refFreqNumerator   [kHz] */   125,
//      /* refFreqDenominator       */     2,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_ANLG_AUX1
//   },
//   {
//      /* modes                    */ TUNER_MODE_ANALOG | TUNER_MODE_SWITCH | TUNER_MODE_678MHZ | TUNER_MODE_SUBALL,
//      /* description              */ "analogue search/scan, bandswitch",
//      /* refFreqNumerator   [kHz] */   125,
//      /* refFreqDenominator       */     2,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_ANLG_SCAN
//   },
//   {
//      /* modes                    */ TUNER_MODE_ANALOG | TUNER_MODE_LOCK | TUNER_MODE_678MHZ | TUNER_MODE_SUBALL,
//      /* description              */ "analogue locked, bandswitch",
//      /* refFreqNumerator   [kHz] */   125,
//      /* refFreqDenominator       */     2,
//      /* outputFrequency    [kHz] */ 36130,
//      /* controlUnits             */ TCU_PhilipsTD1316AL2_ANLG_LOCKED
//   }
//};
//
//static TUNERSubMode_t subModesPhilipsTD1316AL2[] =
//{
//   "Digital/analog TOP 115dBuV",
//   "Digital TOP 115dBuV, Analog external RF-AGC"
//};
//
//static TUNER5BYTEData_t dataPhilipsTD1316AL2 =
//{
//   /* lockMask     */       0x40,
//   /* lockValue    */       0x40,
//   /* controlTable */ ARRAYREF(TCT_PhilipsTD1316AL2)
//};
//
//static TUNERCommonAttr_t commonAttrPhilipsTD1316AL2 =
//{
//   /* name            */  "Philips TD(M) 1316AL-mk2",
//   /* minFreqRF [kHz] */   51000,
//   /* maxFreqRF [kHz] */  858000,
//   /* sub-mode        */       1,
//   /* sub-modes       */  ARRAYREF(subModesPhilipsTD1316AL2)
//};
//
//static TUNERInstance_t tunerPhilipsTD1316AL2 =
//{
//   /* I2C address  */ { TUNER_I2C_ADDR , TUNER_DEV_ID },
//   /* myCommonAttr */ &commonAttrPhilipsTD1316AL2,
//   /* myExtAttr    */ &dataPhilipsTD1316AL2,
//   /* myFunct      */ &TUNER5BYTEFunctions_g
//};
/*----end-Philips-TD(M)-1316AL-mk2--------------------------------------------*/

#ifdef __cplusplus
}
#endif
#endif   /* __DRXBSP_TUNER_TABLES_H__ */
/* End of file */

