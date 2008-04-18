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

#ifndef CHERRYDLL_H_
#define CHERRYDLL_H_

//
// Ensure that CHERRY_DLL is default unless CHERRY_STATIC is defined
//
#if defined(_WIN32) && defined(_DLL)
  #if !defined(CHERRY_DLL) && !defined(CHERRY_STATIC)
    #define CHERRY_DLL
  #endif
#endif


//
// The following block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MITK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MITK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32) && defined(CHERRY_DLL)
  #if defined(CHERRY_EXPORTS)
    #define CHERRY_API __declspec(dllexport)
  #else
    #define CHERRY_API __declspec(dllimport)  
  #endif
#endif


#if !defined(CHERRY_API)
  #define CHERRY_API
#endif

#endif /*CHERRYDLL_H_*/
