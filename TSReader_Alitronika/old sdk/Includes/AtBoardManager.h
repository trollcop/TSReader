// AtBoardManager.h: interface for the CAtBoardManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATBOARDMANAGER_H__70D54956_8DB3_414A_A1D0_7472CFE72F4F__INCLUDED_)
#define AFX_ATBOARDMANAGER_H__70D54956_8DB3_414A_A1D0_7472CFE72F4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _LINUX
#pragma warning (push)
#pragma warning (disable : 4231 4251 4275)
#endif

#include <vector>
#include <map>

class CAtBoardMap;

#ifdef ATDV_API_EXPORTS
#define EXPIMP_TEMPLATE 
#else
#define EXPIMP_TEMPLATE extern
#endif

#include "common.h"
#include "AT_APIDefs.h"


#ifdef WIN32
   EXPIMP_TEMPLATE template class ATDV_API_API std::vector<char*>;
//	template class std::vector<char*>;
#endif /* WIN32 */

/** @defgroup public_api	The ATDV_API exports.
  * All interfacing is done through the following classes.
  * See the Related pages and Examples show how to use them.
  */

/*@{*/
#include "ATBoard.h"


/** \brief CAtDeviceList contains a list of device names.
  *
  * CAtDeviceList contains a list of device names.\n
  * Information about the use of a board (by another application or
  * part of this application) is available. Using this information, the
  * programmer may prevent conflicts when boards are accessed.
  * @note All strings are deleted on destruction, so if one wants to
  * save a device name for later use: MAKE A COPY.
  */

   
class ATDV_API_API CAtDeviceList
{
    public:
	CAtDeviceList();
	virtual ~CAtDeviceList();


	/** \brief Get the number of items in the list
	  *	
	  * This function returns the number of device in the list. 
	  * This includes all device, also the board in use and the
	  * boards not correctly recognized.
	  * 
	  * @returns the number of devices.
	  */
	int		GetListSize();

	/** \brief Get the number of items Free to use.
	  *	
	  * This function returns the number of device in the list 
	  * that are free to use. Boards used in other applications
	  * or not working properly are NOT counted.
	  *  
	  * @returns the number of devices free to use.
	  */
	int		GetFreeSize();

	/** \brief Get the first boardname from the device list.
	  *	
	  * This function returns the first board in the device list.
	  * Use GetNextBoardName() to the next board.
	  *   
	  * @returns NULL when no boards are available, else the name of the board.
	  * @see Device Dialog example
	  */
	char *	GetFirstBoardName();
	
	/** \brief Get the next boardname from the device list.
	  *	
	  * This function returns the next board in the device list.
	  * Before this function is called, use GetFirstBoardName() to set
	  * the list at the first board in the list.
	  *   
	  * @returns NULL when no boards are available, else the name of the board.
	  * @see Device Dialog example  
	  */
	char *	GetNextBoardName();

	/** \brief Get the first free boardname from the device list.
	  *	
	  * This function returns the first free board in the device list.
	  * This does not affect the GetFirstBoardName and GetNextBoardName functions.
	  *   
	  * @returns NULL when no board is available, else the name of the board.
	  */
	char *	GetFirstFreeBoardName();

	/** \brief Get the Friendly name from the boardname
	  *	
	  * This function returns the Friendly name of the board.
	  *   
	  * @returns NULL when failed, else the friendly name of the board.
	  */
	char *  GetFriendlyName(char * boardname);

	/** \brief Get the Friendly name from the boardname
	  *	
	  * This function returns the Friendly name of the board.
	  *   
	  * @returns NULL when failed, else the friendly name of the board.
	  */
	char *  GetBoardPositionString(char * boardname);

	/** \brief Get the serialnumber from the board
	*	
	* This function returns the serialnumber of the board.
	*   
	* @returns NULL when failed, else the serialnumber of the board.
	*/
	__int64 GetSerialNumber(char * boardname);


	/** \brief Get the the device type from the board
	*	
	* This function returns the device type of the board.
	*   
	* @returns NULL when failed, else the device type of the board.
	*/
	DeviceTypeEnum CAtDeviceList::GetDeviceType(char * boardname);


