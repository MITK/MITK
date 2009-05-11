/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-06-10 14:39:04 +0200 (Di, 10 Jun 2008) $
Version:   $Revision: 14573 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef MITKGUICOMMONPLUGINDLL_H_
#define MITKGUICOMMONPLUGINDLL_H_


//
// The following block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MITK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MITK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32)  // && !defined(MITK_STATIC)
  #if defined(org_mitk_gui_common_EXPORTS)
    #define MITK_GUI_COMMON_PLUGIN __declspec(dllexport)
  #else
    #define MITK_GUI_COMMON_PLUGIN __declspec(dllimport)  
  #endif
#endif


#if !defined(MITK_GUI_COMMON_PLUGIN)
  #define MITK_GUI_COMMON_PLUGIN
#endif

#endif /*MITKGUICOMMONPLUGINDLL_H_*/
