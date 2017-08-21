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

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// std includes
#include <string>

// MITK includes
#include "mitkExampleDataStructure.h"
#include "mitkIOUtil.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkExampleDataStructureReaderWriterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkExampleDataStructureReaderWriterTestSuite);

  // Test saving/loading
  MITK_TEST(ReadWrite_ExampleData_SavedAndLoadedDataEqualToExample);
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

  void ReadWrite_ExampleData_SavedAndLoadedDataEqualToExample()
  {
    std::string path = mitk::IOUtil::GetTempPath() + "ExampleDataOutput.txt";
    mitk::IOUtil::Save(m_Data, path);
    mitk::ExampleDataStructure::Pointer loadedData =
      dynamic_cast<mitk::ExampleDataStructure *>(mitk::IOUtil::Load(path)[0].GetPointer());

    itksys::SystemTools::RemoveFile(path);

    CPPUNIT_ASSERT_MESSAGE("Comparing created and loaded example data.",
                           mitk::Equal(m_Data, loadedData, mitk::eps, true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkExampleDataStructureReaderWriter)
