# Microsoft Developer Studio Generated NMAKE File, Based on decoder.dsp
!IF "$(CFG)" == ""
CFG=decoder - Win32 Debug
!MESSAGE No configuration specified. Defaulting to decoder - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "decoder - Win32 Release" && "$(CFG)" != "decoder - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "decoder.mak" CFG="decoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "decoder - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "decoder - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "decoder - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\decoder.exe" "$(OUTDIR)\decoder.bsc"


CLEAN :
	-@erase "$(INTDIR)\decfile.obj"
	-@erase "$(INTDIR)\decfile.sbr"
	-@erase "$(INTDIR)\decoder.obj"
	-@erase "$(INTDIR)\decoder.sbr"
	-@erase "$(INTDIR)\decopts.obj"
	-@erase "$(INTDIR)\decopts.sbr"
	-@erase "$(INTDIR)\rdopts.obj"
	-@erase "$(INTDIR)\rdopts.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc93dhtab.obj"
	-@erase "$(INTDIR)\vc93dhtab.sbr"
	-@erase "$(INTDIR)\vc9cropmv.obj"
	-@erase "$(INTDIR)\vc9cropmv.sbr"
	-@erase "$(INTDIR)\vc9deblock.obj"
	-@erase "$(INTDIR)\vc9deblock.sbr"
	-@erase "$(INTDIR)\vc9debug.obj"
	-@erase "$(INTDIR)\vc9debug.sbr"
	-@erase "$(INTDIR)\vc9dec.obj"
	-@erase "$(INTDIR)\vc9dec.sbr"
	-@erase "$(INTDIR)\vc9dec3dh.obj"
	-@erase "$(INTDIR)\vc9dec3dh.sbr"
	-@erase "$(INTDIR)\vc9dec3dhtab.obj"
	-@erase "$(INTDIR)\vc9dec3dhtab.sbr"
	-@erase "$(INTDIR)\vc9decbit.obj"
	-@erase "$(INTDIR)\vc9decbit.sbr"
	-@erase "$(INTDIR)\vc9decbitpl.obj"
	-@erase "$(INTDIR)\vc9decbitpl.sbr"
	-@erase "$(INTDIR)\vc9decbitpltab.obj"
	-@erase "$(INTDIR)\vc9decbitpltab.sbr"
	-@erase "$(INTDIR)\vc9decblk.obj"
	-@erase "$(INTDIR)\vc9decblk.sbr"
	-@erase "$(INTDIR)\vc9decblktab.obj"
	-@erase "$(INTDIR)\vc9decblktab.sbr"
	-@erase "$(INTDIR)\vc9decent.obj"
	-@erase "$(INTDIR)\vc9decent.sbr"
	-@erase "$(INTDIR)\vc9decmb.obj"
	-@erase "$(INTDIR)\vc9decmb.sbr"
	-@erase "$(INTDIR)\vc9decmbtab.obj"
	-@erase "$(INTDIR)\vc9decmbtab.sbr"
	-@erase "$(INTDIR)\vc9decmv.obj"
	-@erase "$(INTDIR)\vc9decmv.sbr"
	-@erase "$(INTDIR)\vc9decpic.obj"
	-@erase "$(INTDIR)\vc9decpic.sbr"
	-@erase "$(INTDIR)\vc9decpictab.obj"
	-@erase "$(INTDIR)\vc9decpictab.sbr"
	-@erase "$(INTDIR)\vc9decseq.obj"
	-@erase "$(INTDIR)\vc9decseq.sbr"
	-@erase "$(INTDIR)\vc9decslice.obj"
	-@erase "$(INTDIR)\vc9decslice.sbr"
	-@erase "$(INTDIR)\vc9deczz.obj"
	-@erase "$(INTDIR)\vc9deczz.sbr"
	-@erase "$(INTDIR)\vc9derivemv.obj"
	-@erase "$(INTDIR)\vc9derivemv.sbr"
	-@erase "$(INTDIR)\vc9gentab.obj"
	-@erase "$(INTDIR)\vc9gentab.sbr"
	-@erase "$(INTDIR)\vc9hrd.obj"
	-@erase "$(INTDIR)\vc9hrd.sbr"
	-@erase "$(INTDIR)\vc9interp.obj"
	-@erase "$(INTDIR)\vc9interp.sbr"
	-@erase "$(INTDIR)\vc9iquant.obj"
	-@erase "$(INTDIR)\vc9iquant.sbr"
	-@erase "$(INTDIR)\vc9itrans.obj"
	-@erase "$(INTDIR)\vc9itrans.sbr"
	-@erase "$(INTDIR)\vc9pred.obj"
	-@erase "$(INTDIR)\vc9pred.sbr"
	-@erase "$(INTDIR)\vc9predcbp.obj"
	-@erase "$(INTDIR)\vc9predcbp.sbr"
	-@erase "$(INTDIR)\vc9preddcac.obj"
	-@erase "$(INTDIR)\vc9preddcac.sbr"
	-@erase "$(INTDIR)\vc9predmv.obj"
	-@erase "$(INTDIR)\vc9predmv.sbr"
	-@erase "$(INTDIR)\vc9recon.obj"
	-@erase "$(INTDIR)\vc9recon.sbr"
	-@erase "$(INTDIR)\vc9scalemv.obj"
	-@erase "$(INTDIR)\vc9scalemv.sbr"
	-@erase "$(INTDIR)\vc9smooth.obj"
	-@erase "$(INTDIR)\vc9smooth.sbr"
	-@erase "$(INTDIR)\vc9tools.obj"
	-@erase "$(INTDIR)\vc9tools.sbr"
	-@erase "$(INTDIR)\vc9zztab.obj"
	-@erase "$(INTDIR)\vc9zztab.sbr"
	-@erase "$(OUTDIR)\decoder.bsc"
	-@erase "$(OUTDIR)\decoder.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W4 /GX /O2 /I "..\shared" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\decoder.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\decoder.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\decfile.sbr" \
	"$(INTDIR)\decoder.sbr" \
	"$(INTDIR)\decopts.sbr" \
	"$(INTDIR)\rdopts.sbr" \
	"$(INTDIR)\vc93dhtab.sbr" \
	"$(INTDIR)\vc9cropmv.sbr" \
	"$(INTDIR)\vc9deblock.sbr" \
	"$(INTDIR)\vc9debug.sbr" \
	"$(INTDIR)\vc9dec.sbr" \
	"$(INTDIR)\vc9dec3dh.sbr" \
	"$(INTDIR)\vc9dec3dhtab.sbr" \
	"$(INTDIR)\vc9decbit.sbr" \
	"$(INTDIR)\vc9decbitpl.sbr" \
	"$(INTDIR)\vc9decbitpltab.sbr" \
	"$(INTDIR)\vc9decblk.sbr" \
	"$(INTDIR)\vc9decblktab.sbr" \
	"$(INTDIR)\vc9decent.sbr" \
	"$(INTDIR)\vc9decmb.sbr" \
	"$(INTDIR)\vc9decmbtab.sbr" \
	"$(INTDIR)\vc9decmv.sbr" \
	"$(INTDIR)\vc9decpic.sbr" \
	"$(INTDIR)\vc9decpictab.sbr" \
	"$(INTDIR)\vc9decseq.sbr" \
	"$(INTDIR)\vc9decslice.sbr" \
	"$(INTDIR)\vc9deczz.sbr" \
	"$(INTDIR)\vc9derivemv.sbr" \
	"$(INTDIR)\vc9gentab.sbr" \
	"$(INTDIR)\vc9hrd.sbr" \
	"$(INTDIR)\vc9interp.sbr" \
	"$(INTDIR)\vc9iquant.sbr" \
	"$(INTDIR)\vc9itrans.sbr" \
	"$(INTDIR)\vc9pred.sbr" \
	"$(INTDIR)\vc9predcbp.sbr" \
	"$(INTDIR)\vc9preddcac.sbr" \
	"$(INTDIR)\vc9predmv.sbr" \
	"$(INTDIR)\vc9recon.sbr" \
	"$(INTDIR)\vc9scalemv.sbr" \
	"$(INTDIR)\vc9smooth.sbr" \
	"$(INTDIR)\vc9tools.sbr" \
	"$(INTDIR)\vc9zztab.sbr"

