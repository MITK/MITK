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

#ifndef __OSGI_FRAMEWORK_MACROS_H__
#define __OSGI_FRAMEWORK_MACROS_H__

#include "WeakPointer.h"

#include <string>
#include <algorithm>

#define osgiNameMacro(className) \
  virtual const char* GetClassName() const \
  { return #className; }\
  static const char* GetStaticClassName() \
  { return #className; }\

#define osgiManifestMacro(className) \
  static std::string GetManifestName() \
  {\
    std::string str = #className; \
    std::string::iterator b = str.begin(); \
    std::string::iterator e = std::remove(b, str.end(), ':'); \
    return std::string(b, e); \
  }\

#define osgiObjectMacro(className) \
  typedef className       Self; \
  typedef ::osgi::framework::SmartPointer<Self> Pointer; \
  typedef ::osgi::framework::SmartPointer<const Self>  ConstPointer; \
  typedef ::osgi::framework::WeakPointer<Self> WeakPtr; \
  typedef ::osgi::framework::WeakPointer<const Self> ConstWeakPtr; \
  osgiNameMacro(className) \

/**
 * Use this macro instead of osgiObjectMacro if you have a template class
 * with multiple arguments. Declare the Self typedef yourself and supply
 * as className somthing like MyTemplate<A B ...> to avoid the comma
 */
#define osgiObjectMacroT(className) \
  typedef ::osgi::framework::SmartPointer<Self> Pointer; \
  typedef ::osgi::framework::SmartPointer<const Self>  ConstPointer; \
  typedef ::osgi::framework::WeakPointer<Self> WeakPtr; \
  typedef ::osgi::framework::WeakPointer<const Self> ConstWeakPtr; \
  osgiNameMacro(className) \


#define osgiInterfaceMacro(className) \
  public: \
  osgiObjectMacro(className) \
  osgiManifestMacro(className) \



#ifndef OSGI_NO_TYPESAFE_FLAGS

#include "Flags.h"

#define OSGI_DECLARE_FLAGS(_Flags, _Enum)\
typedef osgi::framework::Flags<_Enum> _Flags;

#if defined _MSC_VER && _MSC_VER < 1300
# define OSGI_DECLARE_INCOMPATIBLE_FLAGS(_Flags)
#else
# define OSGI_DECLARE_INCOMPATIBLE_FLAGS(_Flags) \
inline osgi::framework::IncompatibleFlag operator|(_Flags::enum_type f1, int f2) \
{ return osgi::framework::IncompatibleFlag(int(f1) | f2); }
#endif

#define OSGI_DECLARE_OPERATORS_FOR_FLAGS(_Flags) \
inline cherry::Flags<_Flags::enum_type> operator|(_Flags::enum_type f1, _Flags::enum_type f2) \
{ return cherry::Flags<_Flags::enum_type>(f1) | f2; } \
inline cherry::Flags<_Flags::enum_type> operator|(_Flags::enum_type f1, cherry::Flags<_Flags::enum_type> f2) \
{ return f2 | f1; } OSGI_DECLARE_INCOMPATIBLE_FLAGS(_Flags)

#else /* OSGI_NO_TYPESAFE_FLAGS */

#define OSGI_DECLARE_FLAGS(_Flags, _Enum)\
typedef uint _Flags;
#define OSGI_DECLARE_OPERATORS_FOR_FLAGS(_Flags)

#endif /* OSGI_NO_TYPESAFE_FLAGS */


#endif /*__OSGI_FRAMEWORK_MACROS_H__*/
