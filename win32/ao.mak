# Microsoft Developer Studio Generated NMAKE File, Based on ao.dsp
!IF "$(CFG)" == ""
CFG=ao - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ao - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ao - Win32 Release" && "$(CFG)" != "ao - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ao.mak" CFG="ao - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ao - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ao - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "ao - Win32 Release"

OUTDIR=.\Release/dynlib
INTDIR=.\Release/dynlib

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\ao.dll"

!ELSE 

ALL : "dsound - Win32 Release" ".\lib\ao.dll"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"dsound - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ao_au.obj"
	-@erase "$(INTDIR)\ao_null.obj"
	-@erase "$(INTDIR)\ao_raw.obj"
	-@erase "$(INTDIR)\ao_wav.obj"
	-@erase "$(INTDIR)\audio_out.obj"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\dlfcn.obj"
	-@erase "$(INTDIR)\readdir.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\ao.exp"
	-@erase "$(OUTDIR)\ao.lib"
	-@erase ".\lib\ao.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../include" /I "../win32/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ao.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\ao.pdb" /machine:I386 /def:".\ao.def" /out:"lib/ao.dll" /implib:"$(OUTDIR)\ao.lib" 
DEF_FILE= \
	".\ao.def"
LINK32_OBJS= \
	"$(INTDIR)\ao_au.obj" \
	"$(INTDIR)\ao_null.obj" \
	"$(INTDIR)\ao_raw.obj" \
	"$(INTDIR)\ao_wav.obj" \
	"$(INTDIR)\audio_out.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\dlfcn.obj" \
	"$(INTDIR)\readdir.obj"

".\lib\ao.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
PostBuild_Desc=copy export lib
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "dsound - Win32 Release" ".\lib\ao.dll"
   echo on
	copy Release\dynlib\ao.lib .\lib
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "ao - Win32 Debug"

OUTDIR=.\Debug/dynlib
INTDIR=.\Debug/dynlib
# Begin Custom Macros
OutDir=.\Debug/dynlib
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\ao_d.dll" "$(OUTDIR)\ao.bsc"

!ELSE 

ALL : "dsound - Win32 Debug" ".\lib\ao_d.dll" "$(OUTDIR)\ao.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"dsound - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ao_au.obj"
	-@erase "$(INTDIR)\ao_au.sbr"
	-@erase "$(INTDIR)\ao_null.obj"
	-@erase "$(INTDIR)\ao_null.sbr"
	-@erase "$(INTDIR)\ao_raw.obj"
	-@erase "$(INTDIR)\ao_raw.sbr"
	-@erase "$(INTDIR)\ao_wav.obj"
	-@erase "$(INTDIR)\ao_wav.sbr"
	-@erase "$(INTDIR)\audio_out.obj"
	-@erase "$(INTDIR)\audio_out.sbr"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\config.sbr"
	-@erase "$(INTDIR)\dlfcn.obj"
	-@erase "$(INTDIR)\dlfcn.sbr"
	-@erase "$(INTDIR)\readdir.obj"
	-@erase "$(INTDIR)\readdir.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ao.bsc"
	-@erase "$(OUTDIR)\ao_d.exp"
	-@erase "$(OUTDIR)\ao_d.lib"
	-@erase "$(OUTDIR)\ao_d.pdb"
	-@erase ".\lib\ao_d.dll"
	-@erase ".\lib\ao_d.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../include" /I "../win32/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ao.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ao_au.sbr" \
	"$(INTDIR)\ao_null.sbr" \
	"$(INTDIR)\ao_raw.sbr" \
	"$(INTDIR)\ao_wav.sbr" \
	"$(INTDIR)\audio_out.sbr" \
	"$(INTDIR)\config.sbr" \
	"$(INTDIR)\dlfcn.sbr" \
	"$(INTDIR)\readdir.sbr"

"$(OUTDIR)\ao.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\ao_d.pdb" /debug /machine:I386 /def:".\ao.def" /out:"lib/ao_d.dll" /implib:"$(OUTDIR)\ao_d.lib" /pdbtype:sept 
DEF_FILE= \
	".\ao.def"