"$(OUTDIR)\decoder.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\decoder.pdb" /machine:I386 /out:"$(OUTDIR)\decoder.exe" 
LINK32_OBJS= \
	"$(INTDIR)\decfile.obj" \
	"$(INTDIR)\decoder.obj" \
	"$(INTDIR)\decopts.obj" \
	"$(INTDIR)\rdopts.obj" \
	"$(INTDIR)\vc93dhtab.obj" \
	"$(INTDIR)\vc9cropmv.obj" \
	"$(INTDIR)\vc9deblock.obj" \
	"$(INTDIR)\vc9debug.obj" \
	"$(INTDIR)\vc9dec.obj" \
	"$(INTDIR)\vc9dec3dh.obj" \
	"$(INTDIR)\vc9dec3dhtab.obj" \
	"$(INTDIR)\vc9decbit.obj" \
	"$(INTDIR)\vc9decbitpl.obj" \
	"$(INTDIR)\vc9decbitpltab.obj" \
	"$(INTDIR)\vc9decblk.obj" \
	"$(INTDIR)\vc9decblktab.obj" \
	"$(INTDIR)\vc9decent.obj" \
	"$(INTDIR)\vc9decmb.obj" \
	"$(INTDIR)\vc9decmbtab.obj" \
	"$(INTDIR)\vc9decmv.obj" \
	"$(INTDIR)\vc9decpic.obj" \
	"$(INTDIR)\vc9decpictab.obj" \
	"$(INTDIR)\vc9decseq.obj" \
	"$(INTDIR)\vc9decslice.obj" \
	"$(INTDIR)\vc9deczz.obj" \
	"$(INTDIR)\vc9derivemv.obj" \
	"$(INTDIR)\vc9gentab.obj" \
	"$(INTDIR)\vc9hrd.obj" \
	"$(INTDIR)\vc9interp.obj" \
	"$(INTDIR)\vc9iquant.obj" \
	"$(INTDIR)\vc9itrans.obj" \
	"$(INTDIR)\vc9pred.obj" \
	"$(INTDIR)\vc9predcbp.obj" \
	"$(INTDIR)\vc9preddcac.obj" \
	"$(INTDIR)\vc9predmv.obj" \
	"$(INTDIR)\vc9recon.obj" \
	"$(INTDIR)\vc9scalemv.obj" \
	"$(INTDIR)\vc9smooth.obj" \
	"$(INTDIR)\vc9tools.obj" \
	"$(INTDIR)\vc9zztab.obj"

