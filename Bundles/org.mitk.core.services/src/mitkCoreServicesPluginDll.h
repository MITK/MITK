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

#ifndef MITKCORESERVICESPLUGINDLL_H_
#define MITKCORESERVICESPLUGINDLL_H_


//
// The following block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MITK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MITK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32)  // && !defined(MITK_STATIC)
  #if defined(org_mitk_core_services_EXPORTS)
    #define MITK_CORE_SERVICES_PLUGIN __declspec(dllexport)
  #else
    #define MITK_CORE_SERVICES_PLUGIN __declspec(dllimport)  
  #endif
#endif


#if !defined(MITK_CORE_SERVICES_PLUGIN)
  #define MITK_CORE_SERVICES_PLUGIN
#endif

#endif /*MITKCORESERVICESPLUGINDLL_H_*/
