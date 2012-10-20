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


/*
#define berryNewMacro(x) \
static Pointer New(void) \
{ \
  Pointer smartPtr(new x); \
  return smartPtr; \
} \

#define berryNewMacro1Param(x, type1) \
static Pointer New(type1 param1) \
{ \
  Pointer smartPtr(new x(param1)); \
  return smartPtr; \
} \

#define berryNewMacro2Param(x, type1, type2) \
static Pointer New(type1 param1, type2 param2) \
{ \
  Pointer smartPtr(new x(param1, param2)); \
  return smartPtr; \
} \

#define berryNewMacro3Param(x, type1, type2, type3) \
static Pointer New(type1 param1, type2 param2, type3 param3) \
{ \
  Pointer smartPtr (new x(param1, param2, param3)); \
  return smartPtr; \
}
*/


#define BERRY_REGISTER_EXTENSION_CLASS(_ClassType, _PluginContext)\
{\
  QString typeName = _PluginContext->getPlugin()->getSymbolicName();\
  typeName = (typeName + "_") + _ClassType::staticMetaObject.className();\
  ::berry::registerExtensionType<_ClassType>(typeName.toLatin1().data());\
}

#endif /*__BERRY_MACROS_H__*/