"$(OUTDIR)\decoder.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "decoder - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\decoder.exe" "$(OUTDIR)\decoder.bsc"


CLEAN :
	-@erase "$(INTDIR)\decfile.obj"
	-@erase "$(INTDIR)\decfile.sbr"
	-@erase "$(INTDIR)\decoder.obj"
	-@erase "$(INTDIR)\decoder.sbr"
	-@erase "$(INTDIR)\decopts.obj"
	-@erase "$(INTDIR)\decopts.sbr"
	-@erase "$(INTDIR)\rdopts.obj"
	-@erase "$(INTDIR)\rdopts.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vc93dhtab.obj"
	-@erase "$(INTDIR)\vc93dhtab.sbr"
	-@erase "$(INTDIR)\vc9cropmv.obj"
	-@erase "$(INTDIR)\vc9cropmv.sbr"
	-@erase "$(INTDIR)\vc9deblock.obj"
	-@erase "$(INTDIR)\vc9deblock.sbr"
	-@erase "$(INTDIR)\vc9debug.obj"
	-@erase "$(INTDIR)\vc9debug.sbr"
	-@erase "$(INTDIR)\vc9dec.obj"
	-@erase "$(INTDIR)\vc9dec.sbr"
	-@erase "$(INTDIR)\vc9dec3dh.obj"
	-@erase "$(INTDIR)\vc9dec3dh.sbr"
	-@erase "$(INTDIR)\vc9dec3dhtab.obj"
	-@erase "$(INTDIR)\vc9dec3dhtab.sbr"
	-@erase "$(INTDIR)\vc9decbit.obj"
	-@erase "$(INTDIR)\vc9decbit.sbr"
	-@erase "$(INTDIR)\vc9decbitpl.obj"
	-@erase "$(INTDIR)\vc9decbitpl.sbr"
	-@erase "$(INTDIR)\vc9decbitpltab.obj"
	-@erase "$(INTDIR)\vc9decbitpltab.sbr"
	-@erase "$(INTDIR)\vc9decblk.obj"
	-@erase "$(INTDIR)\vc9decblk.sbr"
	-@erase "$(INTDIR)\vc9decblktab.obj"
	-@erase "$(INTDIR)\vc9decblktab.sbr"
	-@erase "$(INTDIR)\vc9decent.obj"
	-@erase "$(INTDIR)\vc9decent.sbr"
	-@erase "$(INTDIR)\vc9decmb.obj"
	-@erase "$(INTDIR)\vc9decmb.sbr"
	-@erase "$(INTDIR)\vc9decmbtab.obj"
	-@erase "$(INTDIR)\vc9decmbtab.sbr"
	-@erase "$(INTDIR)\vc9decmv.obj"
	-@erase "$(INTDIR)\vc9decmv.sbr"
	-@erase "$(INTDIR)\vc9decpic.obj"
	-@erase "$(INTDIR)\vc9decpic.sbr"
	-@erase "$(INTDIR)\vc9decpictab.obj"
	-@erase "$(INTDIR)\vc9decpictab.sbr"
	-@erase "$(INTDIR)\vc9decseq.obj"
	-@erase "$(INTDIR)\vc9decseq.sbr"
	-@erase "$(INTDIR)\vc9decslice.obj"
	-@erase "$(INTDIR)\vc9decslice.sbr"
	-@erase "$(INTDIR)\vc9deczz.obj"
	-@erase "$(INTDIR)\vc9deczz.sbr"
	-@erase "$(INTDIR)\vc9derivemv.obj"
	-@erase "$(INTDIR)\vc9derivemv.sbr"
	-@erase "$(INTDIR)\vc9gentab.obj"
	-@erase "$(INTDIR)\vc9gentab.sbr"
	-@erase "$(INTDIR)\vc9hrd.obj"
	-@erase "$(INTDIR)\vc9hrd.sbr"
	-@erase "$(INTDIR)\vc9interp.obj"
	-@erase "$(INTDIR)\vc9interp.sbr"
	-@erase "$(INTDIR)\vc9iquant.obj"
	-@erase "$(INTDIR)\vc9iquant.sbr"
	-@erase "$(INTDIR)\vc9itrans.obj"
	-@erase "$(INTDIR)\vc9itrans.sbr"
	-@erase "$(INTDIR)\vc9pred.obj"
	-@erase "$(INTDIR)\vc9pred.sbr"
	-@erase "$(INTDIR)\vc9predcbp.obj"
	-@erase "$(INTDIR)\vc9predcbp.sbr"
	-@erase "$(INTDIR)\vc9preddcac.obj"
	-@erase "$(INTDIR)\vc9preddcac.sbr"
	-@erase "$(INTDIR)\vc9predmv.obj"
	-@erase "$(INTDIR)\vc9predmv.sbr"
	-@erase "$(INTDIR)\vc9recon.obj"
	-@erase "$(INTDIR)\vc9recon.sbr"
	-@erase "$(INTDIR)\vc9scalemv.obj"
	-@erase "$(INTDIR)\vc9scalemv.sbr"
	-@erase "$(INTDIR)\vc9smooth.obj"
	-@erase "$(INTDIR)\vc9smooth.sbr"
	-@erase "$(INTDIR)\vc9tools.obj"
	-@erase "$(INTDIR)\vc9tools.sbr"
	-@erase "$(INTDIR)\vc9zztab.obj"
	-@erase "$(INTDIR)\vc9zztab.sbr"
	-@erase "$(OUTDIR)\decoder.bsc"
	-@erase "$(OUTDIR)\decoder.exe"
	-@erase "$(OUTDIR)\decoder.ilk"
	-@erase "$(OUTDIR)\decoder.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W4 /Gm /GX /ZI /Od /I "..\shared" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\decoder.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\decoder.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\decfile.sbr" \
	"$(INTDIR)\decoder.sbr" \
	"$(INTDIR)\decopts.sbr" \
	"$(INTDIR)\rdopts.sbr" \
	"$(INTDIR)\vc93dhtab.sbr" \
	"$(INTDIR)\vc9cropmv.sbr" \
	"$(INTDIR)\vc9deblock.sbr" \
	"$(INTDIR)\vc9debug.sbr" \
	"$(INTDIR)\vc9dec.sbr" \
	"$(INTDIR)\vc9dec3dh.sbr" \
	"$(INTDIR)\vc9dec3dhtab.sbr" \
	"$(INTDIR)\vc9decbit.sbr" \
	"$(INTDIR)\vc9decbitpl.sbr" \
	"$(INTDIR)\vc9decbitpltab.sbr" \
	"$(INTDIR)\vc9decblk.sbr" \
	"$(INTDIR)\vc9decblktab.sbr" \
	"$(INTDIR)\vc9decent.sbr" \
	"$(INTDIR)\vc9decmb.sbr" \
	"$(INTDIR)\vc9decmbtab.sbr" \
	"$(INTDIR)\vc9decmv.sbr" \
	"$(INTDIR)\vc9decpic.sbr" \
	"$(INTDIR)\vc9decpictab.sbr" \
	"$(INTDIR)\vc9decseq.sbr" \
	"$(INTDIR)\vc9decslice.sbr" \
	"$(INTDIR)\vc9deczz.sbr" \
	"$(INTDIR)\vc9derivemv.sbr" \
	"$(INTDIR)\vc9gentab.sbr" \
	"$(INTDIR)\vc9hrd.sbr" \
	"$(INTDIR)\vc9interp.sbr" \
	"$(INTDIR)\vc9iquant.sbr" \
	"$(INTDIR)\vc9itrans.sbr" \
	"$(INTDIR)\vc9pred.sbr" \
	"$(INTDIR)\vc9predcbp.sbr" \
	"$(INTDIR)\vc9preddcac.sbr" \
	"$(INTDIR)\vc9predmv.sbr" \
	"$(INTDIR)\vc9recon.sbr" \
	"$(INTDIR)\vc9scalemv.sbr" \
	"$(INTDIR)\vc9smooth.sbr" \
	"$(INTDIR)\vc9tools.sbr" \
	"$(INTDIR)\vc9zztab.sbr"

