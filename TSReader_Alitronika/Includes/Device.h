// Device.h: interface for the CDevice class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_DEVICE_H__302F6387_AD0E_4891_AE3E_59888C74FD4D__INCLUDED_)
#define AFX_DEVICE_H__302F6387_AD0E_4891_AE3E_59888C74FD4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "common.h"
#include <string>
#include "AT_APIDefs.h"


/** CDevice
 *	Wrapper class of the system device access functions.
 *	Overlapped I/O has not been implemented (yet...)
 */
class ATDV_API_API CDevice  
{
public:
	/** Open a device
	  *
	  */
	BOOL Open (char * pDeviceName);
	
	/** Get a human readable device name.
	 *	
	 */
	/** Close
	 *	Closes the device. No further access is possible.
	 *	The destructor automatically calls this function.
	 */
	void Close();

	/**	IoControl
	 *	Send a command code to the device.
	 *	@param	nCmd Command code to send to the device.
	 *	@param	pDataIn Pointer to the data to accompany the command code.
	 *	@param	nLenIn	Length of pDataIn in bytes.
	 *	@param	pDataOut Pointer to put received data at.
	 *	@param	nLenOut	IN: the max. length of pDataOut. OUT: the number of bytes copied to pDataOut.
	 *	@returns	TRUE on success, otherwise FALSE.
	 */
	BOOL IoControl(u32 nCmd, void *pDataIn, u32 nLenIn, void *pDataOut, u32 &nLenOut);

	/**	Write
	 *	Write data to the device.
	 *	@param	pData Pointer to the data to write.
	 *	@param	nLength	IN: The length of pData, OUT: The number of bytes written to the device.
	 *	@returns	TRUE on success, otherwise FALSE.
	 */
	BOOL Write(void *pData, u32 &nLength);

	/**	Read
	 *	@param	pData Pointer to memory to put the received data at.
	 *	@param	nLength IN: The max. length of pData, OUT: The number of bytes copied to pData.
	 *	@returns	TRUE on success, otherwise FALSE.
	 */
	BOOL Read(void *pData, u32 &nLength);

	/** Constructor
	 *	@param	pDeviceName A zero terminated string containing the name of the device to use.
	 */
	CDevice(void);

	/**	Destructor
	 */
	virtual ~CDevice();
	
	
#ifdef LINUX
	int m_hDevice;				///< Handle to the device.
#else
	HANDLE m_hDevice;				///< Handle to the device.
	LPOVERLAPPED m_pOverlapped;		///< Pointer to an overlapped I/O structure. Not used.
#endif
protected:
	//TSysMutex m_mutexDevice;
	
};

#endif // !defined(AFX_DEVICE_H__302F6387_AD0E_4891_AE3E_59888C74FD4D__INCLUDED_)
