/**
* \file $Id: frequency_plans.h,v 1.5 2005/10/04 15:24:14 gaol Exp $
*
* \brief Frequency plans filled out for different regions
*
* \author Jasper Schrader
*/

/*
* $(c) 2004-2005 Micronas GmbH. All rights reserved.
*
* This software and related documentation (the 'Software') are intellectual
* property owned by Micronas and are copyright of Micronas, unless specifically
* noted otherwise.
*
* Any use of the Software is permitted only pursuant to the terms of the
* license agreement, if any, which accompanies, is included with or applicable
* to the Software ('License Agreement') or upon express written consent of
* Micronas. Any copying, reproduction or redistribution of the Software in
* whole or in part by any means not in accordance with the License Agreement
* or as agreed in writing by Micronas is expressly prohibited.
*
* THE SOFTWARE IS WARRANTED, IF AT ALL, ONLY ACCORDING TO THE TERMS OF THE
* LICENSE AGREEMENT. EXCEPT AS WARRANTED IN THE LICENSE AGREEMENT THE SOFTWARE
* IS DELIVERED 'AS IS' AND MICRONAS HEREBY DISCLAIMS ALL WARRANTIES AND
* CONDITIONS WITH REGARD TO THE SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
* AND CONDITIONS OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIT
* ENJOYMENT, TITLE AND NON-INFRINGEMENT OF ANY THIRD PARTY INTELLECTUAL
* PROPERTY OR OTHER RIGHTS WHICH MAY RESULT FROM THE USE OR THE INABILITY
* TO USE THE SOFTWARE.
*
* IN NO EVENT SHALL MICRONAS BE LIABLE FOR INDIRECT, INCIDENTAL, CONSEQUENTIAL,
* PUNITIVE, SPECIAL OR OTHER DAMAGES WHATSOEVER INCLUDING WITHOUT LIMITATION,
* DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
* INFORMATION, AND THE LIKE, ARISING OUT OF OR RELATING TO THE USE OF OR THE
* INABILITY TO USE THE SOFTWARE, EVEN IF MICRONAS HAS BEEN ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGES, EXCEPT PERSONAL INJURY OR DEATH RESULTING FROM
* MICRONAS' NEGLIGENCE.                                                        $
*
*/

#ifndef __FREQUENCY_PLANS__
#define __FREQUENCY_PLANS__
/*-------------------------------------------------------------------------
INCLUDES
-------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------
FREQUENCY PLAN SWITCHES
-------------------------------------------------------------------------*/

/* #define FREQUENCY_PLAN_ALL          */ /* unmark to use all preconfigured frequency plans */
/* #define FREQUENCY_PLAN_AUSTRALIA    */ /* unmark to use frequency plan of Australia */
/* #define FREQUENCY_PLAN_CHINA        */ /* unmark to use frequency plan of China */
/* #define FREQUENCY_PLAN_EUROPE       */ /* unmark to use frequency plan of most European countries (CCIR) */
/* #define FREQUENCY_PLAN_INDONESIA    */ /* unmark to use frequency plan of Indonesia */
/* #define FREQUENCY_PLAN_IRELAND      */ /* unmark to use frequency plan of Ireland */
/* #define FREQUENCY_PLAN_ITALY        */ /* unmark to use frequency plan of Italy */
/* #define FREQUENCY_PLAN_JAPAN        */ /* unmark to use frequency plan of Japan */
/* #define FREQUENCY_PLAN_NEW_ZEALAND  */ /* unmark to use frequency plan of New Zealand */
/* #define FREQUENCY_PLAN_SOUTH_AFRICA */ /* unmark to use frequency plan of South Africa */
/* #define FREQUENCY_PLAN_USA          */ /* unmark to use frequency plan of USA */

/*------------------------------------------------------------------------------
FREQUENCY PLANS
------------------------------------------------------------------------------*/

/* in case all frequency plans are compiled, FREQUENCY_PLAN_SELECTED points to the European plan */
#ifdef  FREQUENCY_PLAN_ALL
#define FREQUENCY_PLAN_SELECTED  freqPlan_Europe
#endif

