# Microsoft Developer Studio Project File - Name="libipFunc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libipFunc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libipFunc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libipFunc.mak" CFG="libipFunc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libipFunc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libipFunc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libipFunc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../DataStructures" /I "../../DataStructures/ipPic" /I ".." /I "../ipPic" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "lint" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\libipFunc.lib"

!ELSEIF  "$(CFG)" == "libipFunc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libipFunc___Win32_Debug"
# PROP BASE Intermediate_Dir "libipFunc___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "d:\users\ivo\c\ip++\ImageProject" /I "../../DataStructures" /I "../../DataStructures/ipPic" /I ".." /I "../ipPic" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "WIN32" /D "lint" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\lib\libipFuncD.lib"

!ENDIF 

# Begin Target

# Name "libipFunc - Win32 Release"
# Name "libipFunc - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\_BorderX.c
# End Source File
# Begin Source File

SOURCE=.\_DrawPoly.c
# End Source File
# Begin Source File

SOURCE=.\AddC.c
# End Source File
# Begin Source File

SOURCE=.\AddI.c
# End Source File
# Begin Source File

SOURCE=.\AddSl.c
# End Source File
# Begin Source File

SOURCE=.\And.c
# End Source File
# Begin Source File

SOURCE=.\Border.c
# End Source File
# Begin Source File

SOURCE=.\BorderX.c
# End Source File
# Begin Source File

SOURCE=.\Box.c
# End Source File
# Begin Source File

SOURCE=.\Box2d.c
# End Source File
# Begin Source File

SOURCE=.\Box3d.c
# End Source File
# Begin Source File

SOURCE=.\Canny.c
# End Source File
# Begin Source File

SOURCE=.\Close.c
# End Source File
# Begin Source File

SOURCE=.\CompressM.c
# End Source File
# Begin Source File

SOURCE=.\Conv.c
# End Source File
# Begin Source File

SOURCE=.\Convert.c
# End Source File
# Begin Source File

SOURCE=.\CopyTags.c
# End Source File
# Begin Source File

SOURCE=.\Curtosis.c
# End Source File
# Begin Source File

SOURCE=.\CurtosisR.c
# End Source File
# Begin Source File

SOURCE=.\Dila.c
# End Source File
# Begin Source File

SOURCE=.\DivC.c
# End Source File
# Begin Source File

SOURCE=.\DivI.c
# End Source File
# Begin Source File

SOURCE=.\DrawPoly.c
# End Source File
# Begin Source File

SOURCE=.\Equal.c
# End Source File
# Begin Source File

SOURCE=.\Ero.c
# End Source File
# Begin Source File

SOURCE=.\Error.c
# End Source File
# Begin Source File

SOURCE=.\Exp.c
# End Source File
# Begin Source File

SOURCE=.\ExtrC.c
# End Source File
# Begin Source File

SOURCE=.\Extrema.c
# End Source File
# Begin Source File

SOURCE=.\ExtrR.c
# End Source File
# Begin Source File

SOURCE=.\ExtrROI.c
# End Source File
# Begin Source File

SOURCE=.\ExtT.c
# End Source File
# Begin Source File

SOURCE=.\FillArea.c
# End Source File
# Begin Source File

SOURCE=.\Frame.c
# End Source File
# Begin Source File

SOURCE=.\GaussF.c
# End Source File
# Begin Source File

SOURCE=.\Grad.c
# End Source File
# Begin Source File

SOURCE=.\Grav.c
# End Source File
# Begin Source File

SOURCE=.\Hist.c
# End Source File
# Begin Source File

SOURCE=.\Histo.c
# End Source File
# Begin Source File

SOURCE=.\HitMiss.c
# End Source File
# Begin Source File

SOURCE=.\HitMissI.c
# End Source File
# Begin Source File

SOURCE=.\Inertia.c
# End Source File
# Begin Source File

SOURCE=.\Inv.c
# End Source File
# Begin Source File

SOURCE=.\ipGetANew.c
# End Source File
# Begin Source File

SOURCE=.\Label.c
# End Source File
# Begin Source File

SOURCE=.\Laplace.c
# End Source File
# Begin Source File

SOURCE=.\LevWind.c
# End Source File
# Begin Source File

SOURCE=.\LN.c
# End Source File
# Begin Source File

SOURCE=.\Log.c
# End Source File
# Begin Source File

SOURCE=.\MakePicName.c
# End Source File
# Begin Source File

SOURCE=.\Malloc.c
# End Source File
# Begin Source File

