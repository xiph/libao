# Microsoft Developer Studio Generated NMAKE File, Based on dsound.dsp
!IF "$(CFG)" == ""
CFG=dsound - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dsound - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dsound - Win32 Release" && "$(CFG)" != "dsound - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dsound.mak" CFG="dsound - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dsound - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dsound - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dsound - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\win32\lib\dsound.dll"


CLEAN :
	-@erase "$(INTDIR)\ao_dsound.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dsound.exp"
	-@erase "..\..\..\win32\lib\dsound.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\..\include" /I "..\..\..\win32\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dsound.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\dsound.pdb" /machine:I386 /def:".\dsound.def" /out:"../../../win32/lib/dsound.dll" /implib:"$(OUTDIR)\dsound.lib" 
DEF_FILE= \
	".\dsound.def"
LINK32_OBJS= \
	"$(INTDIR)\ao_dsound.obj"

"..\..\..\win32\lib\dsound.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dsound - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\..\win32\lib\dsound_d.dll"


CLEAN :
	-@erase "$(INTDIR)\ao_dsound.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dsound_d.exp"
	-@erase "$(OUTDIR)\dsound_d.pdb"
	-@erase "..\..\..\win32\lib\dsound_d.dll"
	-@erase "..\..\..\win32\lib\dsound_d.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\win32\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dsound.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\dsound_d.pdb" /debug /machine:I386 /def:".\dsound.def" /out:"../../../win32/lib/dsound_d.dll" /implib:"$(OUTDIR)\dsound_d.lib" /pdbtype:sept 
DEF_FILE= \
	".\dsound.def"
LINK32_OBJS= \
	"$(INTDIR)\ao_dsound.obj"

"..\..\..\win32\lib\dsound_d.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dsound.dep")
!INCLUDE "dsound.dep"
!ELSE 
!MESSAGE Warning: cannot find "dsound.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dsound - Win32 Release" || "$(CFG)" == "dsound - Win32 Debug"
SOURCE=.\ao_dsound.c

"$(INTDIR)\ao_dsound.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

