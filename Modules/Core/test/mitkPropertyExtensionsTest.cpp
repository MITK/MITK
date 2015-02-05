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
#include <mitkIPropertyExtensions.h>
#include <mitkPropertyExtension.h>
#include <mitkTestingMacros.h>

class TestPropertyExtension : public mitk::PropertyExtension
{
public:
  mitkClassMacro(TestPropertyExtension, mitk::PropertyExtension);
  mitkNewMacro1Param(Self, const std::string&);

  std::string GetName() const
  {
    return m_Name;
  }

private:
  explicit TestPropertyExtension(const std::string& name)
    : m_Name(name)
  {
  }

  ~TestPropertyExtension()
  {
  }

  std::string m_Name;
};

int mitkPropertyExtensionsTest(int, char* [])
{
  MITK_TEST_BEGIN("mitkPropertyExtensionsTest");

  mitk::IPropertyExtensions* propertyExtensions = mitk::CoreServices::GetPropertyExtensions();
  MITK_TEST_CONDITION_REQUIRED(propertyExtensions != NULL, "Get property extensions service");

  propertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1a").GetPointer());
  propertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1b").GetPointer());
  TestPropertyExtension::Pointer extension1 = dynamic_cast<TestPropertyExtension*>(propertyExtensions->GetExtension("propertyName1").GetPointer());

  MITK_TEST_CONDITION(
    extension1.IsNotNull() && extension1->GetName() == "extension1a",
    "Get extension of \"propertyName1\"");

  propertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1b").GetPointer(), "", true);
  extension1 = dynamic_cast<TestPropertyExtension*>(propertyExtensions->GetExtension("propertyName1").GetPointer());

  MITK_TEST_CONDITION(
    extension1.IsNotNull() && extension1->GetName() == "extension1b",
    "Get overwritten extension of \"propertyName1\"");

  propertyExtensions->AddExtension("propertyName1", TestPropertyExtension::New("extension1c").GetPointer(), "className");
  TestPropertyExtension::Pointer extension2 = dynamic_cast<TestPropertyExtension*>(propertyExtensions->GetExtension("propertyName1", "className").GetPointer());
  extension1 = dynamic_cast<TestPropertyExtension*>(propertyExtensions->GetExtension("propertyName1").GetPointer());

  MITK_TEST_CONDITION(
    extension1.IsNotNull() && extension1->GetName() == "extension1b" && extension2.IsNotNull() && extension2->GetName() == "extension1c",
    "Get extension of \"propertyName1\" restricted to \"className\"");

  MITK_TEST_END();
}
