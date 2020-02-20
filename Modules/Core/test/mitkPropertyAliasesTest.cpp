/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// std includes
#include <string>
#include <algorithm>
// MITK includes
#include <mitkCoreServices.h>
#include <mitkIPropertyAliases.h>
// VTK includes
#include <vtkDebugLeaks.h>

class mitkPropertyAliasesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyAliasesTestSuite);
  MITK_TEST(GetPropertyAliasesService_Success);
  MITK_TEST(GetAliases_Success);
  MITK_TEST(GetAliasesRestricted_Success);
  MITK_TEST(GetPropertyName_Success);
  MITK_TEST(GetPropertyNameNonexisting_Empty);
  MITK_TEST(GetPropertyNameRestricted_Success);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::IPropertyAliases *m_PropertyAliases;
  typedef std::vector<std::string> Aliases;
  Aliases m_Aliases;
  std::string m_PropertyName;

public:
  void setUp()
  {
    m_PropertyAliases = mitk::CoreServices::GetPropertyAliases();
    m_PropertyAliases->AddAlias("propertyName1", "alias1a");
    m_PropertyAliases->AddAlias("propertyName1", "alias1b");
    m_PropertyAliases->AddAlias("propertyName2", "alias2a");
    m_PropertyAliases->AddAlias("propertyName2", "alias2b", "className");
  }
  void tearDown()
  {
    m_PropertyAliases = nullptr;
  }


  void GetPropertyAliasesService_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Get property aliases service", m_PropertyAliases != nullptr);
  }

  void GetAliases_Success()
  {
    m_Aliases = m_PropertyAliases->GetAliases("propertyName1");
    auto it1 = std::find(m_Aliases.begin(), m_Aliases.end(), "alias1a");
    auto it2 = std::find(m_Aliases.begin(), m_Aliases.end(), "alias1b");
    
    CPPUNIT_ASSERT_MESSAGE("Get aliases of \"propertyName1\"", m_Aliases.size() == 2 && it1 != m_Aliases.end() && it2 != m_Aliases.end());
    
    m_Aliases = m_PropertyAliases->GetAliases("propertyName2");
    it1 = std::find(m_Aliases.begin(), m_Aliases.end(), "alias2a");
    
    CPPUNIT_ASSERT_MESSAGE("Get aliases of \"propertyName2\"", m_Aliases.size() == 1 && it1 != m_Aliases.end());
  }

  void GetAliasesRestricted_Success()
  {
      m_Aliases = m_PropertyAliases->GetAliases("propertyName2", "className");
      auto it1 = std::find(m_Aliases.begin(), m_Aliases.end(), "alias2b");
    
      CPPUNIT_ASSERT_MESSAGE("Get aliases of \"propertyName2\" restricted to \"className\"", m_Aliases.size() == 1 && it1 != m_Aliases.end());
  }

  void GetPropertyName_Success()
  {
    m_PropertyName = m_PropertyAliases->GetPropertyName("alias1b");
    
    CPPUNIT_ASSERT_MESSAGE("Get property name of \"alias1b\"", m_PropertyName == "propertyName1");
  }

  void GetPropertyNameNonexisting_Empty()
  {
    m_PropertyName = m_PropertyAliases->GetPropertyName("alias2b");
    
    CPPUNIT_ASSERT_MESSAGE("Get property name of non-existing unrestricted \"alias2b\"", m_PropertyName.empty());
  }

  void GetPropertyNameRestricted_Success()
  {
    m_PropertyName = m_PropertyAliases->GetPropertyName("alias2b", "className");
    
    CPPUNIT_ASSERT_MESSAGE("Get property name of restricted \"alias2b\"", m_PropertyName == "propertyName2");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyAliases)
