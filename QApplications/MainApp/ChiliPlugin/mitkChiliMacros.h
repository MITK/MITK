/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _MITK_CHILI_MACROS__H_
#define _MITK_CHILI_MACROS__H_


// this macro is used to prevent unused variable warnings
// in case CHILI_PLUGIN_VERSION_CODE is not defined
#ifndef  CHILI_PLUGIN_VERSION_CODE
  #define mitkHideIfNoVersionCode(x) 
#else
  #define mitkHideIfNoVersionCode(x) x
#endif


#endif

