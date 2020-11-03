/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryObject.h"

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
#include "berryDebugUtil.h"
#endif

#include "berryLog.h"
#include "berryReflection.h"

#include <QDebug>

#include <list>
#include <memory>
#include <exception>

namespace berry
{

void Object::Delete()
{
  this->UnRegister();
}

#ifdef _WIN32
void*
Object
::operator new(size_t n)
{
  return new char[n];
}

void*
Object
::operator new[](size_t n)
{
  return new char[n];
}

void
Object
::operator delete(void* m)
{
  delete [] (char*)m;
}

void
Object
::operator delete[](void* m, size_t)
{
  delete [] (char*)m;
}
#endif

const char* Object::GetStaticClassName()
{
  return "berry::Object";
}

QString Object::GetClassName() const
{
  return Reflection::DemangleName(typeid(*this).name());
}

Reflection::TypeInfo Object::GetStaticTypeInfo()
{
  return Reflection::TypeInfo::New<Self>();
}

Reflection::TypeInfo Object::GetTypeInfo() const
{
  return Self::GetStaticTypeInfo();
}

QList<Reflection::TypeInfo> Object::GetStaticSuperclasses()
{
  return QList<Reflection::TypeInfo>();
}

QList<Reflection::TypeInfo> Object::GetSuperclasses() const
{
  return GetStaticSuperclasses();
}

QDebug Object::Print(QDebug os, Indent indent) const
{
  os = this->PrintHeader(os, indent);
  os = this->PrintSelf(os, indent.GetNextIndent());
  return this->PrintTrailer(os, indent);
}

QString Object::ToString() const
{
  QString str;
  QDebug ss(&str);
  this->Print(ss);
  return str;
}

uint Object::HashCode() const
{
  return qHash(this);
}

bool Object::operator<(const Object* o) const
{
  return this < o;
}

void Object::Register() const
{
  m_ReferenceCount.ref();
}

void Object::UnRegister(bool del) const
{
  if (!m_ReferenceCount.deref() && del)
  {
    delete this;
  }
}

void Object::SetReferenceCount(int ref)
{
  QMutexLocker lock(&m_ReferenceCountLock);
  m_ReferenceCount.store(ref);

  if (ref == 0)
  {
    delete this;
  }
}

bool Object::operator==(const Object* o) const
{
  return this == o;
}

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
unsigned int Object::GetTraceId() const
{
  return m_TraceId;
}

unsigned int& Object::GetTraceIdCounter() const
{
  static unsigned int traceId = 0;
  return traceId;
}
#endif

Object::Object() :
  m_ReferenceCount(0)
{
#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  unsigned int& id = GetTraceIdCounter();
  m_TraceId = ++id;
  DebugUtil::RegisterObject(this);
#endif
}

Object::~Object()
{
  /**
   * warn user if reference counting is on and the object is being referenced
   * by another object.
   */
  if (m_ReferenceCount.load() > 0)
  {
    // A general exception safety rule is that destructors should
    // never throw.  Something is wrong with a program that reaches
    // this point anyway.  Also this is the least-derived class so the
    // whole object has been destroyed by this point anyway.  Just
    // issue a warning.
    BERRY_WARN << "WARNING: In " __FILE__ ", line " << __LINE__ << "\n"
        << this->GetClassName() << " (" << this
        << "): Trying to delete object with non-zero reference count.";
  }

  /**
   * notifies the registered functions that the object is being destroyed
   */
  m_DestroyMessage.Send();

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  DebugUtil::UnregisterObject(this);
#endif
}

QDebug Object::PrintSelf(QDebug os, Indent Indent) const
{
  QString demangledName = Reflection::DemangleName(typeid(*this).name());
  os << Indent << "RTTI typeinfo:   " << demangledName << '\n';
  os << Indent << "Reference Count: " << m_ReferenceCount.load() << '\n';
  return os;
}

/**
 * Define a default print header for all objects.
 */
QDebug Object::PrintHeader(QDebug os, Indent Indent) const
{
  os << Indent << this->GetClassName() << " (" << this << ")\n";
  return os;
}

/**
 * Define a default print trailer for all objects.
 */
QDebug Object::PrintTrailer(QDebug os, Indent /*Indent*/) const
{
  return os;
}

// ============== Indent related implementations ==============

static const char blanks[41] = "                                        ";

Indent Indent::GetNextIndent()
{
  int Indent = m_Indent + 2;
  if (Indent > 40)
  {
    Indent = 40;
  }
  return Indent;
}

QDebug operator<<(QDebug os, const berry::Indent& ind)
{
  os.nospace() << blanks + (40 - ind.m_Indent);
  return os;
}

} // namespace berry


QDebug operator<<(QDebug os, const berry::Object& o)
{
  return o.Print(os);
}

QDebug operator<<(QDebug os, const berry::SmartPointer<const berry::Object>& o)
{
  return o->Print(os);
}

QDebug operator<<(QDebug os, const berry::SmartPointer<berry::Object>& o)
{
  return o->Print(os);
}

QTextStream& operator<<(QTextStream& os, const berry::Object& o)
{
  os << o.ToString();
  return os;
}

QTextStream& operator<<(QTextStream& os, const berry::SmartPointer<const berry::Object>& o)
{
  os << o->ToString();
  return os;
}

//QTextStream& operator<<(QTextStream& os, const berry::SmartPointer<berry::Object>& o)
//{
//  os << o->ToString();
//  return os;
//}

uint qHash(const berry::Object& o)
{
  return o.HashCode();
}
