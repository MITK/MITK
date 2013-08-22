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

#include <mitkCoreServices.h>
#include <mitkIPropertyAliases.h>
#include <mitkTestingMacros.h>
#include <algorithm>

int mitkPropertyAliasesTest(int, char* [])
{
  MITK_TEST_BEGIN("mitkPropertyAliasesTest");

  mitk::IPropertyAliases* propertyAliases = mitk::CoreServices::GetPropertyAliases();
  MITK_TEST_CONDITION_REQUIRED(propertyAliases != NULL, "Get property aliases service");

  propertyAliases->AddAlias("propertyName1", "alias1a");
  propertyAliases->AddAlias("propertyName1", "alias1b");
  propertyAliases->AddAlias("propertyName2", "alias2a");
  propertyAliases->AddAlias("propertyName2", "alias2b", "className");

  typedef std::vector<std::string> Aliases;
  typedef Aliases::iterator AliasesIterator;

  Aliases aliases = propertyAliases->GetAliases("propertyName1");
  AliasesIterator it1 = std::find(aliases.begin(), aliases.end(), "alias1a");
  AliasesIterator it2 = std::find(aliases.begin(), aliases.end(), "alias1b");

  MITK_TEST_CONDITION(
    aliases.size() == 2 && it1 != aliases.end() && it2 != aliases.end(),
    "Get aliases of \"propertyName1\"");

  aliases = propertyAliases->GetAliases("propertyName2");
  it1 = std::find(aliases.begin(), aliases.end(), "alias2a");

  MITK_TEST_CONDITION(
    aliases.size() == 1 && it1 != aliases.end(),
    "Get aliases of \"propertyName2\"");

  aliases = propertyAliases->GetAliases("propertyName2", "className");
  it1 = std::find(aliases.begin(), aliases.end(), "alias2b");

  MITK_TEST_CONDITION(
    aliases.size() == 1 && it1 != aliases.end(),
    "Get aliases of \"propertyName2\" restricted to \"className\"");

  std::string propertyName = propertyAliases->GetPropertyName("alias1b");

  MITK_TEST_CONDITION(
    propertyName == "propertyName1",
    "Get property name of \"alias1b\"");

  propertyName = propertyAliases->GetPropertyName("alias2b");

  MITK_TEST_CONDITION(
    propertyName.empty(),
    "Get property name of non-existing unrestricted \"alias2b\"");

  propertyName = propertyAliases->GetPropertyName("alias2b", "className");

  MITK_TEST_CONDITION(
    propertyName == "propertyName2",
    "Get property name of restricted \"alias2b\"");

  MITK_TEST_END();
}
