rem echo off
rem $Id: clean_dist.bat,v 1.1 2001/09/05 19:09:57 cwolf Exp $
rem
rem Only need to call example.mak, dependencies will build the other
rem targets.
echo on
del *.ncb
del *.plg
del ao.opt
del example.opt
rmdir /q /s lib
rmdir /q /s Debug
rmdir /q /s Release
