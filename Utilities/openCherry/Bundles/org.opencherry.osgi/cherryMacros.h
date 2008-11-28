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

#ifndef __CHERRY_MACROS_H__
#define __CHERRY_MACROS_H__

#include "cherryObject.h"
#include "cherrySmartPointer.h"

#define cherryClassMacro(className) \
  typedef className       Self; \
  typedef cherry::SmartPointer<Self> Pointer; \
  typedef cherry::SmartPointer<const Self>  ConstPointer; \
  virtual const char* GetNameOfClass() const \
  { return #className; }\

#define cherryManifestMacro(className, namespaze) \
  static const char* GetManifestName() \
  { return #namespaze #className; } \

#define cherryInterfaceMacro(className, namespaze) \
  cherryClassMacro(className) \
  cherryManifestMacro(className, namespaze) \



#define cherryNewMacro(x) \
static Pointer New(void) \
{ \
  Pointer smartPtr = new x; \
  return smartPtr; \
} \

#endif /*__CHERRY_MACROS_H__*/
