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

#include "cherryWeakPointer.h"

#define cherryNameMacro(className) \
  virtual const char* GetClassName() const \
  { return #className; }\
  static const char* GetStaticClassName() \
  { return #className; }\

#define cherryManifestMacro(className, namespaze) \
  static const char* GetManifestName() \
  { return #namespaze #className; } \

#define cherryObjectMacro(className) \
  typedef className       Self; \
  typedef cherry::SmartPointer<Self> Pointer; \
  typedef cherry::SmartPointer<const Self>  ConstPointer; \
  typedef cherry::WeakPointer<Self> WeakPtr; \
  typedef cherry::WeakPointer<const Self> ConstWeakPtr; \
  cherryNameMacro(className) \


#define cherryInterfaceMacro(className, namespaze) \
  protected: className() {} \
  public: \
  cherryObjectMacro(className) \
  cherryManifestMacro(className, namespaze) \

#define cherrySimpleInterfaceMacro(className, namespaze) \
  protected: className() {} \
  public: \
  cherryNameMacro(className) \
  cherryManifestMacro(className, namespaze) \

#define cherryNewMacro(x) \
static Pointer New(void) \
{ \
  Pointer smartPtr = new x; \
  return smartPtr; \
} \


#ifndef CHERRY_NO_TYPESAFE_FLAGS

#include "cherryFlags.h"

#define CHERRY_DECLARE_FLAGS(_Flags, _Enum)\
typedef cherry::Flags<_Enum> _Flags;

#if defined _MSC_VER && _MSC_VER < 1300
# define CHERRY_DECLARE_INCOMPATIBLE_FLAGS(_Flags)
#else
# define CHERRY_DECLARE_INCOMPATIBLE_FLAGS(_Flags) \
inline cherry::IncompatibleFlag operator|(_Flags::enum_type f1, int f2) \
{ return cherry::IncompatibleFlag(int(f1) | f2); }
#endif

#define CHERRY_DECLARE_OPERATORS_FOR_FLAGS(_Flags) \
inline cherry::Flags<_Flags::enum_type> operator|(_Flags::enum_type f1, _Flags::enum_type f2) \
{ return cherry::Flags<_Flags::enum_type>(f1) | f2; } \
inline cherry::Flags<_Flags::enum_type> operator|(_Flags::enum_type f1, cherry::Flags<_Flags::enum_type> f2) \
{ return f2 | f1; } CHERRY_DECLARE_INCOMPATIBLE_FLAGS(_Flags)

#else /* CHERRY_NO_TYPESAFE_FLAGS */

#define CHERRY_DECLARE_FLAGS(_Flags, _Enum)\
typedef uint _Flags;
#define CHERRY_DECLARE_OPERATORS_FOR_FLAGS(_Flags)

#endif /* CHERRY_NO_TYPESAFE_FLAGS */


#endif /*__CHERRY_MACROS_H__*/
