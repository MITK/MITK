/*=========================================================================
 
Program:   openCherry Platform
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

#ifndef MITKQTDATAMANAGERDLL_H_
#define MITKQTDATAMANAGERDLL_H_

//
// The following block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the org_mitk_gui_qt_application_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MITK_QT_APP functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32) && !defined(CHERRY_STATIC)
  #if defined(org_mitk_gui_qt_datamanager_EXPORTS)
    #define MITK_QT_DATAMANAGER __declspec(dllexport)
  #else
    #define MITK_QT_DATAMANAGER __declspec(dllimport)  
  #endif
#endif

#if !defined(MITK_QT_DATAMANAGER)
  #define MITK_QT_DATAMANAGER
#endif

#endif /*MITKQTDATAMANAGERDLL_H_*/
