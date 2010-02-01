/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _IGTTOOLPAIRNAVIGATION_EXPORT_DLL_H_
#define _IGTTOOLPAIRNAVIGATION_EXPORT_DLL_H_


//
// The following block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the org_mitk_gui_qt_igttoolpairnavigation_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// org_mitk_gui_qt_igttoolpairnavigation_EXPORTS functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32) && !defined(MITK_STATIC)
  #if defined(org_mitk_gui_qt_igttoolpairnavigation_EXPORTS)
    #define IGTTOOLPAIRNAVIGATION_EXPORT __declspec(dllexport)
  #else
    #define IGTTOOLPAIRNAVIGATION_EXPORT __declspec(dllimport)
  #endif
#endif


#if !defined(IGTTOOLPAIRNAVIGATION_EXPORT)
  #define IGTTOOLPAIRNAVIGATION_EXPORT
#endif

#endif /*_IGTTOOLPAIRNAVIGATION_EXPORT_DLL_H_*/