"$(OUTDIR)\decoder.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\decoder.pdb" /debug /machine:I386 /out:"$(OUTDIR)\decoder.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\decfile.obj" \
	"$(INTDIR)\decoder.obj" \
	"$(INTDIR)\decopts.obj" \
	"$(INTDIR)\rdopts.obj" \
	"$(INTDIR)\vc93dhtab.obj" \
	"$(INTDIR)\vc9cropmv.obj" \
	"$(INTDIR)\vc9deblock.obj" \
	"$(INTDIR)\vc9debug.obj" \
	"$(INTDIR)\vc9dec.obj" \
	"$(INTDIR)\vc9dec3dh.obj" \
	"$(INTDIR)\vc9dec3dhtab.obj" \
	"$(INTDIR)\vc9decbit.obj" \
	"$(INTDIR)\vc9decbitpl.obj" \
	"$(INTDIR)\vc9decbitpltab.obj" \
	"$(INTDIR)\vc9decblk.obj" \
	"$(INTDIR)\vc9decblktab.obj" \
	"$(INTDIR)\vc9decent.obj" \
	"$(INTDIR)\vc9decmb.obj" \
	"$(INTDIR)\vc9decmbtab.obj" \
	"$(INTDIR)\vc9decmv.obj" \
	"$(INTDIR)\vc9decpic.obj" \
	"$(INTDIR)\vc9decpictab.obj" \
	"$(INTDIR)\vc9decseq.obj" \
	"$(INTDIR)\vc9decslice.obj" \
	"$(INTDIR)\vc9deczz.obj" \
	"$(INTDIR)\vc9derivemv.obj" \
	"$(INTDIR)\vc9gentab.obj" \
	"$(INTDIR)\vc9hrd.obj" \
	"$(INTDIR)\vc9interp.obj" \
	"$(INTDIR)\vc9iquant.obj" \
	"$(INTDIR)\vc9itrans.obj" \
	"$(INTDIR)\vc9pred.obj" \
	"$(INTDIR)\vc9predcbp.obj" \
	"$(INTDIR)\vc9preddcac.obj" \
	"$(INTDIR)\vc9predmv.obj" \
	"$(INTDIR)\vc9recon.obj" \
	"$(INTDIR)\vc9scalemv.obj" \
	"$(INTDIR)\vc9smooth.obj" \
	"$(INTDIR)\vc9tools.obj" \
	"$(INTDIR)\vc9zztab.obj"

