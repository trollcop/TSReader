
#ifndef PROGRAMMSCANNER_H___
#define PROGRAMMSCANNER_H___


class TSPidFilter;
class Programm;
class TSSource;


typedef long HRESULT;

class ProgrammScanner
{
protected:
TSPidFilter *SDTFilter_;

public:
	ProgrammScanner();

	virtual ~ProgrammScanner();

	HRESULT scanTSSource(TSSource *Source, Programm **Prog);

	HRESULT scanProgramm(TSSource *Source,Programm *Prog);	

};


#endif