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

#include <mitkIPropertyAliases.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkPropertyExtension.h>
#include <mitkIPropertyExtensions.h>
#include <mitkIPropertyFilters.h>
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

  bool operator()(const std::pair<std::string, std::vector<std::string> >& element)
  {
    std::vector<std::string>::const_iterator iter = std::find(element.second.begin(), element.second.end(), m_Alias);
    return iter != element.second.end();
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
  class PropertiesActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext* context)
    {
      m_PropertyAliases = PropertyAliasesImpl::New();
      context->RegisterService<IPropertyAliases>(m_PropertyAliases);

      m_PropertyDescriptions = PropertyDescriptionsImpl::New();
      context->RegisterService<IPropertyDescriptions>(m_PropertyDescriptions);

      m_PropertyExtensions = PropertyExtensionsImpl::New();
      context->RegisterService<IPropertyExtensions>(m_PropertyExtensions);

      m_PropertyFilters = PropertyFiltersImpl::New();
      context->RegisterService<IPropertyFilters>(m_PropertyFilters);
    }

    void Unload(us::ModuleContext*)
    {
    }

  private:
    class PropertyAliasesImpl : public itk::LightObject, public IPropertyAliases
    {
    public:
      mitkClassMacro(PropertyAliasesImpl, itk::LightObject);
      itkNewMacro(Self);

      bool AddAlias(const std::string& propertyName, const std::string& alias, const std::string& className);
      std::vector<std::string> GetAliases(const std::string& propertyName, const std::string& className);
      std::string GetPropertyName(const std::string& alias, const std::string& className);
      bool HasAliases(const std::string& propertyName, const std::string& className);
      void RemoveAlias(const std::string& propertyName, const std::string& alias, const std::string& className);
      void RemoveAliases(const std::string& propertyName, const std::string& className);
      void RemoveAllAliases(const std::string& className);

    private:
      std::map<std::string, std::map<std::string, std::vector<std::string> > > m_Aliases;
    };

    class PropertyDescriptionsImpl : public itk::LightObject, public IPropertyDescriptions
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

    class PropertyExtensionsImpl : public itk::LightObject, public IPropertyExtensions
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

    class PropertyFiltersImpl : public itk::LightObject, public IPropertyFilters
    {
    public:
      mitkClassMacro(PropertyFiltersImpl, itk::LightObject);
      itkNewMacro(Self);

      bool AddFilter(const PropertyFilter& filter, const std::string& className, bool overwrite);
      std::map<std::string, BaseProperty::Pointer> ApplyFilter(const std::map<std::string, BaseProperty::Pointer>& propertyMap, const std::string& className) const;
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

  bool PropertiesActivator::PropertyAliasesImpl::AddAlias(const std::string& propertyName, const std::string& alias, const std::string& className)
  {
    if (alias.empty())
      return false;

    std::map<std::string, std::vector<std::string> >& aliases = m_Aliases[className];
    std::map<std::string, std::vector<std::string> >::iterator iter = aliases.find(propertyName);

    if (iter != aliases.end())
    {
      if (std::find(iter->second.begin(), iter->second.end(), alias) == iter->second.end())
        iter->second.push_back(alias);
    }
    else
    {
      aliases.insert(std::make_pair(propertyName, std::vector<std::string>(1, alias)));
    }

    return true;
  }

  std::vector<std::string> PropertiesActivator::PropertyAliasesImpl::GetAliases(const std::string& propertyName, const std::string& className)
  {
    if (!propertyName.empty())
    {
      std::map<std::string, std::vector<std::string> >& aliases = m_Aliases[className];
      std::map<std::string, std::vector<std::string> >::const_iterator iter = aliases.find(propertyName);

      if (iter != aliases.end())
        return iter->second;
    }

    return std::vector<std::string>();
  }

  std::string PropertiesActivator::PropertyAliasesImpl::GetPropertyName(const std::string& alias, const std::string& className)
  {
    if (!alias.empty())
    {
      std::map<std::string, std::vector<std::string> >& aliases = m_Aliases[className];
      std::map<std::string, std::vector<std::string> >::const_iterator iter = std::find_if(aliases.begin(), aliases.end(), AliasEquals(alias));

      if (iter != aliases.end())
        return iter->first;
    }

    return "";
  }

  bool PropertiesActivator::PropertyAliasesImpl::HasAliases(const std::string& propertyName, const std::string& className)
  {
    const std::map<std::string, std::vector<std::string> >& aliases = m_Aliases[className];

    return !propertyName.empty()
      ? aliases.find(propertyName) != aliases.end()
      : false;
  }

  void PropertiesActivator::PropertyAliasesImpl::RemoveAlias(const std::string& propertyName, const std::string& alias, const std::string& className)
  {
    if (!propertyName.empty() && !alias.empty())
    {
      std::map<std::string, std::vector<std::string> >& aliases = m_Aliases[className];
      std::map<std::string, std::vector<std::string> >::iterator iter = aliases.find(propertyName);

      if (iter != aliases.end())
      {
        std::vector<std::string>::const_iterator aliasIter = std::find(iter->second.begin(), iter->second.end(), alias);

        if (aliasIter != iter->second.end())
        {
          iter->second.erase(aliasIter);

          if (iter->second.empty())
            aliases.erase(propertyName);
        }
      }
    }
  }

  void PropertiesActivator::PropertyAliasesImpl::RemoveAliases(const std::string& propertyName, const std::string& className)
  {
    if (!propertyName.empty())
    {
      std::map<std::string, std::vector<std::string> >& aliases = m_Aliases[className];
      aliases.erase(propertyName);
    }
  }

  void PropertiesActivator::PropertyAliasesImpl::RemoveAllAliases(const std::string& className)
  {
    std::map<std::string, std::vector<std::string> >& aliases = m_Aliases[className];
    aliases.clear();
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

  bool PropertiesActivator::PropertyFiltersImpl::AddFilter(const PropertyFilter& filter, const std::string& className, bool overwrite)
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

  std::map<std::string, BaseProperty::Pointer> PropertiesActivator::PropertyFiltersImpl::ApplyFilter(const std::map<std::string, BaseProperty::Pointer>& propertyMap, const std::string& className) const
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
