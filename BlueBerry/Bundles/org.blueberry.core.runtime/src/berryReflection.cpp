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

#include "berryReflection.h"

#include "berryObject.h"

// Better name demangling for gcc
#if __GNUC__ > 3 || ( __GNUC__ == 3 && __GNUC_MINOR__ > 0 )
#define GCC_USEDEMANGLE
#endif

#ifdef GCC_USEDEMANGLE
#include <cstdlib>
#include <cxxabi.h>
#endif

#if defined(_WIN32) && defined(NDEBUG)
// exported from VC CRT
extern "C"
char * __unDName(char * outputString, const char * name, int maxStringLength,
                void * (* pAlloc )(size_t), void (* pFree )(void *),
                unsigned short disableFlags);
#endif

namespace berry {

namespace Reflection {

QString DemangleName(const char* mangledName)
{
  QString name(mangledName);
#ifdef GCC_USEDEMANGLE
  int status;
  char* unmangled = abi::__cxa_demangle(mangledName, 0, 0, &status);

  if(status == 0)
  {
    name = QString(unmangled);
    free(unmangled);
  }
#elif defined(_WIN32) && defined(NDEBUG)
  char * const unmangled = __unDName(0, mangledName, 0, malloc, free, 0x2800);
  if (unmangled)
  {
    QString unmangledName(unmangled);
    name = unmangledName.split(' ', QString::SkipEmptyParts).back();
    free(unmangled);
  }
#else
  name = name.split(' ', QString::SkipEmptyParts).back();
#endif
  return name;
}

#ifdef GetClassName
// clash with WinUser.h definition
#undef GetClassName
#endif

QString GetClassName(const Object* obj)
{
  return DemangleName(typeid(*const_cast<Object*>(obj)).name());
}

TypeInfo::Concept::~Concept(){}

template<>
struct TypeInfo::Model<EmptyType> : Concept {
  QString GetName() const { return QString(); }
  QList<TypeInfo> GetSuperclasses() const { return QList<TypeInfo>(); }
};

TypeInfo::TypeInfo()
  : m_Self(std::make_shared<Model<EmptyType> >())
{
}

bool TypeInfo::operator==(const TypeInfo& other) const
{
  return this->GetName() == other.GetName();
}

QString TypeInfo::GetName() const
{
  return m_Self->GetName();
}

QList<TypeInfo> TypeInfo::GetSuperclasses() const
{
  return m_Self->GetSuperclasses();
}

}

}
