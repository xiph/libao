echo off
rem $Id: install.bat,v 1.2 2001/09/08 00:02:10 cwolf Exp $
rem
rem Install script for win32 ao
setlocal
rem ---------------------------------------------------------
rem Alter these settings for your environment
set SRCBASE=c:\src\codec\vorbis
set DLLPATH=c:\opt\bin
set PLUGINPATH=c:\Program Files\Common Files\Xiphophorus\ao
rem ---------------------------------------------------------
echo on
copy %SRCBASE%\ao\win32\lib\ao_d.dll "%DLLPATH%"
copy %SRCBASE%\ao\win32\lib\ao.dll "%DLLPATH%"
copy %SRCBASE%\ao\win32\lib\mmsound_d.dll "%PLUGINPATH%"
rem copy %SRCBASE%\ao\win32\lib\mmsound.dll "%PLUGINPATH%"
rem copy %SRCBASE%\ao\win32\lib\dsound_d.dll "%PLUGINPATH%"
rem copy %SRCBASE%\ao\win32\lib\dsound.dll "%PLUGINPATH%"
endlocal