"$(OUTDIR)\decoder.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("decoder.dep")
!INCLUDE "decoder.dep"
!ELSE 
!MESSAGE Warning: cannot find "decoder.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "decoder - Win32 Release" || "$(CFG)" == "decoder - Win32 Debug"
SOURCE=.\decfile.c

"$(INTDIR)\decfile.obj"	"$(INTDIR)\decfile.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\decoder.c

"$(INTDIR)\decoder.obj"	"$(INTDIR)\decoder.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\decopts.c

"$(INTDIR)\decopts.obj"	"$(INTDIR)\decopts.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\shared\rdopts.c

"$(INTDIR)\rdopts.obj"	"$(INTDIR)\rdopts.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc93dhtab.c

"$(INTDIR)\vc93dhtab.obj"	"$(INTDIR)\vc93dhtab.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9cropmv.c

"$(INTDIR)\vc9cropmv.obj"	"$(INTDIR)\vc9cropmv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9deblock.c

"$(INTDIR)\vc9deblock.obj"	"$(INTDIR)\vc9deblock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9debug.c

"$(INTDIR)\vc9debug.obj"	"$(INTDIR)\vc9debug.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\vc9dec.c

"$(INTDIR)\vc9dec.obj"	"$(INTDIR)\vc9dec.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9dec3dh.c

