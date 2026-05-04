

JDVB Log List
------------------------


JDVB 0.0.2 29.Oct
->implemented File Setting Reader / Writer (file ChannelSetting.h / ChannelSetting.cpp)
->outsourced the channel scanning code from Transponder.cpp to ChannelScanner.cpp
->moved TSPidFilter storage code from Programm.cpp to DirectShowTSTarget.cpp
->erased DVBSource specific functions from TSSource.h
->extended TSPidFilter functionalty (notifyTransponderChange / isProcessingDone)
->beginning of Graphical Channel Management
Known Bugs:
->there seems to be TSPacket locking bug / TSPacket loss bug somewhere affecting the playback of video / audio
----------------------------------
JDVB 0.0.1 Primary release 27 Oct
->FTA watching
->broadcast of full TS Stream over IP