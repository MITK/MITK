/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __BERRY_MACROS_H__
#define __BERRY_MACROS_H__

#include "berryWeakPointer.h"

#include "berryExtensionType.h"


#define berryObjectMacro(className)                      \
  typedef className       Self;                          \
  typedef berry::SmartPointer<Self> Pointer;             \
  typedef berry::SmartPointer<const Self>  ConstPointer; \
  typedef berry::WeakPointer<Self> WeakPtr;              \
  typedef berry::WeakPointer<const Self> ConstWeakPtr;   \
  static const char* GetStaticClassName()                \
  { return #className; }

#define BERRY_REGISTER_EXTENSION_CLASS(_ClassType, _PluginContext)\
{\
  QString typeName = _PluginContext->getPlugin()->getSymbolicName();\
  typeName = (typeName + "_") + _ClassType::staticMetaObject.className();\
  ::berry::registerExtensionType<_ClassType>(typeName.toLatin1().data());\
}

#endif /*__BERRY_MACROS_H__*/
