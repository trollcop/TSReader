/* ShowBoardNames()
 *	
 *	This functions creates a list of Alitronika device attached to the system.
 *
 *	First the list is updated. Then the first board name is read from the list.
 *	If the board name is not NULL, the board name is checked if it`s valid.
 *	If the board name is valid, the board is checked if it`s free to use.
 *	So there are 3 situations, Board is invallid, Board is free to use and Board is in use.
 *
 *	After the first board name is handled, the next board name is read from the list.
 *	This will be looped until no board name is found.
 */
void ShowBoardNames()
{
	char * pFriendlyName = NULL;
	char * pDevicesBoardName = NULL;

	CAtDeviceList iDevices;
	CAtBoardManager *pMan = CAtBoardManager::Instance();

	//Update device list.
	pMan->GetDeviceList(iDevices);

	int BoardPos = 0;

	//Get the first board name from the device list.
	 = 	iDevices.GetFirstBoardName();

	// Loop through all devices in the list.
	// Stop the loop when the board name = NULL indicating no board found 
	while(pDevicesBoardName)
	{
		//Get the friendly name of the board
		pFriendlyName = iDevices.GetFriendlyName(pDevicesBoardName);

		//Check if the boardname is 
		if(iDevices.IsBoardNameValid(pDevicesBoardName))
		{

			DWORD processid = 0;
			if(iDevices.IsBoardFree(pDevicesBoardName, processid))
			{
				// Board is free to use....
				//
				// Do your own handling here
			}
			else
			{
				// Board is NOT free to use...
				//
				// Do your own handling here
			}
		}
		else
		{
			// Board has errors, not detected correctly
			//
			// Do your own handling here
		}

		//Get the next board name from the device list
		pDevicesBoardName =	iDevices.GetNextBoardName();
	}

}

/* SelectBoard()
 *	
 *	Select the board to be used by the application.
 *	The board name is stored in the CurrentBoard class.
 *
 *	Use: 	CCurrBoard CurrBoard;
 *			CATBoard &ATBoard = CurrBoard;
 *	
 *	to get a handle to the board and start communication with it.
 *
 */
void SelectBoard(char * pDevicesBoardName)
{
	SetCurrentBoardName(pDevicesBoardName);
}