/*----------------------------------------------------------------------------*/
/* Australian frequency plan                                                  */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_AUSTRALIA
#define FREQUENCY_PLAN_AUSTRALIA
#endif
#endif
#ifdef  FREQUENCY_PLAN_AUSTRALIA

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_Australia
#endif

static DRXFrequencyPlan_t freqPlan_Australia[] = {
         {  48500,  48500, DRX_BANDWIDTH_7MHZ },
         {  59500,  66500, DRX_BANDWIDTH_7MHZ },
         {  88500,  88500, DRX_BANDWIDTH_7MHZ },
         {  97500, 104500, DRX_BANDWIDTH_7MHZ },
         { 140500, 140500, DRX_BANDWIDTH_7MHZ },
         { 177500, 226500, DRX_BANDWIDTH_7MHZ },
         { 529500, 816500, DRX_BANDWIDTH_7MHZ }
       };
#endif /* FREQUENCY_PLAN_AUSTRALIA */

/*----------------------------------------------------------------------------*/
/* Chinese frequency plan                                                     */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_CHINA
#define FREQUENCY_PLAN_CHINA
#endif
#endif
#ifdef  FREQUENCY_PLAN_CHINA

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_China
#endif

static DRXFrequencyPlan_t freqPlan_China[] = {
         {  52500,  68500, DRX_BANDWIDTH_8MHZ },
         {  80000,  88000, DRX_BANDWIDTH_8MHZ },
         { 171000, 219000, DRX_BANDWIDTH_8MHZ },
         { 474000, 866000, DRX_BANDWIDTH_8MHZ }
       };
#endif /* FREQUENCY_PLAN_CHINA */

/*----------------------------------------------------------------------------*/
/* European frequency plan                                                    */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_EUROPE
#define FREQUENCY_PLAN_EUROPE
#endif
#endif
#ifdef  FREQUENCY_PLAN_EUROPE

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED  freqPlan_Europe
#endif

static DRXFrequencyPlan_t freqPlan_Europe[] = {
         {  50500,  64500, DRX_BANDWIDTH_7MHZ },
         { 177500, 226500, DRX_BANDWIDTH_7MHZ },
         { 474000, 858000, DRX_BANDWIDTH_8MHZ }
       };
#endif /* FREQUENCY_PLAN_EUROPE */

/*----------------------------------------------------------------------------*/
/* Indonesian frequency plan                                                  */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_INDONESIA
#define FREQUENCY_PLAN_INDONESIA
#endif
#endif
#ifdef  FREQUENCY_PLAN_INDONESIA

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_Indonesia
#endif

static DRXFrequencyPlan_t freqPlan_Indonesia[] = {
         {  46500,  46500, DRX_BANDWIDTH_7MHZ },
         {  57500,  64500, DRX_BANDWIDTH_7MHZ },
         { 177500, 226500, DRX_BANDWIDTH_7MHZ },
         { 474000, 858000, DRX_BANDWIDTH_8MHZ }
       };
#endif /* FREQUENCY_PLAN_INDONESIA */

/*----------------------------------------------------------------------------*/
/* Irish frequency plan                                                       */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_IRELAND
#define FREQUENCY_PLAN_IRELAND
#endif
#endif
#ifdef  FREQUENCY_PLAN_IRELAND

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_Ireland
#endif

static DRXFrequencyPlan_t freqPlan_Ireland[] = {
         {  48500,  64500, DRX_BANDWIDTH_8MHZ },
         { 178000, 218000, DRX_BANDWIDTH_8MHZ },
         { 474000, 858000, DRX_BANDWIDTH_8MHZ }
       };
#endif /* FREQUENCY_PLAN_IRELAND */

/*----------------------------------------------------------------------------*/
/* Italian frequency plan                                                     */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_ITALY
#define FREQUENCY_PLAN_ITALY
#endif
#endif
#ifdef  FREQUENCY_PLAN_ITALY

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_Italy
#endif

