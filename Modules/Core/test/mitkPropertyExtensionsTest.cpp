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
#include <mitkIPropertyExtensions.h>
#include <mitkPropertyExtension.h>
// VTK includes
#include <vtkDebugLeaks.h>

class TestPropertyExtension : public mitk::PropertyExtension
{
public:
  mitkClassMacro(TestPropertyExtension, mitk::PropertyExtension);
  mitkNewMacro1Param(Self, const std::string &);

  std::string GetName() const { return m_Name; }
private:
  explicit TestPropertyExtension(const std::string &name) : m_Name(name) {}
  ~TestPropertyExtension() override {}
  std::string m_Name;
};

class mitkPropertyExtensionsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyExtensionsTestSuite);
  MITK_TEST(GetPropertyExtensionService_Success);
  MITK_TEST(GetPropertyExtension_Success);
  MITK_TEST(GetOverwrittenExtension_Success);
  MITK_TEST(GetPropertyExtensionRestricted_Success);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::IPropertyExtensions *m_PropertyExtensions;
  TestPropertyExtension::Pointer m_Extension1;

public:
  void setUp()
  {
    m_PropertyExtensions = mitk::CoreServices::GetPropertyExtensions();
    m_PropertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1a").GetPointer());
    m_PropertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1b").GetPointer());
  }

  void tearDown()
  {
    m_PropertyExtensions = nullptr;
  }

  void GetPropertyExtensionService_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Get property extensions service", m_PropertyExtensions != nullptr);
  }

  void GetPropertyExtension_Success()
  {
    TestPropertyExtension::Pointer extension1 =
      dynamic_cast<TestPropertyExtension *>(m_PropertyExtensions->GetExtension("propertyName1").GetPointer());
    
    CPPUNIT_ASSERT_MESSAGE("Get extension of \"propertyName1\"", 
      extension1.IsNotNull() && extension1->GetName() == "extension1a");
  }

  void GetOverwrittenExtension_Success()
  {
    m_PropertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1b").GetPointer(), "", true);
    m_Extension1 = dynamic_cast<TestPropertyExtension *>(m_PropertyExtensions->GetExtension("propertyName1").GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Get overwritten extension of \"propertyName1\"",
             m_Extension1.IsNotNull() && m_Extension1->GetName() == "extension1b");
  }

  void GetPropertyExtensionRestricted_Success()
  {
    m_PropertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1c").GetPointer(), "className");
    TestPropertyExtension::Pointer extension2 =
      dynamic_cast<TestPropertyExtension *>(m_PropertyExtensions->GetExtension("propertyName1", "className").GetPointer());
    m_Extension1 = dynamic_cast<TestPropertyExtension *>(m_PropertyExtensions->GetExtension("propertyName1").GetPointer());
    
    CPPUNIT_ASSERT_MESSAGE("Get extension of \"propertyName1\" restricted to \"className\"",
      m_Extension1.IsNotNull() && m_Extension1->GetName() == "extension1b" && extension2.IsNotNull() &&
      extension2->GetName() == "extension1c");
  }
  };
  MITK_TEST_SUITE_REGISTRATION(mitkPropertyExtensions)

