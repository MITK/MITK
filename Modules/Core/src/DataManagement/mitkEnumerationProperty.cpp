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

#include <mitkEnumerationProperty.h>
#include <algorithm>

mitk::EnumerationProperty::EnumerationProperty()
  : m_CurrentValue(0)
{
}

mitk::EnumerationProperty::EnumerationProperty(const EnumerationProperty &other)
  : BaseProperty(other),
    m_CurrentValue(other.m_CurrentValue),
    m_IdMap(other.m_IdMap),
    m_NameMap(other.m_NameMap)
{
}

bool mitk::EnumerationProperty::AddEnum(const std::string &name, const IdType &id)
{
  if (false == this->IsValidEnumerationValue(id) &&
      false == this->IsValidEnumerationValue(name))
  {
    this->GetEnumIds().insert(std::make_pair(id, name));
    this->GetEnumStrings().insert(std::make_pair(name, id));

    return true;
  }

  return false;
}

bool mitk::EnumerationProperty::SetValue(const std::string &name)
{
  if (this->IsValidEnumerationValue(name))
  {
    m_CurrentValue = this->GetEnumId(name);
    this->Modified();

    return true;
  }

  return false;
}

bool mitk::EnumerationProperty::SetValue(const IdType &id)
{
  if (this->IsValidEnumerationValue(id))
  {
    m_CurrentValue = id;
    this->Modified();

    return true;
  }

  return false;
}

mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetValueAsId() const
{
  return m_CurrentValue;
}

std::string mitk::EnumerationProperty::GetValueAsString() const
{
  return this->GetEnumString(m_CurrentValue);
}

void mitk::EnumerationProperty::Clear()
{
  this->GetEnumIds().clear();
  this->GetEnumStrings().clear();

  m_CurrentValue = 0;
}

mitk::EnumerationProperty::EnumIdsContainerType::size_type mitk::EnumerationProperty::Size() const
{
  return this->GetEnumIds().size();
}

mitk::EnumerationProperty::EnumConstIterator mitk::EnumerationProperty::Begin() const
{
  return this->GetEnumIds().cbegin();
}

mitk::EnumerationProperty::EnumConstIterator mitk::EnumerationProperty::End() const
{
  return this->GetEnumIds().cend();
}

std::string mitk::EnumerationProperty::GetEnumString(const IdType &id) const
{
  return this->IsValidEnumerationValue(id)
    ? this->GetEnumIds().find(id)->second
    : std::string("invalid enum id or enums empty");
}

mitk::EnumerationProperty::IdType mitk::EnumerationProperty::GetEnumId(const std::string &name) const
{
  return this->IsValidEnumerationValue(name)
    ? this->GetEnumStrings().find(name)->second
    : 0;
}

bool mitk::EnumerationProperty::IsEqual(const BaseProperty &property) const
{
  const auto &other = static_cast<const Self &>(property);

  return
    this->Size() == other.Size() &&
    this->GetValueAsId() == other.GetValueAsId() &&
    std::equal(this->Begin(), this->End(), other.Begin());
}

bool mitk::EnumerationProperty::Assign(const BaseProperty &property)
{
  const auto &other = static_cast<const Self &>(property);

  this->GetEnumIds() = other.GetEnumIds();
  this->GetEnumStrings() = other.GetEnumStrings();

  m_CurrentValue = other.m_CurrentValue;

  return true;
}

bool mitk::EnumerationProperty::IsValidEnumerationValue(const IdType &id) const
{
  return this->GetEnumIds().end() != this->GetEnumIds().find(id);
}

bool mitk::EnumerationProperty::IsValidEnumerationValue(const std::string &name) const
{
  return this->GetEnumStrings().end() != this->GetEnumStrings().find(name);
}

mitk::EnumerationProperty::EnumIdsContainerType & mitk::EnumerationProperty::GetEnumIds()
{
  return m_IdMap;
}

const mitk::EnumerationProperty::EnumIdsContainerType & mitk::EnumerationProperty::GetEnumIds() const
{
  return m_IdMap;
}

mitk::EnumerationProperty::EnumStringsContainerType & mitk::EnumerationProperty::GetEnumStrings()
{
  return m_NameMap;
}

const mitk::EnumerationProperty::EnumStringsContainerType & mitk::EnumerationProperty::GetEnumStrings() const
{
  return m_NameMap;
}
