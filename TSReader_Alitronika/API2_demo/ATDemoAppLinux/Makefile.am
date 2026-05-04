# Filename		:	Makefile.am
# Description	:	Root Automake input file
# Author		:	Patrick Visser
# Date			:	2007-05-15
#

#
AUTOMAKE_OPTIONS		= foreign

INCLUDES				= -I/usr/local/include \
					  -I/usr/local/include/atdv_api3 \
					  -I/usr/local/include/atdv_api3/tuner
LDFLAGS					= -L/usr/local/lib

AM_CPPFLAGS				= -DLINUX

## Define separate programs
bin_PROGRAMS 			= ATDemoApp

include_HEADERS			= ../Src/ATDemo.h \
				  ../Src/ATBoards/ATBoardModDVBC.h \
				  ../Src/ATBoards/ATBoardModDVBT.h \
				  ../Src/ATBoards/ATBoardModRfIf.h \
				  ../Src/ATBoards/ATBoardPlayRec.h \
				  ../Src/ATBoards/ATBoardTunDVBCAnnexA.h \
				  ../Src/ATBoards/ATBoardTunDVBCAnnexB.h \
				  ../Src/ATBoards/ATBoardTunDVBT.h \
				  ../Src/Support/getopt.h \
				  ../Src/Support/msg.h \
				  ../Src/Support/tsrate.h \
				  ../Src/Support/strfunc.h \
				  ../Src/Sys/SysFunc.h


## Create alitest as a separate program
ATDemoApp_SOURCES			= ../Src/ATDemoMain.cpp  \
					  ../Src/ATDemoModDVBC.cpp \
					  ../Src/ATDemoModDVBT.cpp \
					  ../Src/ATDemoPlay.cpp \
					  ../Src/ATDemoRec.cpp \
					  ../Src/ATDemoTunDVBCAnnexA.cpp \
					  ../Src/ATDemoTunDVBCAnnexB.cpp \
					  ../Src/ATDemoTunDVBT.cpp \
					  ../Src/ATBoards/ATBoardModDVBC.cpp \
					  ../Src/ATBoards/ATBoardModDVBT.cpp \
					  ../Src/ATBoards/ATBoardModRfIf.cpp \
					  ../Src/ATBoards/ATBoardPlayRec.cpp \
					  ../Src/ATBoards/ATBoardTunDVBCAnnexA.cpp \
					  ../Src/ATBoards/ATBoardTunDVBCAnnexB.cpp \
					  ../Src/ATBoards/ATBoardTunDVBT.cpp \
					  ../Src/Support/getopt.c \
					  ../Src/Support/msg.cpp \
					  ../Src/Support/tsrate.cpp \
					  ../Src/Support/strfunc.cpp \
					  ../Src/Sys/SysFunc.cpp 

