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

#include "mitkPropertyList.h"

#include "mitkNumericTypes.h"
#include "mitkProperties.h"
#include "mitkStringProperty.h"

mitk::BaseProperty::ConstPointer mitk::PropertyList::GetConstProperty(const std::string &propertyKey, const std::string &/*contextName*/, bool /*fallBackOnDefaultContext*/) const
{
  PropertyMap::const_iterator it;

  it = m_Properties.find(propertyKey);
  if (it != m_Properties.cend())
    return it->second.GetPointer();
  else
    return nullptr;
};

std::vector<std::string> mitk::PropertyList::GetPropertyKeys(const std::string &contextName, bool includeDefaultContext) const
{
  std::vector<std::string> propertyKeys;

  if (contextName.empty() || includeDefaultContext)
  {
    for (auto property : this->m_Properties)
      propertyKeys.push_back(property.first);
  }

  return propertyKeys;
};

std::vector<std::string> mitk::PropertyList::GetPropertyContextNames() const
{
  return std::vector<std::string>();
};

mitk::BaseProperty *mitk::PropertyList::GetProperty(const std::string &propertyKey) const
{
  PropertyMap::const_iterator it;

  it = m_Properties.find(propertyKey);
  if (it != m_Properties.cend())
    return it->second;
  else
    return nullptr;
}

mitk::BaseProperty * mitk::PropertyList::GetNonConstProperty(const std::string &propertyKey, const std::string &/*contextName*/, bool /*fallBackOnDefaultContext*/)
{
  return this->GetProperty(propertyKey);
}

void mitk::PropertyList::SetProperty(const std::string &propertyKey, BaseProperty *property, const std::string &/*contextName*/, bool /*fallBackOnDefaultContext*/)
{
  if (propertyKey.empty())
    mitkThrow() << "Property key is empty.";

  if (!property)
    return;
  // make sure that BaseProperty*, which may have just been created and never been
  // assigned to a SmartPointer, is registered/unregistered properly. If we do not
  // do that, it will a) not deleted in case it is identical to the old one or
  // b) possibly deleted when temporarily added to a smartpointer somewhere below.
  BaseProperty::Pointer tmpSmartPointerToProperty = property;

  auto it(m_Properties.find(propertyKey));

  // Is a property with key @a propertyKey contained in the list?
  if (it != m_Properties.cend())
  {
    // yes
    // is the property contained in the list identical to the new one?
    if (it->second->operator==(*property))
    {
      // yes? do nothing and return.
      return;
    }

    if (it->second->AssignProperty(*property))
    {
      // The assignment was successfull
      this->Modified();
    }
    else
    {
      MITK_ERROR << "In " __FILE__ ", l." << __LINE__ << ": Trying to set existing property " << it->first
        << " of type " << it->second->GetNameOfClass() << " to a property with different type "
        << property->GetNameOfClass() << "."
        << " Use ReplaceProperty() instead." << std::endl;
    }
    return;
  }

  // no? add it.
  m_Properties.insert(PropertyMap::value_type(propertyKey, property));
  this->Modified();
}

void mitk::PropertyList::ReplaceProperty(const std::string &propertyKey, BaseProperty *property)
{
  if (!property)
    return;

  auto it(m_Properties.find(propertyKey));

  // Is a property with key @a propertyKey contained in the list?
  if (it != m_Properties.cend())
  {
    it->second = nullptr;
    m_Properties.erase(it);
  }

  // no? add/replace it.
  m_Properties.insert(PropertyMap::value_type(propertyKey, property));
  Modified();
}

void mitk::PropertyList::RemoveProperty(const std::string &propertyKey, const std::string &/*contextName*/, bool /*fallBackOnDefaultContext*/)
{
  auto it(m_Properties.find(propertyKey));

  // Is a property with key @a propertyKey contained in the list?
  if (it != m_Properties.cend())
  {
    it->second = nullptr;
    m_Properties.erase(it);
    Modified();
  }
}

mitk::PropertyList::PropertyList()
{
}

mitk::PropertyList::PropertyList(const mitk::PropertyList &other) : itk::Object()
{
  for (auto i = other.m_Properties.cbegin(); i != other.m_Properties.cend(); ++i)
  {
    m_Properties.insert(std::make_pair(i->first, i->second->Clone()));
  }
}

mitk::PropertyList::~PropertyList()
{
  Clear();
}

/**
 * Consider the list as changed when any of the properties has changed recently.
 */
unsigned long mitk::PropertyList::GetMTime() const
{
  for (auto it = m_Properties.cbegin(); it != m_Properties.cend(); ++it)
  {
    if (it->second.IsNull())
    {
      itkWarningMacro(<< "Property '" << it->first << "' contains nothing (nullptr).");
      continue;
    }
    if (Superclass::GetMTime() < it->second->GetMTime())
    {
      Modified();
      break;
    }
  }

  return Superclass::GetMTime();
}

bool mitk::PropertyList::DeleteProperty(const std::string &propertyKey)
{
  auto it = m_Properties.find(propertyKey);

  if (it != m_Properties.end())
  {
    it->second = nullptr;
    m_Properties.erase(it);
    Modified();
    return true;
  }
  return false;
}

void mitk::PropertyList::Clear()
{
  auto it = m_Properties.begin(), end = m_Properties.end();
  while (it != end)
  {
    it->second = nullptr;
    ++it;
  }
  m_Properties.clear();
}

