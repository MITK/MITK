/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkEnumerationProperty.h"
#include <algorithm>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace
{
  typedef boost::shared_lock<boost::shared_mutex> SharedLock;
  typedef boost::unique_lock<boost::shared_mutex> UniqueLock;

  template <typename TKey, typename TValue>
  class MapForClassName
  {
  public:
    typedef std::map<TKey, TValue> EnumContainerType;
    typedef std::pair<boost::shared_mutex, EnumContainerType> EnumContainer;

    EnumContainer& GetEnum(const mitk::EnumerationProperty& container)
    {
      const std::string className = container.GetNameOfClass(); // virtual!
      {
        const SharedLock lock(mapForClassNameGuard);
        auto iter = mapForClassName.find(className);
        if (mapForClassName.end() != iter) {
          return iter->second;
        }
      }
      const UniqueLock lock(mapForClassNameGuard);
      return mapForClassName[className];
    }

  private:
    typedef std::map<std::string, EnumContainer> MapForClassNameContainerType;

    MapForClassNameContainerType mapForClassName;
    boost::shared_mutex mapForClassNameGuard;
  };

  MapForClassName<mitk::EnumerationProperty::IdType, std::string> s_IdMapForClassName;
  MapForClassName<std::string, mitk::EnumerationProperty::IdType> s_StringMapForClassName;
}
mitk::EnumerationProperty::EnumerationProperty()
{
  m_CurrentValue = 0;
}

mitk::EnumerationProperty::EnumerationProperty(const EnumerationProperty& other)
  : BaseProperty(other)
  , m_CurrentValue(other.m_CurrentValue)
{
}

bool mitk::EnumerationProperty::AddEnum( const std::string& name, const IdType& id )
{
  if ( ( ! IsValidEnumerationValue( name ) ) && ( ! IsValidEnumerationValue( id ) ) )
  {
    {
      auto& ids = s_IdMapForClassName.GetEnum(*this);
      const UniqueLock lock(ids.first);
      ids.second.insert( std::make_pair( id, name ) );
    }
    {
      auto& strings = s_StringMapForClassName.GetEnum(*this);
      const UniqueLock lock(strings.first);
      strings.second.insert(std::make_pair(name, id));
    }
    return true;
  }
  else
  {
    return false;
  }
}


bool mitk::EnumerationProperty::SetValue( const std::string& name )
{
  if ( IsValidEnumerationValue( name ) )
  {
    m_CurrentValue = GetEnumId( name );
    Modified();
    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::EnumerationProperty::SetValue( const IdType& id )
{
  if ( IsValidEnumerationValue( id ) )
  {
    m_CurrentValue = id;
    Modified();
    return true;
  }
  else
  {
    return false;
  }
}

mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetValueAsId() const
{
  return m_CurrentValue;
}

std::string mitk::EnumerationProperty::GetValueAsString() const
{
  return GetEnumString( m_CurrentValue );
}


void mitk::EnumerationProperty::Clear()
{
  {
    auto& ids = s_IdMapForClassName.GetEnum(*this);
    const UniqueLock lock(ids.first);
    ids.second.clear();
  }
  {
    auto& strings = s_StringMapForClassName.GetEnum(*this);
    const UniqueLock lock(strings.first);
    strings.second.clear();
  }
  m_CurrentValue = 0;
}


size_t mitk::EnumerationProperty::Size() const
{
  auto& ids = s_IdMapForClassName.GetEnum(*this);
  const SharedLock lock(ids.first);
  return ids.second.size();
}

void mitk::EnumerationProperty::EnumerateIdsContainer(const mitk::EnumerationProperty::IdsContainerIterator& enumerator) const
{
  auto& ids = s_IdMapForClassName.GetEnum(*this);
  const SharedLock lock(ids.first);
  for (auto item : ids.second) {
    enumerator(item.first, item.second);
  }
}

void mitk::EnumerationProperty::EnumerateStringsContainer(const mitk::EnumerationProperty::StringsContainerIterator& enumerator) const
{
  auto& strings = s_StringMapForClassName.GetEnum(*this);
  const SharedLock lock(strings.first);
  for (auto item : strings.second) {
    enumerator(item.first, item.second);
  }
}


std::string mitk::EnumerationProperty::GetEnumString( const IdType& id ) const
{
  if ( IsValidEnumerationValue( id ) )
  {
    auto& ids = s_IdMapForClassName.GetEnum(*this);
    const SharedLock lock(ids.first);
    return ids.second.find( id )->second;
  }
  else
  {
    return "invalid enum id or enums empty";
  }
}


mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetEnumId( const std::string& name ) const
{
  if ( IsValidEnumerationValue( name ) )
  {
    auto& strings = s_StringMapForClassName.GetEnum(*this);
    const SharedLock lock(strings.first);
    return strings.second.find( name )->second;
  }
  else
  {
    return 0;
  }
}


bool mitk::EnumerationProperty::IsEqual( const BaseProperty& property ) const
{
  const Self& other = static_cast<const Self&>(property);
  if (this->Size() != other.Size() || this->GetValueAsId() != other.GetValueAsId()) {
    return false;
  }
  auto& ids = s_IdMapForClassName.GetEnum(*this);
  auto& otherIds = s_IdMapForClassName.GetEnum(other);
  const SharedLock lock(ids.first);
  const SharedLock otherLock(otherIds.first);
  return std::equal(ids.second.begin(), ids.second.end(), otherIds.second.begin());
}

bool mitk::EnumerationProperty::Assign( const BaseProperty& property )
{
  const Self& other = static_cast<const Self&>(property);
  {
    auto& ids = s_IdMapForClassName.GetEnum(*this);
    auto& otherIds = s_IdMapForClassName.GetEnum(other);
    if (&ids != &otherIds) {
      UniqueLock lock(ids.first, boost::defer_lock);
      SharedLock otherLock(otherIds.first, boost::defer_lock);
      boost::lock(lock, otherLock);
      ids.second = otherIds.second;
    }
  }
  {
    auto& strings = s_StringMapForClassName.GetEnum(*this);
    auto& otherStrings = s_StringMapForClassName.GetEnum(other);
    if (&strings != &otherStrings) {
      UniqueLock lock(strings.first, boost::defer_lock);
      SharedLock otherLock(otherStrings.first, boost::defer_lock);
      boost::lock(lock, otherLock);
      strings.second = otherStrings.second;
    }
  }
  this->m_CurrentValue = other.m_CurrentValue;
  return true;
}


bool mitk::EnumerationProperty::IsValidEnumerationValue( const IdType& val ) const
{
  auto& ids = s_IdMapForClassName.GetEnum(*this);
  const SharedLock lock(ids.first);
  return (ids.second.find( val ) != ids.second.end() );
}


bool mitk::EnumerationProperty::IsValidEnumerationValue( const std::string& val ) const
{
  auto& strings = s_StringMapForClassName.GetEnum(*this);
  const SharedLock lock(strings.first);
  return (strings.second.find( val ) != strings.second.end() );
}

itk::LightObject::Pointer mitk::EnumerationProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}
