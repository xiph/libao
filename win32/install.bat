echo off
rem $Id: install.bat,v 1.3 2002/07/19 08:34:42 msmith Exp $
rem
rem Install script for win32 ao
setlocal
rem ---------------------------------------------------------
rem Alter these settings for your environment
set SRCBASE=c:\src\codec\vorbis
set DLLPATH=c:\opt\bin
set PLUGINPATH=c:\Program Files\Common Files\Xiph\ao
rem ---------------------------------------------------------
echo on
copy %SRCBASE%\ao\win32\lib\ao_d.dll "%DLLPATH%"
copy %SRCBASE%\ao\win32\lib\ao.dll "%DLLPATH%"
copy %SRCBASE%\ao\win32\lib\mmsound_d.dll "%PLUGINPATH%"
rem copy %SRCBASE%\ao\win32\lib\mmsound.dll "%PLUGINPATH%"
rem copy %SRCBASE%\ao\win32\lib\dsound_d.dll "%PLUGINPATH%"
rem copy %SRCBASE%\ao\win32\lib\dsound.dll "%PLUGINPATH%"
endlocal
