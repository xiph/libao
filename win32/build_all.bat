rem echo off
rem $Id: build_all.bat,v 1.1 2001/09/05 19:09:57 cwolf Exp $
rem
rem Builds all targets.
rem
rem Only need to call example.mak, dependencies will build the other
rem targets.
rem
echo on
nmake -f example.mak CFG="example - Win32 Debug"
nmake -f example.mak CFG="example - Win32 Release"
