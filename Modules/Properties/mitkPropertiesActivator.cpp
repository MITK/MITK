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

#include "mitkPropertyAliases.h"
#include "mitkPropertyDescriptions.h"
#include "mitkPropertyExtension.h"
#include "mitkPropertyExtensions.h"
#include "mitkPropertyFilters.h"
#include <itkObjectFactory.h>
#include <mitkCommon.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

class AliasEquals
{
public:
  AliasEquals(const std::string& alias)
    : m_Alias(alias)
  {
  }

  bool operator()(const std::pair<std::string, std::string>& element)
  {
    return element.second == m_Alias;
  }

private:
  std::string m_Alias;
};

class DeleteExtension
{
public:
  void operator()(const std::pair<std::string, mitk::PropertyExtension*>& element)
  {
    delete element.second;
  }
};

namespace mitk
{
  class PropertiesActivator : public ModuleActivator
  {
  public:
    void Load(ModuleContext* context)
    {
      m_PropertyAliases = PropertyAliasesImpl::New();
      context->RegisterService<PropertyAliases>(m_PropertyAliases);

      m_PropertyDescriptions = PropertyDescriptionsImpl::New();
      context->RegisterService<PropertyDescriptions>(m_PropertyDescriptions);

      m_PropertyExtensions = PropertyExtensionsImpl::New();
      context->RegisterService<PropertyExtensions>(m_PropertyExtensions);

      m_PropertyFilters = PropertyFiltersImpl::New();
      context->RegisterService<PropertyFilters>(m_PropertyFilters);
    }

    void Unload(ModuleContext*)
    {
    }

  private:
    class PropertyAliasesImpl : public itk::LightObject, public PropertyAliases
    {
    public:
      mitkClassMacro(PropertyAliasesImpl, itk::LightObject);
      itkNewMacro(Self);

      bool AddAlias(const std::string& propertyName, const std::string& alias, bool overwrite);
      std::string GetAlias(const std::string& propertyName) const;
      std::string GetPropertyName(const std::string& alias) const;
      bool HasAlias(const std::string& propertyName) const;
      void RemoveAllAliases();
      void RemoveAlias(const std::string& propertyName);

    private:
      std::map<std::string, std::string> m_Aliases;
    };

    class PropertyDescriptionsImpl : public itk::LightObject, public PropertyDescriptions
    {
    public:
      mitkClassMacro(PropertyDescriptionsImpl, itk::LightObject);
      itkNewMacro(Self);

      bool AddDescription(const std::string& propertyName, const std::string& description, bool overwrite);
      std::string GetDescription(const std::string& propertyName) const;
      bool HasDescription(const std::string& propertyName) const;
      void RemoveAllDescriptions();
      void RemoveDescription(const std::string& propertyName);

    private:
      std::map<std::string, std::string> m_Descriptions;
    };

    class PropertyExtensionsImpl : public itk::LightObject, public PropertyExtensions
    {
    public:
      mitkClassMacro(PropertyExtensionsImpl, itk::LightObject);
      itkNewMacro(Self);

      bool AddExtension(const std::string& propertyName, PropertyExtension* extension, bool overwrite);
      PropertyExtension* GetExtension(const std::string& propertyName) const;
      bool HasExtension(const std::string& propertyName) const;
      void RemoveAllExtensions();
      void RemoveExtension(const std::string& propertyName);

    private:
      std::map<std::string, PropertyExtension*> m_Extensions;
    };

    class PropertyFiltersImpl : public itk::LightObject, public PropertyFilters
    {
    public:
      mitkClassMacro(PropertyFiltersImpl, itk::LightObject);
      itkNewMacro(Self);

