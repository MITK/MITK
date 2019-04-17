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

#include "berryReflection.h"
#include "berryExtensionType.h"

class QStringList;

#define berryArgGlue(x, y) x y
#define berryArgCount(_1,_2,_3,_4,_5,_6,_7,_8,count,...) count
#define berryExpandArgs(args) berryArgCount args
#define berryCountArgsMax8(...) berryExpandArgs((__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define berryOverloadMacro2(name, count) name##count
#define berryOverloadMacro1(name, count) berryOverloadMacro2(name, count)
#define berryOverloadMacro(name, count) berryOverloadMacro1(name, count)
#define berryCallOverload(name, ...) berryArgGlue(berryOverloadMacro(name, berryCountArgsMax8(__VA_ARGS__)), (__VA_ARGS__))

#define berryObjectMacro(...) berryCallOverload(berryObjectMacro, __VA_ARGS__)

#define berryObjectTypeInfo(...)                                         \
  static ::berry::Reflection::TypeInfo GetStaticTypeInfo()               \
  { return ::berry::Reflection::TypeInfo::New<Self>(); }                 \
  ::berry::Reflection::TypeInfo GetTypeInfo() const override             \
  { return Self::GetStaticTypeInfo(); }                                  \
  /*typedef ::berry::Reflection::TypeList<__VA_ARGS__> SuperclassTypes;*/\
  static QList< ::berry::Reflection::TypeInfo> GetStaticSuperclasses()   \
  { return ::berry::Reflection::GetSuperclasses<Self>(); }               \
  QList< ::berry::Reflection::TypeInfo> GetSuperclasses() const override \
  { return Self::GetStaticSuperclasses(); }

#define berryObjectMacro1(className)                                     \
  typedef className       Self;                                          \
  typedef berry::SmartPointer<Self> Pointer;                             \
  typedef berry::SmartPointer<const Self>  ConstPointer;                 \
  typedef berry::WeakPointer<Self> WeakPtr;                              \
  typedef berry::WeakPointer<const Self> ConstWeakPtr;                   \
  static const char* GetStaticClassName()                                \
  { return #className; }

#define berryObjectMacro2(className, super1) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1)
#define berryObjectMacro3(className, super1, super2) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1, super2)
#define berryObjectMacro4(className, super1, super2, super3) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1, super2, super3)
#define berryObjectMacro5(className, super1, super2, super3, super4) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1, super2, super3, super4)
#define berryObjectMacro6(className, super1, super2, super3, super4, super5) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1, super2, super3, super4, super5)
#define berryObjectMacro7(className, super1, super2, super3, super4, super5, super6) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1, super2, super3, super4, super5, super6)
#define berryObjectMacro8(className, super1, super2, super3, super4, super5, super6, super7) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1, super2, super3, super4, super5, super6, super7)
#define berryObjectMacro9(className, super1, super2, super3, super4, super5, super6, super7, super8) \
  berryObjectMacro1(className) \
  berryObjectTypeInfo(super1, super2, super3, super4, super5, super6, super7, super8)

#define BERRY_REGISTER_EXTENSION_CLASS(_ClassType, _PluginContext)        \
{                                                                         \
  Q_UNUSED(_PluginContext)                                                \
  QString typeName = _ClassType::staticMetaObject.className();            \
  ::berry::registerExtensionType<_ClassType>(typeName.toLatin1().data()); \
}

#endif /*__BERRY_MACROS_H__*/
