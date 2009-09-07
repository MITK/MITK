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

SOURCE=.\mitkIpPicAddT.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicClear.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicClone.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicCopyH.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicCopyS.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicDelT.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicElements.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicFree.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicGet.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicGetH.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicGetMem.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicGetS.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicGetT.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicInfo.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicNew.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicOldGet.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicOldGetH.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicOldGetS.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicPut.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicPutMem.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicPutS.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicQueryT.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicSize.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicTSVElements.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicTSVSize.c
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicType.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\mitkIpPic.h
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicAnnotation.h
# End Source File
# Begin Source File

SOURCE=.\mitkIpPicOldP.h
# End Source File
# Begin Source File

SOURCE=.\mitkIpTypes.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\win32\zlib.lib
# End Source File
# End Target
# End Project
