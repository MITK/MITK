# Microsoft Developer Studio Project File - Name="libipPic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libipPic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libipPic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libipPic.mak" CFG="libipPic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libipPic - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libipPic - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libipPic - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libipPic___Win32_Release"
# PROP BASE Intermediate_Dir "libipPic___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I ".." /I "win32" /I "$(QTDIR)\src\3rdparty\zlib" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "lint" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\libipPic.lib"

!ELSEIF  "$(CFG)" == "libipPic - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libipPic___Win32_Debug"
# PROP BASE Intermediate_Dir "libipPic___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I "$(QTDIR)\src\3rdparty\zlib" /I ".." /I "win32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "lint" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\libipPicD.lib"

!ENDIF 

# Begin Target

# Name "libipPic - Win32 Release"
# Name "libipPic - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ipEndian.c
# End Source File
# Begin Source File

SOURCE=.\ipError.c
# End Source File
# Begin Source File

SOURCE=.\ipFRead.c
# End Source File
# Begin Source File

SOURCE=.\ipFWrite.c
# End Source File
# Begin Source File

SOURCE=.\ipPicAddT.c
# End Source File
# Begin Source File

SOURCE=.\ipPicClear.c
# End Source File
# Begin Source File

SOURCE=.\ipPicClone.c
# End Source File
# Begin Source File

SOURCE=.\ipPicCopyH.c
# End Source File
# Begin Source File

SOURCE=.\ipPicCopyS.c
# End Source File
# Begin Source File

SOURCE=.\ipPicDelT.c
# End Source File
# Begin Source File

SOURCE=.\ipPicElements.c
# End Source File
# Begin Source File

SOURCE=.\ipPicFree.c
# End Source File
# Begin Source File

SOURCE=.\ipPicGet.c
# End Source File
# Begin Source File

SOURCE=.\ipPicGetH.c
# End Source File
# Begin Source File

SOURCE=.\ipPicGetMem.c
# End Source File
# Begin Source File

SOURCE=.\ipPicGetS.c
# End Source File
# Begin Source File

SOURCE=.\ipPicGetT.c
# End Source File
# Begin Source File

SOURCE=.\ipPicInfo.c
# End Source File
# Begin Source File

SOURCE=.\ipPicNew.c
# End Source File
# Begin Source File

SOURCE=.\ipPicOldGet.c
# End Source File
# Begin Source File

SOURCE=.\ipPicOldGetH.c
# End Source File
# Begin Source File

SOURCE=.\ipPicOldGetS.c
# End Source File
# Begin Source File

SOURCE=.\ipPicPut.c
# End Source File
# Begin Source File

SOURCE=.\ipPicPutMem.c
# End Source File
# Begin Source File

SOURCE=.\ipPicPutS.c
# End Source File
# Begin Source File

SOURCE=.\ipPicQueryT.c
# End Source File
# Begin Source File

SOURCE=.\ipPicSize.c
# End Source File
# Begin Source File

SOURCE=.\ipPicTSVElements.c
# End Source File
# Begin Source File

SOURCE=.\ipPicTSVSize.c
# End Source File
# Begin Source File

SOURCE=.\ipPicType.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ipPic.h
# End Source File
# Begin Source File

SOURCE=.\ipPicAnnotation.h
# End Source File
# Begin Source File

SOURCE=.\ipPicOldP.h
# End Source File
# Begin Source File

SOURCE=.\ipTypes.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\win32\zlib.lib
# End Source File
# End Target
# End Project
