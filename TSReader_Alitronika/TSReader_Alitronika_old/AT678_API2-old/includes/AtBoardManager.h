// AtBoardManager.h: interface for the CAtBoardManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATBOARDMANAGER_H__70D54956_8DB3_414A_A1D0_7472CFE72F4F__INCLUDED_)
#define AFX_ATBOARDMANAGER_H__70D54956_8DB3_414A_A1D0_7472CFE72F4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class CAtBoardMap;

#ifdef ATDV_API_EXPORTS
#define EXPIMP_TEMPLATE 
#else
#define EXPIMP_TEMPLATE extern
#endif

#pragma warning (push)
#pragma warning (disable : 4231 4251)

EXPIMP_TEMPLATE template class ATDV_API_API std::vector<char*>;

/** @defgroup public_api	The ATDV_API exports.
 *	All interfacing is done through the following classes.
 *	See the Related pages and Examples show how to use them.
 */

/*@{*/
#include "ATBoard.h"
//class  CATBoard ;


/** 
 *	CAtDeviceList contains a list of device names. 
 *	@note All strings are deleted on destruction, so if one wants to 
 *  save a device name for later use: MAKE A COPY.
 */
class ATDV_API_API CAtDeviceList 
{
public:
			CAtDeviceList();
	virtual ~CAtDeviceList();

public:
	std::vector<char*> Names; ///< all the names of the devices found.
};

/** 
 *	The CAtBoardManager class manages the physical board <-> object relation.
 *	It assures that one physical board has only one instantiation of the CATBoard class.
 *	If an object is no longer referenced, it will be deleted.
 *	Also, if the single instance of the board manager is destroyed, all instances of 
 *	the class CATBoard are destroyed.\n
 *	All board objects are requested using their system device name. (which can be listed
 *	by the GetDeviceList method.
 */
class ATDV_API_API CAtBoardManager  
{
public:
			/** Release a CATBoard reference.\n
			 *	This method releases the previously acquired pointer to a CATBoard object.
			 *	After calling this method the given CATBoard * is no longer guaranteed to be valid.
			 *	@param	pBoard Pointer to the object to release.
			 */
			void				ReleaseBoard	(CATBoard* pBoard);
			
			/** Release all CATBoard reference.\n
			 *	This method releases all previously acquired pointers to CATBoard objects.
			 *	After calling this method non of the CATBoard pointers are valid.
			 */
			void				ReleaseAll	();
			
			/** Get a list of all device names.\n
			 *	Retrieves a list of names of all available devices.
			 *	@param DevList Container of the names list.
			 *	@see std::vector<>
			 */
			void				GetDeviceList	(CAtDeviceList& DevList);
			
			/** Get a CATBoard reference.\n
			 *	Request a pointer to a CATBoard instance for the given device.
			 *	Each successful call to GetBoard MUST be released by calling ReleaseBoard(CATBoard* pBoard). 
			 *	@param pDevName A null terminated string containing the requested device name.
			 *	@returns NULL pointer on failure; Otherwise a pointer to a CATBoard object.
			 *	@see ReleaseBoard(CATBoard* pBoard)
			 */
			CATBoard *			GetBoard		(char* pDevName);
	
			/** Returns the one and only instance of CAtBoardManager.
			 *	@returns a pointer to the CAtBoardManager instance.
			 *  @see Singleton design pattern.
			 */		
	static	CAtBoardManager*	Instance		();

//@internal
//@{
private:
	friend struct _instcontainer;
	virtual				~CAtBoardManager();
						CAtBoardManager	();
	CAtBoardMap			*m_pMapATBoards;  ///< a map of all the CATBoard instances indexed on their device name.
//@}
};

#pragma warning (pop)

/*@}*/

#endif // !defined(AFX_ATBOARDMANAGER_H__70D54956_8DB3_414A_A1D0_7472CFE72F4F__INCLUDED_)