SOURCE=.\Mean.c
# End Source File
# Begin Source File

SOURCE=.\MeanC.c
# End Source File
# Begin Source File

SOURCE=.\MeanF.c
# End Source File
# Begin Source File

SOURCE=.\MeanR.c
# End Source File
# Begin Source File

SOURCE=.\MeanROI.c
# End Source File
# Begin Source File

SOURCE=.\Median.c
# End Source File
# Begin Source File

SOURCE=.\Morph.c
# End Source File
# Begin Source File

SOURCE=.\MultC.c
# End Source File
# Begin Source File

SOURCE=.\MultI.c
# End Source File
# Begin Source File

SOURCE=.\Norm.c
# End Source File
# Begin Source File

SOURCE=.\NormXY.c
# End Source File
# Begin Source File

SOURCE=.\Not.c
# End Source File
# Begin Source File

SOURCE=.\OpCl.c
# End Source File
# Begin Source File

SOURCE=.\Open.c
# End Source File
# Begin Source File

SOURCE=.\Or.c
# End Source File
# Begin Source File

SOURCE=.\Pot.c
# End Source File
# Begin Source File

SOURCE=.\Range.c
# End Source File
# Begin Source File

SOURCE=.\Rank.c
# End Source File
# Begin Source File

SOURCE=.\Refl.c
# End Source File
# Begin Source File

SOURCE=.\RegGrow.c
# End Source File
# Begin Source File

SOURCE=.\Roberts.c
# End Source File
# Begin Source File

SOURCE=.\Rotate.c
# End Source File
# Begin Source File

SOURCE=.\Scale.c
# End Source File
# Begin Source File

SOURCE=.\ScBl.c
# End Source File
# Begin Source File

SOURCE=.\ScFact.c
# End Source File
# Begin Source File

SOURCE=.\ScNN.c
# End Source File
# Begin Source File

SOURCE=.\SDev.c
# End Source File
# Begin Source File

SOURCE=.\SDevC.c
# End Source File
# Begin Source File

SOURCE=.\SDevR.c
# End Source File
# Begin Source File

SOURCE=.\SDevROI.c
# End Source File
# Begin Source File

SOURCE=.\Select.c
# End Source File
# Begin Source File

SOURCE=.\SelInv.c
# End Source File
# Begin Source File

SOURCE=.\SelMM.c
# End Source File
# Begin Source File

SOURCE=.\SetErrno.c
# End Source File
# Begin Source File

SOURCE=.\SetTag.c
# End Source File
# Begin Source File

SOURCE=.\Shp.c
# End Source File
# Begin Source File

SOURCE=.\Skewness.c
# End Source File
# Begin Source File

SOURCE=.\SkewnessR.c
# End Source File
# Begin Source File

SOURCE=.\Sobel.c
# End Source File
# Begin Source File

SOURCE=.\Sqrt.c
# End Source File
# Begin Source File

SOURCE=.\SubC.c
# End Source File
# Begin Source File

SOURCE=.\SubI.c
# End Source File
# Begin Source File

SOURCE=.\Thresh.c
# End Source File
# Begin Source File

SOURCE=.\Transpose.c
# End Source File
# Begin Source File

SOURCE=.\Var.c
# End Source File
# Begin Source File

SOURCE=.\VarC.c
# End Source File
# Begin Source File

SOURCE=.\VarR.c
# End Source File
# Begin Source File

SOURCE=.\VarROI.c
# End Source File
# Begin Source File

SOURCE=.\Window.c
# End Source File
# Begin Source File

SOURCE=.\WindowR.c
# End Source File
# Begin Source File

SOURCE=.\Xchange.c
# End Source File
# Begin Source File

SOURCE=.\ZeroCr.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gradient.h
# End Source File
# Begin Source File

SOURCE=.\ipArithmeticAndLogical.h
# End Source File
# Begin Source File

SOURCE=.\mitkIpFunc.h
# End Source File
# Begin Source File

SOURCE=.\mitkIpFuncP.h
# End Source File
# Begin Source File

SOURCE=.\ipGeometrical.h
# End Source File
# Begin Source File

SOURCE=.\ipLocal.h
# End Source File
# Begin Source File

SOURCE=.\ipMorphological.h
# End Source File
# Begin Source File

SOURCE=.\ipPointOperators.h
# End Source File
# Begin Source File

SOURCE=.\ipStatistical.h
# End Source File
# End Group
# End Target
# End Project