itk::LightObject::Pointer mitk::PropertyList::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  result->UnRegister();
  return result;
}

void mitk::PropertyList::ConcatenatePropertyList(PropertyList *pList, bool replace)
{
  if (pList)
  {
    const PropertyMap *propertyMap = pList->GetMap();

    for (auto iter = propertyMap->cbegin(); // m_PropertyList is created in the constructor, so we don't check it here
         iter != propertyMap->cend();
         ++iter)
    {
      const std::string key = iter->first;
      BaseProperty *value = iter->second;
      if (replace)
      {
        ReplaceProperty(key.c_str(), value);
      }
      else
      {
        SetProperty(key.c_str(), value);
      }
    }
  }
}

bool mitk::PropertyList::GetBoolProperty(const char *propertyKey, bool &boolValue) const
{
  BoolProperty *gp = dynamic_cast<BoolProperty *>(GetProperty(propertyKey));
  if (gp != nullptr)
  {
    boolValue = gp->GetValue();
    return true;
  }
  return false;
  // Templated Method does not work on Macs
  // return GetPropertyValue<bool>(propertyKey, boolValue);
}

bool mitk::PropertyList::GetIntProperty(const char *propertyKey, int &intValue) const
{
  IntProperty *gp = dynamic_cast<IntProperty *>(GetProperty(propertyKey));
  if (gp != nullptr)
  {
    intValue = gp->GetValue();
    return true;
  }
  return false;
  // Templated Method does not work on Macs
  // return GetPropertyValue<int>(propertyKey, intValue);
}

bool mitk::PropertyList::GetFloatProperty(const char *propertyKey, float &floatValue) const
{
  FloatProperty *gp = dynamic_cast<FloatProperty *>(GetProperty(propertyKey));
  if (gp != nullptr)
  {
    floatValue = gp->GetValue();
    return true;
  }
  return false;
  // Templated Method does not work on Macs
  // return GetPropertyValue<float>(propertyKey, floatValue);
}

bool mitk::PropertyList::GetStringProperty(const char *propertyKey, std::string &stringValue) const
{
  StringProperty *sp = dynamic_cast<StringProperty *>(GetProperty(propertyKey));
  if (sp != nullptr)
  {
    stringValue = sp->GetValue();
    return true;
  }
  return false;
}

void mitk::PropertyList::SetIntProperty(const char *propertyKey, int intValue)
{
  SetProperty(propertyKey, mitk::IntProperty::New(intValue));
}

void mitk::PropertyList::SetBoolProperty(const char *propertyKey, bool boolValue)
{
  SetProperty(propertyKey, mitk::BoolProperty::New(boolValue));
}

void mitk::PropertyList::SetFloatProperty(const char *propertyKey, float floatValue)
{
  SetProperty(propertyKey, mitk::FloatProperty::New(floatValue));
}

void mitk::PropertyList::SetStringProperty(const char *propertyKey, const char *stringValue)
{
  SetProperty(propertyKey, mitk::StringProperty::New(stringValue));
}

void mitk::PropertyList::Set(const char *propertyKey, bool boolValue)
{
  this->SetBoolProperty(propertyKey, boolValue);
}

void mitk::PropertyList::Set(const char *propertyKey, int intValue)
{
  this->SetIntProperty(propertyKey, intValue);
}

void mitk::PropertyList::Set(const char *propertyKey, float floatValue)
{
  this->SetFloatProperty(propertyKey, floatValue);
}

void mitk::PropertyList::Set(const char *propertyKey, double doubleValue)
{
  this->SetDoubleProperty(propertyKey, doubleValue);
}

void mitk::PropertyList::Set(const char *propertyKey, const char *stringValue)
{
  this->SetStringProperty(propertyKey, stringValue);
}

void mitk::PropertyList::Set(const char *propertyKey, const std::string &stringValue)
{
  this->SetStringProperty(propertyKey, stringValue.c_str());
}

bool mitk::PropertyList::Get(const char *propertyKey, bool &boolValue) const
{
  return this->GetBoolProperty(propertyKey, boolValue);
}

bool mitk::PropertyList::Get(const char *propertyKey, int &intValue) const
{
  return this->GetIntProperty(propertyKey, intValue);
}

bool mitk::PropertyList::Get(const char *propertyKey, float &floatValue) const
{
  return this->GetFloatProperty(propertyKey, floatValue);
}

bool mitk::PropertyList::Get(const char *propertyKey, double &doubleValue) const
{
  return this->GetDoubleProperty(propertyKey, doubleValue);
}

bool mitk::PropertyList::Get(const char *propertyKey, std::string &stringValue) const
{
  return this->GetStringProperty(propertyKey, stringValue);
}

bool mitk::PropertyList::GetDoubleProperty(const char *propertyKey, double &doubleValue) const
{
  DoubleProperty *gp = dynamic_cast<DoubleProperty *>(GetProperty(propertyKey));
  if (gp != nullptr)
  {
    doubleValue = gp->GetValue();
    return true;
  }
  return false;
}

void mitk::PropertyList::SetDoubleProperty(const char *propertyKey, double doubleValue)
{
  SetProperty(propertyKey, mitk::DoubleProperty::New(doubleValue));
}