static DRXFrequencyPlan_t freqPlan_Italy[] = {
         {  56000,  56000, DRX_BANDWIDTH_7MHZ },
         {  64500,  64500, DRX_BANDWIDTH_7MHZ },
         {  84500,  84500, DRX_BANDWIDTH_7MHZ },
         { 177500, 177500, DRX_BANDWIDTH_7MHZ },
         { 186000, 186000, DRX_BANDWIDTH_7MHZ },
         { 194500, 194500, DRX_BANDWIDTH_7MHZ },
         { 203500, 203500, DRX_BANDWIDTH_7MHZ },
         { 212500, 226500, DRX_BANDWIDTH_7MHZ },
         { 474000, 858000, DRX_BANDWIDTH_8MHZ }
       };
#endif /* FREQUENCY_PLAN_ITALY */

/*----------------------------------------------------------------------------*/
/* Japanese frequency plan                                                    */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_JAPAN
#define FREQUENCY_PLAN_JAPAN
#endif
#endif
#ifdef  FREQUENCY_PLAN_JAPAN

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_Japan
#endif

static DRXFrequencyPlan_t freqPlan_Japan[] = {
         {  93000, 105000, DRX_BANDWIDTH_6MHZ },
         { 173000, 191000, DRX_BANDWIDTH_6MHZ },
         { 195000, 219000, DRX_BANDWIDTH_6MHZ },
         { 473000, 767000, DRX_BANDWIDTH_6MHZ }
       };
#endif /* FREQUENCY_PLAN_JAPAN */

/*----------------------------------------------------------------------------*/
/* New Zealand's frequency plan                                               */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_NEW_ZEALAND
#define FREQUENCY_PLAN_NEW_ZEALAND
#endif
#endif
#ifdef  FREQUENCY_PLAN_NEW_ZEALAND

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_New_Zealand
#endif

static DRXFrequencyPlan_t freqPlan_New_Zealand[] = {
         {  47500,  47500, DRX_BANDWIDTH_7MHZ },
         {  57500,  64500, DRX_BANDWIDTH_7MHZ },
         { 177500, 219500, DRX_BANDWIDTH_7MHZ },
         { 474000, 858000, DRX_BANDWIDTH_8MHZ }
       };
#endif /* FREQUENCY_PLAN_NEW_ZEALAND */

/*----------------------------------------------------------------------------*/
/* South African frequency plan                                               */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_SOUTH_AFRICA
#define FREQUENCY_PLAN_SOUTH_AFRICA
#endif
#endif
#ifdef  FREQUENCY_PLAN_SOUTH_AFRICA

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_South_Africa
#endif

static DRXFrequencyPlan_t freqPlan_South_Africa[] = {
         { 178000, 234000, DRX_BANDWIDTH_8MHZ },
         { 250000, 250000, DRX_BANDWIDTH_8MHZ },
         { 474000, 858000, DRX_BANDWIDTH_8MHZ }
       };
#endif /* FREQUENCY_PLAN_SOUTH_AFRICA */

/*----------------------------------------------------------------------------*/
/* USA terrestrial TV's frequency plan                                        */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_USA
#define FREQUENCY_PLAN_USA
#endif
#endif
#ifdef  FREQUENCY_PLAN_USA

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_USA
#endif

static DRXFrequencyPlan_t freqPlan_USA[] = {
         {  57000,  69000, DRX_BANDWIDTH_6MHZ, 2   },
         {  79000,  85000, DRX_BANDWIDTH_6MHZ, 5   },
         { 177000, 213000, DRX_BANDWIDTH_6MHZ, 7   },
         { 473000, 887000, DRX_BANDWIDTH_6MHZ, 14  }
       };
#endif /* FREQUENCY_PLAN_USA */

/*----------------------------------------------------------------------------*/
/* USA STD cable TV's frequency plan                                          */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_USA_CATV
#define FREQUENCY_PLAN_USA_CATV
#endif
#endif
#ifdef  FREQUENCY_PLAN_USA_CATV

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_USA_CATV
#endif