      bool AddFilter(const PropertyFilter& filter, bool overwrite);
      bool AddFilter(const std::string& className, const PropertyFilter& filter, bool overwrite);
      std::map<std::string, BaseProperty::Pointer> ApplyFilter(const std::map<std::string, BaseProperty::Pointer>& propertyMap) const;
      std::map<std::string, BaseProperty::Pointer> ApplyFilter(const std::string& className, const std::map<std::string, BaseProperty::Pointer>& propertyMap) const;
      PropertyFilter GetFilter(const std::string& className) const;
      bool HasFilter(const std::string& className) const;
      void RemoveAllFilters();
      void RemoveFilter(const std::string& className);

    private:
      std::map<std::string, PropertyFilter> m_Filters;
    };

    PropertyAliasesImpl::Pointer m_PropertyAliases;
    PropertyDescriptionsImpl::Pointer m_PropertyDescriptions;
    PropertyExtensionsImpl::Pointer m_PropertyExtensions;
    PropertyFiltersImpl::Pointer m_PropertyFilters;
  };

  bool PropertiesActivator::PropertyAliasesImpl::AddAlias(const std::string& propertyName, const std::string& alias, bool overwrite)
  {
    if (alias.empty())
      return false;

    std::pair<std::map<std::string, std::string>::iterator, bool> ret = m_Aliases.insert(std::make_pair(propertyName, alias));

    if (!ret.second && overwrite)
    {
      ret.first->second = alias;
      ret.second = true;
    }

    return ret.second;
  }

  std::string PropertiesActivator::PropertyAliasesImpl::GetAlias(const std::string& propertyName) const
  {
    if (!propertyName.empty())
    {
      std::map<std::string, std::string>::const_iterator iter = m_Aliases.find(propertyName);

      if (iter != m_Aliases.end())
        return iter->second;
    }

    return "";
  }

  std::string PropertiesActivator::PropertyAliasesImpl::GetPropertyName(const std::string& alias) const
  {
    if (!alias.empty())
    {
      std::map<std::string, std::string>::const_iterator iter = std::find_if(m_Aliases.begin(), m_Aliases.end(), AliasEquals(alias));

      if (iter != m_Aliases.end())
        return iter->first;
    }

    return "";
  }

  bool PropertiesActivator::PropertyAliasesImpl::HasAlias(const std::string& propertyName) const
  {
    return !propertyName.empty()
      ? m_Aliases.find(propertyName) != m_Aliases.end()
      : false;
  }

  void PropertiesActivator::PropertyAliasesImpl::RemoveAlias(const std::string& propertyName)
  {
    if (!propertyName.empty())
      m_Aliases.erase(propertyName);
  }

  void PropertiesActivator::PropertyAliasesImpl::RemoveAllAliases()
  {
    m_Aliases.clear();
  }

  bool PropertiesActivator::PropertyDescriptionsImpl::AddDescription(const std::string& propertyName, const std::string& description, bool overwrite)
  {
    if (!propertyName.empty())
    {
      std::pair<std::map<std::string, std::string>::iterator, bool> ret = m_Descriptions.insert(std::make_pair(propertyName, description));

      if (!ret.second && overwrite)
      {
        ret.first->second = description;
        ret.second = true;
      }

      return ret.second;
    }

    return false;
  }

  std::string PropertiesActivator::PropertyDescriptionsImpl::GetDescription(const std::string& propertyName) const
  {
    if (!propertyName.empty())
    {
      std::map<std::string, std::string>::const_iterator iter = m_Descriptions.find(propertyName);

      if (iter != m_Descriptions.end())
        return iter->second;
    }

    return "";
  }

  bool PropertiesActivator::PropertyDescriptionsImpl::HasDescription(const std::string& propertyName) const
  {
    return !propertyName.empty()
      ? m_Descriptions.find(propertyName) != m_Descriptions.end()
      : false;
  }

  void PropertiesActivator::PropertyDescriptionsImpl::RemoveAllDescriptions()
  {
    m_Descriptions.clear();
  }

  void PropertiesActivator::PropertyDescriptionsImpl::RemoveDescription(const std::string& propertyName)
  {
    if (!propertyName.empty())
      m_Descriptions.erase(propertyName);
  }