LINK32_OBJS= \
	"$(INTDIR)\ao_au.obj" \
	"$(INTDIR)\ao_null.obj" \
	"$(INTDIR)\ao_raw.obj" \
	"$(INTDIR)\ao_wav.obj" \
	"$(INTDIR)\audio_out.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\dlfcn.obj" \
	"$(INTDIR)\readdir.obj"

".\lib\ao_d.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\Debug/dynlib
# End Custom Macros

$(DS_POSTBUILD_DEP) : "dsound - Win32 Debug" ".\lib\ao_d.dll" "$(OUTDIR)\ao.bsc"
   echo on
	copy Debug\dynlib\ao_d.lib .\lib
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

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
!IF EXISTS("ao.dep")
!INCLUDE "ao.dep"
!ELSE 
!MESSAGE Warning: cannot find "ao.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ao - Win32 Release" || "$(CFG)" == "ao - Win32 Debug"
SOURCE=..\src\ao_au.c

!IF  "$(CFG)" == "ao - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "../include" /I "../win32/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ao_au.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../include" /I "../win32/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\ao_au.obj"	"$(INTDIR)\ao_au.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\src\ao_null.c

!IF  "$(CFG)" == "ao - Win32 Release"


"$(INTDIR)\ao_null.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"


"$(INTDIR)\ao_null.obj"	"$(INTDIR)\ao_null.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\ao_raw.c

!IF  "$(CFG)" == "ao - Win32 Release"


"$(INTDIR)\ao_raw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"


"$(INTDIR)\ao_raw.obj"	"$(INTDIR)\ao_raw.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\ao_wav.c

!IF  "$(CFG)" == "ao - Win32 Release"


"$(INTDIR)\ao_wav.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"


"$(INTDIR)\ao_wav.obj"	"$(INTDIR)\ao_wav.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\audio_out.c

!IF  "$(CFG)" == "ao - Win32 Release"


"$(INTDIR)\audio_out.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"


"$(INTDIR)\audio_out.obj"	"$(INTDIR)\audio_out.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\src\config.c

!IF  "$(CFG)" == "ao - Win32 Release"


"$(INTDIR)\config.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"


"$(INTDIR)\config.obj"	"$(INTDIR)\config.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\dlfcn.c

!IF  "$(CFG)" == "ao - Win32 Release"


"$(INTDIR)\dlfcn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"


"$(INTDIR)\dlfcn.obj"	"$(INTDIR)\dlfcn.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\src\readdir.c

!IF  "$(CFG)" == "ao - Win32 Release"


"$(INTDIR)\readdir.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "ao - Win32 Debug"


"$(INTDIR)\readdir.obj"	"$(INTDIR)\readdir.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

!IF  "$(CFG)" == "ao - Win32 Release"

"dsound - Win32 Release" : 
   cd "\src\codec\vorbis\ao\src\plugins\dsound"
   $(MAKE) /$(MAKEFLAGS) /F .\dsound.mak CFG="dsound - Win32 Release" 
   cd "..\..\..\win32"

"dsound - Win32 ReleaseCLEAN" : 
   cd "\src\codec\vorbis\ao\src\plugins\dsound"
   $(MAKE) /$(MAKEFLAGS) /F .\dsound.mak CFG="dsound - Win32 Release" RECURSE=1 CLEAN 
   cd "..\..\..\win32"

!ELSEIF  "$(CFG)" == "ao - Win32 Debug"

"dsound - Win32 Debug" : 
   cd "\src\codec\vorbis\ao\src\plugins\dsound"
   $(MAKE) /$(MAKEFLAGS) /F .\dsound.mak CFG="dsound - Win32 Debug" 
   cd "..\..\..\win32"

"dsound - Win32 DebugCLEAN" : 
   cd "\src\codec\vorbis\ao\src\plugins\dsound"
   $(MAKE) /$(MAKEFLAGS) /F .\dsound.mak CFG="dsound - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\..\..\win32"

!ENDIF 


!ENDIF 