static DRXFrequencyPlan_t freqPlan_USA_CATV[] = {
         {  57000,  69000, DRX_BANDWIDTH_6MHZ, 2   },   /* [2,    4] */
         {  79000,  85000, DRX_BANDWIDTH_6MHZ, 5   },   /* [5,    6] */
         {  93000, 117000, DRX_BANDWIDTH_6MHZ, 95  },   /* [95,  99] */
         { 123000, 171000, DRX_BANDWIDTH_6MHZ, 14  },   /* [14,  22] */
         { 177000, 213000, DRX_BANDWIDTH_6MHZ, 7   },   /* [7,   13] */
         { 219000, 645000, DRX_BANDWIDTH_6MHZ, 23  },   /* [23,  94] */
         { 651000, 999000, DRX_BANDWIDTH_6MHZ, 100 }    /* [100,158] */
       };
#endif /* FREQUENCY_PLAN_USA_CATV */

/*----------------------------------------------------------------------------*/
/* USA IRC cable TV's frequency plan                                          */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_USA_IRC
#define FREQUENCY_PLAN_USA_IRC
#endif
#endif
#ifdef  FREQUENCY_PLAN_USA_IRC

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_USA_IRC
#endif

static DRXFrequencyPlan_t freqPlan_USA_IRC[] = {
         {  57000,  69000, DRX_BANDWIDTH_6MHZ, 2   },   /* [2,    4] */
         {  81000,  87000, DRX_BANDWIDTH_6MHZ, 5   },   /* [5,    6] Only difference from CATV */  
         {  93000, 117000, DRX_BANDWIDTH_6MHZ, 95  },   /* [95,  99] */
         { 123000, 171000, DRX_BANDWIDTH_6MHZ, 14  },   /* [14,  22] */
         { 177000, 213000, DRX_BANDWIDTH_6MHZ, 7   },   /* [7,   13] */
         { 219000, 645000, DRX_BANDWIDTH_6MHZ, 23  },   /* [23,  94] */
         { 651000, 999000, DRX_BANDWIDTH_6MHZ, 100 }    /* [100,158] */
       };
#endif /* FREQUENCY_PLAN_USA_IRC */

/*----------------------------------------------------------------------------*/
/* USA HRC cable TV's frequency plan                                          */
/*----------------------------------------------------------------------------*/

#ifdef  FREQUENCY_PLAN_ALL
#ifndef FREQUENCY_PLAN_USA_HRC
#define FREQUENCY_PLAN_USA_HRC
#endif
#endif
#ifdef  FREQUENCY_PLAN_USA_HRC

/* use FREQUENCY_PLAN_SELECTED to access the selected frequency plan (single plan only) */
#ifndef FREQUENCY_PLAN_SELECTED
#define FREQUENCY_PLAN_SELECTED freqPlan_USA_HRC
#endif

static DRXFrequencyPlan_t freqPlan_USA_HRC[] = {
         {  55753,  67753, DRX_BANDWIDTH_6MHZ, 2   },   /* [2,    4] */
         {  79754,  85754, DRX_BANDWIDTH_6MHZ, 5   },   /* [5,    6] */
         {  91755, 115755, DRX_BANDWIDTH_6MHZ, 95  },   /* [95,  99] */
         { 121756, 169756, DRX_BANDWIDTH_6MHZ, 14  },   /* [14,  22] */
         { 175759, 211759, DRX_BANDWIDTH_6MHZ, 7   },   /* [7,   13] */
         { 217761, 643761, DRX_BANDWIDTH_6MHZ, 23  },   /* [23,  94] */
         { 649782, 997782, DRX_BANDWIDTH_6MHZ, 100 }    /* [100,158] */
       };
#endif /* FREQUENCY_PLAN_USA_HRC */

/*-------------------------------------------------------------------------
THE END
-------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
#endif /*  __FREQUENCY_PLANS__ */