  bool PropertiesActivator::PropertyExtensionsImpl::AddExtension(const std::string& propertyName, PropertyExtension* extension, bool overwrite)
  {
    if (!propertyName.empty())
    {
      std::pair<std::map<std::string, PropertyExtension*>::iterator, bool> ret = m_Extensions.insert(std::make_pair(propertyName, extension));

      if (!ret.second && overwrite)
      {
        ret.first->second = extension;
        ret.second = true;
      }

      return ret.second;
    }

    return false;
  }

  PropertyExtension* PropertiesActivator::PropertyExtensionsImpl::GetExtension(const std::string& propertyName) const
  {
    if (!propertyName.empty())
    {
      std::map<std::string, PropertyExtension*>::const_iterator iter = m_Extensions.find(propertyName);

      if (iter != m_Extensions.end())
        return iter->second;
    }

    return NULL;
  }

  bool PropertiesActivator::PropertyExtensionsImpl::HasExtension(const std::string& propertyName) const
  {
    return !propertyName.empty()
      ? m_Extensions.find(propertyName) != m_Extensions.end()
      : false;
  }

  void PropertiesActivator::PropertyExtensionsImpl::RemoveAllExtensions()
  {
    std::for_each(m_Extensions.begin(), m_Extensions.end(), DeleteExtension());
    m_Extensions.clear();
  }

  void PropertiesActivator::PropertyExtensionsImpl::RemoveExtension(const std::string& propertyName)
  {
    if (!propertyName.empty())
    {
      delete m_Extensions[propertyName];
      m_Extensions.erase(propertyName);
    }
  }

  bool PropertiesActivator::PropertyFiltersImpl::AddFilter(const PropertyFilter& filter, bool overwrite)
  {
    return this->AddFilter("", filter, overwrite);
  }

  bool PropertiesActivator::PropertyFiltersImpl::AddFilter(const std::string& className, const PropertyFilter& filter, bool overwrite)
  {
    if (!filter.IsEmpty())
    {
      std::pair<std::map<std::string, PropertyFilter>::iterator, bool> ret = m_Filters.insert(std::make_pair(className, filter));

      if (!ret.second && overwrite)
      {
        ret.first->second = filter;
        ret.second = true;
      }

      return ret.second;
    }

    return false;
  }

  std::map<std::string, BaseProperty::Pointer> PropertiesActivator::PropertyFiltersImpl::ApplyFilter(const std::map<std::string, BaseProperty::Pointer>& propertyMap) const
  {
    return this->ApplyFilter("", propertyMap);
  }

  std::map<std::string, BaseProperty::Pointer> PropertiesActivator::PropertyFiltersImpl::ApplyFilter(const std::string& className, const std::map<std::string, BaseProperty::Pointer>& propertyMap) const
  {
    std::map<std::string, BaseProperty::Pointer> ret = propertyMap;
    PropertyFilter filter = this->GetFilter("");

    if (!filter.IsEmpty())
      ret = filter.Apply(ret);

    if (!className.empty())
    {
      filter = this->GetFilter(className);

      if (!filter.IsEmpty())
        ret = filter.Apply(ret);
    }

    return ret;
  }

  PropertyFilter PropertiesActivator::PropertyFiltersImpl::GetFilter(const std::string& className) const
  {
    std::map<std::string, PropertyFilter>::const_iterator iter = m_Filters.find(className);

    if (iter != m_Filters.end())
        return iter->second;

    return PropertyFilter();
  }

  bool PropertiesActivator::PropertyFiltersImpl::HasFilter(const std::string& className) const
  {
    return m_Filters.find(className) != m_Filters.end();
  }

  void PropertiesActivator::PropertyFiltersImpl::RemoveAllFilters()
  {
    m_Filters.clear();
  }

  void PropertiesActivator::PropertyFiltersImpl::RemoveFilter(const std::string& className)
  {
    m_Filters.erase(className);
  }
}

US_EXPORT_MODULE_ACTIVATOR(Properties, mitk::PropertiesActivator)
