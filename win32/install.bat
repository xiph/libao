rem echo off
rem $Id: install.bat,v 1.1 2001/09/05 19:09:57 cwolf Exp $
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
copy %SRCBASE%\ao\win32\lib\dsound_d.dll "%PLUGINPATH%"
copy %SRCBASE%\ao\win32\lib\dsound.dll "%PLUGINPATH%"
endlocal
