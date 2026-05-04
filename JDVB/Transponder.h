
#ifndef TRANSPONDER_H___
#define TRANSPONDER_H___

class Programm;
class TSSource;
class TSPidFilterQueue;
class TSPidFilter;
class TSPacket;
class TSPacketQueue;


typedef long HRESULT;

class Transponder
{
protected:
	unsigned long Frequency_;
	unsigned long SymbolRate_;
	unsigned long DiscardedPackets_;
	unsigned long ProcessedPackets_;
	bool Vertical_;
	Programm *HeadProgramm_;
	Programm *TailProgramm_;
	TSPidFilterQueue *Queue_;
	TSSource *Source_;
	Transponder *Next_;
	TSPidFilter *SDTFilter_;
	TSPacketQueue *PacketQueue_;
public:
	Transponder(const unsigned long Frequency, const unsigned long SymbolRate, const bool Vertical);

	virtual ~Transponder();
	HRESULT notifyTransponderChange(const bool Select);
	HRESULT insertProgramm(Programm * Prog);
	HRESULT deleteProgramm(Programm * Prog);
	HRESULT registerTSPidFilter(const unsigned short Pid, TSPidFilter *Filter);
	HRESULT unregisterTSPidFilter(TSPidFilter *Filter);

	HRESULT findProgrammBySID(const unsigned short SIDPid, Programm **Prog);
	HRESULT findProgrammByPMT(const unsigned short PMTPid, Programm **Prog);
	HRESULT findProgrammByName(const char *Name, Programm **Prog);

	void setNextTransponder(Transponder *Next);
	const bool dispatch(TSPacket *Packet);
	HRESULT deleteAllProgramm();
	const unsigned long getFrequency() const;
	const unsigned long getSymbolRate() const;
	const bool isVerticalPolarization()const;
	Transponder *getNextTransponder() const;
	HRESULT findQueue(const unsigned short Pid, TSPidFilterQueue **Queue);
	HRESULT removeQueue(TSPidFilterQueue *Queue);

	const unsigned long getNumOfDiscardedPackets() const;
	const unsigned long getNumOfProcessedPackets() const;


};

#endif