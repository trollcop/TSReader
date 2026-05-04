BDA Source for TSReader 31012005

A DVB-T BDA source for TSReader

Pieced together by Jeremy Quirke

INTRODUCTION

This is a basic plugin for the software TSReader that allows a BDA (Microsoft Broadcast Driver Architecture) capable DVB-T capture card to be used. 

I put this together from bits and pieces for my own personal use, but anyone willing to adapt it into something better is welcome.

REQUIREMENTS

- DVB-T card with BDA capable drivers
- TSReader 2.6 (recompile needed for 2.4)

INSTALLATION

The binary TSReader_BDASource-x.x.x.dll (depending on your version of TSReader) should be copied into the Sources folder. Or to be safe, recompile/link against the headers and libraries in SampleSource/inc and SampleSource/lib respectively.

The usage is simple, select the source, and enter the frequency.

KNOWN PROBLEMS

- The statistics from the IBDA_SignalStatistics interface do not appear to show signal locking status. 

- On some occasions, I have noticed tuning stops working? (fixed?)

- It would be good to have this adapted for generic BDA usage, I have tried to follow a 'generic' style of coding to make addition of ATSC etc support easy in future.

- I'm not experienced with DirectShow and BDA. I have probably done some things the wrong way, but this was originally a personal project and as long as it worked reasonably well I was not too concerned. If someone wishes to improve the code, be my guest.

CREDITS

- Ideas from BDA WebScheduler source (http://dvb-ws.sourceforge.net)

- Uses functions LoadFilter and ConnectFilters from MS DX9 SDK example code