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
#include <algorithm>
#include <utility>

#ifdef _MSC_VER
#pragma warning(disable : 4503) // "decorated name length exceeded, name was truncated"
#endif

class AliasEquals
{
public:
  AliasEquals(const std::string &alias) : m_Alias(alias) {}
  bool operator()(const std::pair<std::string, std::vector<std::string>> &element)
  {
    auto iter = std::find(element.second.begin(), element.second.end(), m_Alias);
    return iter != element.second.end();
  }

private:
  std::string m_Alias;
};

mitk::PropertyAliases::PropertyAliases()
{
}

mitk::PropertyAliases::~PropertyAliases()
{
}

bool mitk::PropertyAliases::AddAlias(const std::string &propertyName,
                                     const std::string &alias,
                                     const std::string &className)
{
  if (alias.empty())
    return false;

  AliasesMap &aliases = m_Aliases[className];
  auto iter = aliases.find(propertyName);

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

std::vector<std::string> mitk::PropertyAliases::GetAliases(const std::string &propertyName,
                                                           const std::string &className)
{
  if (!propertyName.empty())
  {
    AliasesMap &aliases = m_Aliases[className];
    AliasesMapConstIterator iter = aliases.find(propertyName);

    if (iter != aliases.end())
      return iter->second;
  }

  return std::vector<std::string>();
}

std::string mitk::PropertyAliases::GetPropertyName(const std::string &alias, const std::string &className)
{
  if (!alias.empty())
  {
    AliasesMap &aliases = m_Aliases[className];
    AliasesMapConstIterator iter = std::find_if(aliases.begin(), aliases.end(), AliasEquals(alias));

    if (iter != aliases.end())
      return iter->first;
  }

  return "";
}

bool mitk::PropertyAliases::HasAliases(const std::string &propertyName, const std::string &className)
{
  const AliasesMap &aliases = m_Aliases[className];

  return !propertyName.empty() ? aliases.find(propertyName) != aliases.end() : false;
}

void mitk::PropertyAliases::RemoveAlias(const std::string &propertyName,
                                        const std::string &alias,
                                        const std::string &className)
{
  if (!propertyName.empty() && !alias.empty())
  {
    AliasesMap &aliases = m_Aliases[className];
    auto iter = aliases.find(propertyName);

    if (iter != aliases.end())
    {
      auto aliasIter = std::find(iter->second.begin(), iter->second.end(), alias);

      if (aliasIter != iter->second.end())
      {
        iter->second.erase(aliasIter);

        if (iter->second.empty())
          aliases.erase(propertyName);
      }
    }
  }
}

void mitk::PropertyAliases::RemoveAliases(const std::string &propertyName, const std::string &className)
{
  if (!propertyName.empty())
  {
    AliasesMap &aliases = m_Aliases[className];
    aliases.erase(propertyName);
  }
}

void mitk::PropertyAliases::RemoveAllAliases(const std::string &className)
{
  AliasesMap &aliases = m_Aliases[className];
  aliases.clear();
}
