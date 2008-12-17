/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date: 2008-06-13 21:02:28 +0200 (Fr, 13 Jun 2008) $
Version:   $Revision: 14620 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef @UBUNDLE_NAMESPACE@_@UNormalizedPluginID@_DLL.H
#define @UBUNDLE_NAMESPACE@_@UNormalizedPluginID@_DLL.H


//
// The following block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the @NormalizedPluginID@_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
//@NormalizedPluginID@_EXPORTS functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
//
#if defined(_WIN32) && !defined(@PROJECT_STATIC_VAR@)
  #if defined(@NormalizedPluginID@_EXPORTS)
    #define @UBUNDLE_NAMESPACE@_@UDLL_EXPORT_IMPORT_DEFINE@ __declspec(dllexport)
  #else
    #define @UBUNDLE_NAMESPACE@_@UDLL_EXPORT_IMPORT_DEFINE@ __declspec(dllimport)  
  #endif
#endif


#if !defined(@UBUNDLE_NAMESPACE@_@UDLL_EXPORT_IMPORT_DEFINE@)
  #define @UBUNDLE_NAMESPACE@_@UDLL_EXPORT_IMPORT_DEFINE@
#endif

#endif /*@UBUNDLE_NAMESPACE@_@UNormalizedPluginID@_DLL.H*/
