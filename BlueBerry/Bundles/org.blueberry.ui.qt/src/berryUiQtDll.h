/*=========================================================================
 
Program:   BlueBerry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef BERRYUIQTDLL_H_
#define BERRYUIQTDLL_H_


//
// The following block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MITK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MITK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32) && !defined(BERRY_STATIC)
  #if defined(org_blueberry_ui_qt_EXPORTS)
    #define BERRY_UI_QT __declspec(dllexport)
  #else
    #define BERRY_UI_QT __declspec(dllimport)  
  #endif
#endif


#if !defined(BERRY_UI_QT)
  #define BERRY_UI_QT
#endif

#endif /*BERRYUIQTDLL_H_*/
