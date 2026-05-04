/**
* \file $Id: bsp_host.c,v 1.7 2005/10/13 10:16:50 jasper Exp $
*
* \brief Host and OS dependent functions
*
* \author Jasper Schrader
*
*
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

/*-----------------------------------------------------------------------------
INCLUDE FILES
----------------------------------------------------------------------------*/

#include "stdafx.h"
#include <stdlib.h>
#include "bsp_host.h"

/* Includes for the millisecond ticker */
#if defined(__CYGWIN__)
#include <sys/time.h>
#include <sys/types.h>
#elif defined(WINDOWS) || defined(_WIN32)
#include <windows.h>
#include <winsock.h>    /* For definition of "timeval" structure */
#include <sys/timeb.h>  /* For prototype of "_ftime()" */
#else
#error "Platform not supported by bsp_host."
#error "Re-implement DRXBSP_HST_Clock() or gettimeofday() routines."
*; /* Generate a fatal compiler error to make sure it stops here,
      this is necesarry because not all compilers stop after a #error. */
#endif

/*-----------------------------------------------------------------------------
VARIABLES
----------------------------------------------------------------------------*/

struct timeval tp_start;

/*-----------------------------------------------------------------------------
FUNCTIONS
----------------------------------------------------------------------------*/

#if defined(WINDOWS) || defined(_WIN32)

/**
* \fn int gettimeofday(struct timeval *tp, void *tzp)
* \brief  Emulation of gettimeofday() function from gcc.
* \return int
* \retval 0: succes
* \retval -1: failure
*
* tzp may be ignored.
* tv_sec field: seconds since midnight (00:00:00) 1 Jan 1970
* tv_usec field: fraction of a second in microseconds
*
* Resolution of _ftime seems to be 15 millisec, while gcc/gettimeofday()
* shows a resolution of 1 millisec.
*
*/
int gettimeofday(struct timeval *tp, void *tzp)
{
#ifndef _CH_
   struct _timeb  windowsTime;
#else
   struct timeb  windowsTime;
#endif /* _CH_ */

   if (tp == (struct timeval *) NULL)
   {
      return (-1);
   }

#ifndef _CH_
   _ftime(&windowsTime);
#else
   ftime (&windowsTime);
#endif /* _CH_ */

   tp->tv_sec  = windowsTime.time + windowsTime.timezone;
   tp->tv_usec = windowsTime.millitm * 1000;

   return(0);
}

#endif /*  defined(WINDOWS) || defined(_WIN32) */

/*-----------------------------------------------------------------------------
EXPORTED FUNCTIONS
----------------------------------------------------------------------------*/
/**
* \fn DRXBSP_HST_Init ( void )
* \brief  Function to initialise the host bsp module.
* \return DRXStatus_t.
* \retval DRX_STS_OK:    Successful initialization.
* \retval DRX_STS_ERROR: Something went wrong.
*
*/
DRXStatus_t DRXBSP_HST_Init( void )
{
   if ( gettimeofday( &tp_start, NULL) != 0 )
   {
      return DRX_STS_ERROR;
   }

   return (DRX_STS_OK);
}

/**
* \fn DRXBSP_HST_Term ( void )
* \brief  Function to terminate the host bsp module.
* \return DRXStatus_t.
* \retval DRX_STS_OK:    Successful termination.
* \retval DRX_STS_ERROR: Something went wrong.
*
*/
DRXStatus_t DRXBSP_HST_Term( void )
{
   return (DRX_STS_OK);
}


/**
* \fn DRXBSP_HST_Malloc( s )
* \brief  Function to allocate space.
* \param  nBytes: The number of bytes to allocate.
* \return A void pointer to the allocated memory area or NULL.
* \retval NULL:     malloc failed.
* \retval not NULL: malloc succeeded.
*
*/
void *DRXBSP_HST_Malloc( int nBytes )
{
   return( malloc((size_t) (nBytes)) );
}

/**
* \fn DRXBSP_HST_Free( s )
* \brief  Function to free space.
* \param  space:  space to free.
*
*/
void DRXBSP_HST_Free( void *space )
{
   if ( space != NULL )
   {
      free( space );
   }
   return;
}

/**
* \fn DRXBSP_HST_Clock( void )
* \brief  Function returning the value of a millisecond ticker.
*
* For cygwin/gcc the type of the tv_sec & tv_usec is time_t.
* For windows this type is long.
* To let this funcyion work both long or time_t must be double sizeof(u16_t).
* This is ok for cygwin/gcc and windows/MSVS C++ 6.0
*
*/
u32_t DRXBSP_HST_Clock( void )
{
   u32_t  ticker = 0;
   struct timeval tp_curr;

   if ( gettimeofday( &tp_curr, NULL) != 0 )
   {
      /* Error,
         no suitable error value available but to give some indication ... */
      return (-1);
   }

   /* The -1 & +1000000 are used to avoid a signed division */
   ticker = (u32_t)(( tp_curr.tv_sec - tp_start.tv_sec -1) *1000);
   ticker+= (u32_t)(( tp_curr.tv_usec -tp_start.tv_usec + 1000000)/1000);

   return ( ticker ); /* Millisec */
}


/* End of file */
