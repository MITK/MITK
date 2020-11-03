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

// MITK includes
#include "mitkExampleDataStructure.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkExampleDataStructureTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkExampleDataStructureTestSuite);

  // Test the append method
  MITK_TEST(Append_ExampleString_AddsExampleStringToData);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::ExampleDataStructure::Pointer m_Data;
  std::string m_DefaultDataString;

public:
  /**
  * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used
  * members for a new test case. (If the members are not used in a test, the method does not need to be called).
  */
  void setUp() override
  {
    m_DefaultDataString = "This is the example data content\nAnd a second line\n";
    m_Data = mitk::ExampleDataStructure::New();
    m_Data->SetData(m_DefaultDataString);
  }

  void tearDown() override
  {
    m_DefaultDataString = "";
    m_Data = nullptr;
  }

  void Append_ExampleString_AddsExampleStringToData()
  {
    std::string appendedString = "And a third line\n";
    std::string wholeString = m_DefaultDataString + appendedString;
    m_Data->AppendAString(appendedString);

    CPPUNIT_ASSERT_MESSAGE("Checking whether string was correctly appended.", m_Data->GetData() == wholeString);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkExampleDataStructure)
