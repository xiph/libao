# Microsoft Developer Studio Generated NMAKE File, Based on example.dsp
!IF "$(CFG)" == ""
CFG=example - Win32 Debug
!MESSAGE No configuration specified. Defaulting to example - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "example - Win32 Release" && "$(CFG)" != "example - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "example.mak" CFG="example - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "example - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "example - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "example - Win32 Release"

OUTDIR=.\Release/example
INTDIR=.\Release/example

!IF "$(RECURSE)" == "0" 

ALL : ".\Release\bin\dynamic\ao_example.exe"

!ELSE 

ALL : "dsound - Win32 Release" "ao - Win32 Release" ".\Release\bin\dynamic\ao_example.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"ao - Win32 ReleaseCLEAN" "dsound - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ao_example.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase ".\Release\bin\dynamic\ao_example.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "../include" /I "../win32/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\example.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\ao_example.pdb" /machine:I386 /out:"Release/bin/dynamic/ao_example.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ao_example.obj" \
	".\Release\dynlib\ao.lib"

".\Release\bin\dynamic\ao_example.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "example - Win32 Debug"

OUTDIR=.\Debug/example
INTDIR=.\Debug/example
# Begin Custom Macros
OutDir=.\Debug/example
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\Debug\bin\dynamic\ao_example.exe" "$(OUTDIR)\example.bsc"

!ELSE 

ALL : "dsound - Win32 Debug" "ao - Win32 Debug" ".\Debug\bin\dynamic\ao_example.exe" "$(OUTDIR)\example.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"ao - Win32 DebugCLEAN" "dsound - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\ao_example.obj"
	-@erase "$(INTDIR)\ao_example.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\ao_example.pdb"
	-@erase "$(OUTDIR)\example.bsc"
	-@erase ".\Debug\bin\dynamic\ao_example.exe"
	-@erase ".\Debug\bin\dynamic\ao_example.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "../include" /I "../win32/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\example.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ao_example.sbr"

"$(OUTDIR)\example.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ao_d.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\ao_example.pdb" /debug /machine:I386 /out:"Debug/bin/dynamic/ao_example.exe" /pdbtype:sept /libpath:"./lib" 
LINK32_OBJS= \
	"$(INTDIR)\ao_example.obj" \
	".\Debug\dynlib\ao_d.lib"

".\Debug\bin\dynamic\ao_example.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("example.dep")
!INCLUDE "example.dep"
!ELSE 
!MESSAGE Warning: cannot find "example.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "example - Win32 Release" || "$(CFG)" == "example - Win32 Debug"
SOURCE=..\doc\ao_example.c

!IF  "$(CFG)" == "example - Win32 Release"


"$(INTDIR)\ao_example.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "example - Win32 Debug"


"$(INTDIR)\ao_example.obj"	"$(INTDIR)\ao_example.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

!IF  "$(CFG)" == "example - Win32 Release"

"ao - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\ao.mak CFG="ao - Win32 Release" 
   cd "."

"ao - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\ao.mak CFG="ao - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "example - Win32 Debug"

"ao - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\ao.mak CFG="ao - Win32 Debug" 
   cd "."

"ao - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F .\ao.mak CFG="ao - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "example - Win32 Release"

"dsound - Win32 Release" : 
   cd "\src\codec\vorbis\ao\src\plugins\dsound"
   $(MAKE) /$(MAKEFLAGS) /F .\dsound.mak CFG="dsound - Win32 Release" 
   cd "..\..\..\win32"

"dsound - Win32 ReleaseCLEAN" : 
   cd "\src\codec\vorbis\ao\src\plugins\dsound"
   $(MAKE) /$(MAKEFLAGS) /F .\dsound.mak CFG="dsound - Win32 Release" RECURSE=1 CLEAN 
   cd "..\..\..\win32"

!ELSEIF  "$(CFG)" == "example - Win32 Debug"

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