	/** \brief Check if the board is free to use.
	  *	
	  *	Returns TRUE when the board is free to use.
	  * FALSE if the board is in use by an other application or the
	  * board is not working properly.
	  * 
	  * If the board is used by an other application, the process id 
	  * of this application is send back.
	  *
	  * @returns TRUE when the board is free, else FALSE.
	  */
	BOOL	IsBoardFree(char * boardname, DWORD & processid);

	/** \brief Mark the board that it is used by this appliaction
	  *	
	  * This functions marks the board that it`s in use / not used by this application.
	  * The board state is set TRUE when the boar is used, FALSE if the board is no
	  * longer needed.
	  *    
	  * @returns FALSE when function fails, else TRUE
	  */
	BOOL	SetBoardUsed(char * boardname, BOOL state);

	/** \brief Check if the board is still in the device list.
	  *	
	  * This functions searches for the board in the device list.
	  *	If the board is still in the list, the return is TRUE,
	  * if the device name is not found FALSE.
	  *	    
	  * @returns TRUE when found, else FALSE
	  */
	BOOL	CheckBoard(char * boardname);

	/** \brief Check if the boardname is valid to use.
	  * 
	  * It can happen that windows does not detect the board.
	  * This is checked walking through the Hubs attached to
	  * the system. When a board is detected that is not in
	  * the list, it is added with a part of the boardname,
	  * and with an invalid flag so the application can report this.
	  * 
	  * @returns a TRUE when devicename is valid to use.
	  */
	BOOL	IsBoardNameValid(char * boardname);
};

/** \brief Manages the physical board <-> object relation.
  *
  * The CAtBoardManager class manages the physical board <-> object relation.
  * It assures that one physical board has only one instantiation of the CATBoard class.
  * If an object is no longer referenced, it will be deleted.
  * Also, if the single instance of the board manager is destroyed, all instances of
  * the class CATBoard are destroyed.\n
  * All board objects are requested using their system device name
  * (which can be listed by the GetDeviceList method.
  */
class ATDV_API_API CAtBoardManager  
{
    public:
	/** \brief Release a CATBoard reference.
	  *
	  *Release a CATBoard reference.\n
	  * This method releases the previously acquired pointer to a CATBoard object.
	  * After calling this method the given CATBoard * is no longer guaranteed to be valid.
	  * @param	pBoard Pointer to the object to release.
	  */
	void				ReleaseBoard	(CATBoard* pBoard);

	/** \brief Release all CATBoard reference.
	  *
	  *Release all CATBoard reference.\n
	  * This method releases all previously acquired pointers to CATBoard objects.
	  * After calling this method non of the CATBoard pointers are valid.
	  */
	void				ReleaseAll	();

	/** \brief Get a list of all device names.
	  *
	  * Get a list of all device names.\n
	  * Retrieves a list of names of all available devices.
	  * @param DevList Container of the names list.
	  * 
	  */
	void				GetDeviceList	(CAtDeviceList& DevList);

	/** \brief Get a CATBoard reference.
	  *
	  * Get a CATBoard reference.\n
	  * Request a pointer to a CATBoard instance for the given device.
	  * Each successful call to GetBoard MUST be released by calling ReleaseBoard(CATBoard* pBoard).
	  * @param pDevName A null terminated string containing the requested device name.
	  * @returns NULL pointer on failure; Otherwise a pointer to a CATBoard object.
	  * @see ReleaseBoard(CATBoard* pBoard)
	  */
	CATBoard *			GetBoard		(char* pDevName);
	
	/** \brief Get the pointer to the BoardManager.
	  * 
	  * Returns the one and only instance of CAtBoardManager.
	  * @returns a pointer to the CAtBoardManager instance.
	  * @see Singleton design pattern.
	  */
	static	CAtBoardManager*	Instance		();
	
	//@internal
	//@{
    private:
	friend struct _instcontainer;
	~CAtBoardManager();
	CAtBoardManager	();
	CAtBoardMap			*m_pMapATBoards;  ///< a map of all the CATBoard instances indexed on their device name.
//@}
};

#ifdef WIN32
#pragma warning (pop)
#endif


/*@}*/

#endif // !defined(AFX_ATBOARDMANAGER_H__70D54956_8DB3_414A_A1D0_7472CFE72F4F__INCLUDED_)

