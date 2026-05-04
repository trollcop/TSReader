#include "ChannelSetting.h"
#include "Programm.h"

#include <winerror.h>

//-------------------------------------------------------------------
ChannelSetting::ChannelSetting ()
{

}
//-------------------------------------------------------------------
ChannelSetting::~ChannelSetting ()
{


}
//-------------------------------------------------------------------
HRESULT ChannelSetting::readProgramm (Programm *Prog,FILE *fd, TransponderInfo *TransInfo)
{
	if(TransInfo == 0 || fd == 0 || Prog == 0)
		return E_FAIL;

	unsigned char ProgId = 0;

	while(ProgId != ProgrammId && feof(fd) == 0)
		fread(&ProgId,1,1,fd);
		

	if(feof(fd))
		return E_FAIL;

	if(ProgId != ProgrammId)
		return E_FAIL;

	unsigned char current_char = 0x0;
	fread(&current_char,1,1,fd);
	
	unsigned short Pid = 0x0;
	fread(&Pid,2,1,fd);
	Prog->setSIDPid (Pid);
	fread(&Pid,2,1,fd);
	Prog->setPMTPid (Pid);
	fread(&Pid,2,1,fd);
	Prog->setAudioPid (Pid,current_char & 0x4);
	fread(&Pid,2,1,fd);
	Prog->setVideoPid (Pid);
	fread(&Pid,2,1,fd);
	Prog->setTeleTextPid (Pid);
	fread(&Pid,2,1,fd);
	Prog->setECMPid(Pid);
	fread(&Pid,2,1,fd);
	Prog->setPCRPid(Pid);

	char Buffer[1024];
	int Position = 0;

	current_char = 0xFF;

	while(current_char != '\0' && feof(fd) == 0)
	{
		if(fread(&current_char,1,1,fd)	!= 1)
			break;
		Buffer[Position++] = current_char;
	}
	if(feof(fd))
		return E_FAIL;

	Prog->setFrequency (TransInfo->Frequency );
	Prog->setSymbolRate (TransInfo->SymbolRate );
	Prog->setPolarization (TransInfo->BitField1 & 0x80);


	Prog->setName (Buffer);

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ChannelSetting::writeProgramm(Programm *Prog, FILE *fd)
{
	if(Prog == 0 || fd == 0)
		return E_FAIL;

	fwrite(&ProgrammId,1,1,fd);
	
	unsigned char BitMask1 = 0x0;
	BitMask1 |= (Prog->isVerticalPolarization () & 0x1) << 7;
	//BitMask1 |= Prog->getProgrammType()<<4;
	//BitMask1|= Prog->isScrambled()<<3;
	//BitMask1|= Prog->isAudioAC3()<< 2;
	
	fwrite(&BitMask1,1,1,fd);

	unsigned short Pid = Prog->getSIDPid ();
	fwrite(&Pid,2,1,fd);
	Pid = Prog->getPMTPid ();
	fwrite(&Pid,2,1,fd);
	Pid = Prog->getAudioPid ();
	fwrite(&Pid,2,1,fd);
	Pid = Prog->getVideoPid ();
	fwrite(&Pid,2,1,fd);
	Pid = Prog->getTeleTextPid ();
	fwrite(&Pid,2,1,fd);
	Pid = Prog->getECMPid ();
	fwrite(&Pid,2,1,fd);
	Pid = Prog->getPCRPid ();
	fwrite(&Pid,2,1,fd);
	const char *Name = Prog->getName ();
	if(Name)
		fwrite(Name,1,strlen(Name)+1,fd);
	else
	{
		char end = ' ';
		fwrite(&end,1,1,fd);
		end = '\0';
		fwrite(&end,1,1,fd);
	}
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ChannelSetting::writeTransponder (TransponderInfo *TransInfo, FILE *fd)
{
	if(TransInfo == 0)
		return E_FAIL;

	fwrite(&TransponderId,1,1,fd);
	fwrite(&TransInfo->BitField1 ,1,1,fd);
	fwrite(&TransInfo->Frequency ,sizeof(unsigned long),1,fd);
	fwrite(&TransInfo->SymbolRate ,sizeof(unsigned long),1,fd);

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ChannelSetting::readTransponder (TransponderInfo *TransInfo,FILE *fd)
{
	if(TransInfo == 0 || fd == 0)
		return E_FAIL;

	unsigned char TransID = 0x00;

	while(TransID != TransponderId && feof(fd) == 0)
		fread(&TransID,1,1,fd);


	if(feof(fd))
		return E_FAIL;

	if(TransID != TransponderId)
		return E_FAIL;

	fread(&TransInfo->BitField1 ,1,1,fd);
	fread(&TransInfo->Frequency ,sizeof(unsigned long),1,fd);
	fread(&TransInfo->SymbolRate,sizeof(unsigned long),1,fd); 

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ChannelSetting::writeSatellite (const char *SatelliteName,const char *DeviceName,FILE *fd)
{
	if(SatelliteName == 0 || DeviceName == 0 || fd == 0)
		return E_FAIL;
	fwrite(&SatelliteId,1,1,fd);
	fwrite(SatelliteName,1,strlen(SatelliteName)+1,fd);
	fwrite(DeviceName,1,strlen(SatelliteName)+1,fd);
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ChannelSetting::readSatellite(SatelliteInfo *SatInfo,FILE *fd)
{
	if(SatInfo == 0 || fd == 0)
		return E_FAIL;

	unsigned char SATId = 0;

	while(SATId != SatelliteId && feof(fd) == 0)
		if(fread(&SATId,1,1,fd) != 1)
			break;

	if(feof(fd))
		return E_FAIL;

	if(SATId != SatelliteId)
		return E_FAIL;

	char Buffer[1024];
	int Position = 0;
	unsigned char char_read = 0xFF;
	
	while(char_read != '\0' && feof(fd) == 0)
	{
		if(fread(&char_read,1,1,fd)	!= 1)
			break;
		Buffer[Position++] = char_read;
	}
	if(feof(fd))
		return E_FAIL;

	delete [] SatInfo->Name ;
	SatInfo->Name = _strdup(Buffer);

	char_read = 0xFF;
	Position = 0;

	while(char_read != '\0' && feof(fd) == 0)
	{
		if(fread(&char_read,1,1,fd)	!= 1)
			break;
		Buffer[Position++] = char_read;
	}
	if(feof(fd))
		return E_FAIL;

	delete [] SatInfo->Device ;
	SatInfo->Device = _strdup(Buffer);

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ChannelSetting::read(Programm **Prog, const char *FileName)
{
	if(Prog == 0 || FileName == 0)
		return E_FAIL;

	FILE *fd = fopen(FileName,"r");
	if(fd == 0)
		return E_FAIL;


	SatelliteInfo *SatInfo = new SatelliteInfo();
	SatInfo->Device  = 0;
	SatInfo->Name = 0;
		
	if(FAILED(readSatellite(SatInfo,fd)))
	{
		delete SatInfo;
		fclose(fd);
		return E_FAIL;
	}

	TransponderInfo *TransInfo = new TransponderInfo();

	if(FAILED(readTransponder(TransInfo,fd)))
	{
		delete [] SatInfo->Name ;
		delete [] SatInfo->Device ;
		delete SatInfo;
		delete TransInfo;
		fclose(fd);
		return E_FAIL;
	}

	Programm *HeadProgramm = 0;
	Programm *TailProgramm = 0;
	unsigned char TableId = 0xFF;

	do
	{
		fread(&TableId,1,1,fd);

		switch(TableId)
		{
			case SatelliteId:
			{
				fseek(fd,-1l,SEEK_CUR);
				if(FAILED(readSatellite(SatInfo,fd)))
				{
					delete [] SatInfo->Name ;
					delete [] SatInfo->Device ;
					delete SatInfo;
					delete TransInfo;
					*Prog = HeadProgramm; 
					return S_OK;
				}
				
				break;
			}
			case TransponderId:
			{
				fseek(fd,-1l,SEEK_CUR);
				if(FAILED(readTransponder(TransInfo,fd)))
				{
					delete [] SatInfo->Name ;
					delete [] SatInfo->Device ;
					delete SatInfo;
					delete TransInfo;
					*Prog = HeadProgramm; 
					return S_OK;
				}
				break;
			}
			case ProgrammId:
			{
				Programm * Current = new Programm();
				fseek(fd,-1l,SEEK_CUR);
				if(FAILED(readProgramm(Current,fd,TransInfo)))
				{
					delete [] SatInfo->Name ;
					delete [] SatInfo->Device ;
					delete SatInfo;
					delete TransInfo;
					delete Current;				
					*Prog = HeadProgramm; 
					return S_OK;
				}
				if(HeadProgramm == 0)
					HeadProgramm = TailProgramm = Current;
				else
				{
					TailProgramm->setNextProgramm (Current);
					Current->setLastProgramm (TailProgramm);
					TailProgramm = Current;
				}
				break;
			}	
		
		}




	}
	while(feof(fd) == 0);


	*Prog = HeadProgramm;
	delete [] SatInfo->Device;
	delete [] SatInfo->Name ;
	delete SatInfo;
	delete TransInfo;
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ChannelSetting::write(Programm *Prog, const int NumOfProgramm, const char *FileName, const char *SatelliteName, const char * DeviceName)
{
	if(Prog == 0 || SatelliteName == 0 || DeviceName == 0)
		return E_FAIL;

	int ProgrammCount = 0;

	if(NumOfProgramm < 0)
		ProgrammCount = 0x7FFFFFFF;
	else
		ProgrammCount = NumOfProgramm;

	FILE *fd = 0;

	fd = fopen(FileName,"a");

	if(fd == 0)
		return E_FAIL;

	if(FAILED(writeSatellite(SatelliteName,DeviceName,fd)))
	{
		fclose(fd);
		return E_FAIL;
	}

	//satellite header has been written, lets write our other stuff

	Programm *CurrentProgramm = Prog;
	TransponderInfo *TransInfo = new TransponderInfo();
	TransInfo->TableId = TransponderId;
	TransInfo->Frequency = Prog->getFrequency ();
	TransInfo->SymbolRate = Prog->getSymbolRate ();
	TransInfo->BitField1 = 0;	
	TransInfo->BitField1 |= (Prog->isVerticalPolarization ()& 0x1) << 7;

	if(FAILED(writeTransponder(TransInfo,fd)))
	{
		delete TransInfo;
		fclose(fd);
		return E_FAIL;
	}


	do
	{
		if(CurrentProgramm->getFrequency () != TransInfo->Frequency ||
		   CurrentProgramm->getSymbolRate () != TransInfo->SymbolRate ||
		   CurrentProgramm->isVerticalPolarization () != ((TransInfo->BitField1 & 0x80)>>7))
		{
			TransInfo->Frequency = CurrentProgramm->getFrequency ();
			TransInfo->SymbolRate = CurrentProgramm->getSymbolRate ();
			TransInfo->BitField1 = 0;
			TransInfo->BitField1 != (CurrentProgramm->isVerticalPolarization () & 0x1) << 7;

			if(FAILED(writeTransponder(TransInfo,fd)))
			{
				delete TransInfo;
				fclose(fd);
				return E_FAIL;
			}
		}			

		if(FAILED(writeProgramm(CurrentProgramm,fd)))
		{
			delete TransInfo;
			fclose(fd);
			return E_FAIL;
		}

		CurrentProgramm = CurrentProgramm->getNextProgramm ();
		ProgrammCount;
	}
	while(CurrentProgramm && ProgrammCount > 0);

	delete TransInfo;
	fclose(fd);
	return S_OK;
}
//-------------------------------------------------------------------