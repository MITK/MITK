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
#include <mitkIPropertyDescriptions.h>
// VTK includes
#include <vtkDebugLeaks.h>

class mitkPropertyDescriptionsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyDescriptionsTestSuite);
  MITK_TEST(GetPropertyDescriptionService_Success);
  MITK_TEST(GetPropertyDescription_Success);
  MITK_TEST(GetOverwrittenDescription_Success);
  MITK_TEST(GetPropertyDescriptionRestricted_Success);
  CPPUNIT_TEST_SUITE_END();

  private:
  mitk::IPropertyDescriptions *m_PropertyDescriptions;
  std::string m_Description1;

  public:
  void setUp()
  {
    m_PropertyDescriptions = mitk::CoreServices::GetPropertyDescriptions();
    m_PropertyDescriptions->AddDescription("propertyName1", "description1a");
    m_PropertyDescriptions->AddDescription("propertyName1", "description1b");
  }

  void tearDown()
  {
    m_PropertyDescriptions = nullptr;
  }

  void GetPropertyDescriptionService_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Get property descriptions service", m_PropertyDescriptions != nullptr);
  }

  void GetPropertyDescription_Success()
  {
    m_Description1 = m_PropertyDescriptions->GetDescription("propertyName1");
    CPPUNIT_ASSERT_MESSAGE("Get description of \"propertyName1\"", m_Description1 == "description1a");
  }

  void GetOverwrittenDescription_Success()
  {
    m_PropertyDescriptions->AddDescription("propertyName1", "description1b", "", true);
    m_Description1 = m_PropertyDescriptions->GetDescription("propertyName1");
    CPPUNIT_ASSERT_MESSAGE("Get overwritten description of \"propertyName1\"", m_Description1 == "description1b");
  }

  void GetPropertyDescriptionRestricted_Success()
  {
    m_PropertyDescriptions->AddDescription("propertyName1", "description1c", "className");
    std::string description2 = m_PropertyDescriptions->GetDescription("propertyName1", "className");
    m_Description1 = m_PropertyDescriptions->GetDescription("propertyName1");
    
    CPPUNIT_ASSERT_MESSAGE("Get description of \"propertyName1\" restricted to \"className\"",
                            m_Description1 == "description1b" && description2 == "description1c");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyDescriptions)
