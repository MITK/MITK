/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREFLECTION_H
#define BERRYREFLECTION_H

#include <org_blueberry_core_runtime_Export.h>

#include <QString>
#include <QList>

#include <iostream>
#include <memory>

#ifdef GetClassName
// clash with WinUser.h definition
#undef GetClassName
#endif

namespace berry {

class Object;

namespace Reflection {

org_blueberry_core_runtime_EXPORT QString DemangleName(const char* typeName);

org_blueberry_core_runtime_EXPORT QString GetClassName(const Object* obj);

template<typename T>
QString GetClassName()
{
  return DemangleName(typeid(T).name());
}

class TypeInfo;

template<typename T>
QList<TypeInfo> GetSuperclasses();

struct EmptyType {};

template<
  typename T0=EmptyType,
  typename T1=EmptyType,
  typename T2=EmptyType,
  typename T3=EmptyType,
  typename T4=EmptyType,
  typename T5=EmptyType,
  typename T6=EmptyType,
  typename T7=EmptyType,
  typename T8=EmptyType,
  typename T9=EmptyType
> struct TypeList;

template<
  typename T0,
  typename T1,
  typename T2,
  typename T3,
  typename T4,
  typename T5,
  typename T6,
  typename T7,
  typename T8,
  typename T9
> struct TypeList
{
  typedef T0 head;
  typedef TypeList<T1,T2,T3,T4,T5,T6,T7,T8,T9> tail;
  enum
  {
    length = tail::length+1
  };
};

template<>
struct TypeList<EmptyType, EmptyType, EmptyType, EmptyType, EmptyType,
                EmptyType, EmptyType, EmptyType, EmptyType, EmptyType>
{
  enum
  {
    length = 0
  };
};

// MapReduce
template<
  typename TL,
  template <typename> class Map,
  template <typename> class Reduce
>
struct MapReduce
{
  typedef typename Map<char>::type map_type;
  typedef typename Reduce<map_type>::result type;

  static type& value(type& result)
  {
    return MapReduce<typename TL::tail, Map, Reduce>::value(Reduce<map_type>::value(result, Map<typename TL::head>::value()));
  }

  static type value()
  {
    type result;
    return MapReduce<typename TL::tail, Map, Reduce>::value(Reduce<map_type>::value(result, Map<typename TL::head>::value()));
  }
};

template<
  template <typename> class Map,
  template <typename> class Reduce
>
struct MapReduce<TypeList<>, Map, Reduce>
{
  typedef typename Map<char>::type map_type;
  typedef typename Reduce<map_type>::result type;

  static type value()
  {
    return type();
  }

  static type& value(type& result)
  {
    return result;
  }
};

class org_blueberry_core_runtime_EXPORT TypeInfo
{
public:

  TypeInfo();

  template<typename T>
  explicit TypeInfo(T* /*dummy*/)
    : m_Self(std::make_shared<Model<T> >())
  {}

  bool operator==(const TypeInfo& other) const;

  template<typename T>
  static TypeInfo New()
  { T* dummy = nullptr; return TypeInfo(dummy); }

  QString GetName() const;
  QList<TypeInfo> GetSuperclasses() const;


private:

  struct org_blueberry_core_runtime_EXPORT Concept {
    virtual ~Concept();
    virtual QString GetName() const = 0;
    virtual QList<TypeInfo> GetSuperclasses() const = 0;
  };

  template<typename T>
  struct Model : Concept {
    QString GetName() const override
    { return GetClassName<T>(); }

    QList<TypeInfo> GetSuperclasses() const override;
  };

  std::shared_ptr<const Concept> m_Self;
};

template<typename T>
struct MapToTypeInfo
{
  typedef TypeInfo type;
  static type value()
  {
    return TypeInfo::New<T>();
  }
};

template<typename T>
struct ReduceToList
{
  typedef T type;
  typedef QList<T> result;
  static result& value(result& list, const type& item)
  {
    list.push_back(item);
    return list;
  }
};

template<typename T>
class HasTypeSuperclass
{
  typedef char Small;
  struct Big { char dummy[2]; };

  template<typename U> static Small Test(typename U::SuperclassTypes*);
  template<typename U> static Big Test(...);
public:
  enum { value = sizeof(Test<T>(nullptr)) == sizeof(Small) };
};

template<typename T, bool>
struct GetSuperclassTypeList { typedef TypeList<> value; };

template<typename T>
struct GetSuperclassTypeList<T, true> { typedef typename T::SuperclassTypes value; };

template<typename T>
QList<TypeInfo> GetSuperclasses()
{
  return MapReduce<typename GetSuperclassTypeList<T, HasTypeSuperclass<T>::value>::value, MapToTypeInfo, ReduceToList>::value();
}

template<typename T>
QList<TypeInfo> TypeInfo::Model<T>::GetSuperclasses() const
{
  return ::berry::Reflection::GetSuperclasses<T>();
}

} // end namespace Reflection

} // end namespace berry

#endif // BERRYREFLECTION_H