"$(INTDIR)\vc9dec3dh.obj"	"$(INTDIR)\vc9dec3dh.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9dec3dhtab.c

"$(INTDIR)\vc9dec3dhtab.obj"	"$(INTDIR)\vc9dec3dhtab.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decbit.c

"$(INTDIR)\vc9decbit.obj"	"$(INTDIR)\vc9decbit.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decbitpl.c

"$(INTDIR)\vc9decbitpl.obj"	"$(INTDIR)\vc9decbitpl.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decbitpltab.c

"$(INTDIR)\vc9decbitpltab.obj"	"$(INTDIR)\vc9decbitpltab.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decblk.c

"$(INTDIR)\vc9decblk.obj"	"$(INTDIR)\vc9decblk.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decblktab.c

"$(INTDIR)\vc9decblktab.obj"	"$(INTDIR)\vc9decblktab.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decent.c

"$(INTDIR)\vc9decent.obj"	"$(INTDIR)\vc9decent.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decmb.c

"$(INTDIR)\vc9decmb.obj"	"$(INTDIR)\vc9decmb.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decmbtab.c

"$(INTDIR)\vc9decmbtab.obj"	"$(INTDIR)\vc9decmbtab.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decmv.c

"$(INTDIR)\vc9decmv.obj"	"$(INTDIR)\vc9decmv.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decpic.c

"$(INTDIR)\vc9decpic.obj"	"$(INTDIR)\vc9decpic.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decpictab.c

"$(INTDIR)\vc9decpictab.obj"	"$(INTDIR)\vc9decpictab.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decseq.c

"$(INTDIR)\vc9decseq.obj"	"$(INTDIR)\vc9decseq.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9decslice.c

"$(INTDIR)\vc9decslice.obj"	"$(INTDIR)\vc9decslice.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc9deczz.c

"$(INTDIR)\vc9deczz.obj"	"$(INTDIR)\vc9deczz.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=..\shared\vc9derivemv.c

"$(INTDIR)\vc9derivemv.obj"	"$(INTDIR)\vc9derivemv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9gentab.c

"$(INTDIR)\vc9gentab.obj"	"$(INTDIR)\vc9gentab.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9hrd.c

"$(INTDIR)\vc9hrd.obj"	"$(INTDIR)\vc9hrd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9interp.c

"$(INTDIR)\vc9interp.obj"	"$(INTDIR)\vc9interp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9iquant.c

"$(INTDIR)\vc9iquant.obj"	"$(INTDIR)\vc9iquant.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9itrans.c

"$(INTDIR)\vc9itrans.obj"	"$(INTDIR)\vc9itrans.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9pred.c

"$(INTDIR)\vc9pred.obj"	"$(INTDIR)\vc9pred.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9predcbp.c

"$(INTDIR)\vc9predcbp.obj"	"$(INTDIR)\vc9predcbp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9preddcac.c

"$(INTDIR)\vc9preddcac.obj"	"$(INTDIR)\vc9preddcac.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9predmv.c

"$(INTDIR)\vc9predmv.obj"	"$(INTDIR)\vc9predmv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9recon.c

"$(INTDIR)\vc9recon.obj"	"$(INTDIR)\vc9recon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9scalemv.c

"$(INTDIR)\vc9scalemv.obj"	"$(INTDIR)\vc9scalemv.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9smooth.c

"$(INTDIR)\vc9smooth.obj"	"$(INTDIR)\vc9smooth.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9tools.c

"$(INTDIR)\vc9tools.obj"	"$(INTDIR)\vc9tools.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\shared\vc9zztab.c

"$(INTDIR)\vc9zztab.obj"	"$(INTDIR)\vc9zztab.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

