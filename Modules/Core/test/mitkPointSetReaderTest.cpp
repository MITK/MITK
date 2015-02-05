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

#include "mitkPointSet.h"
#include "mitkTestingMacros.h"
#include "mitkFileReaderRegistry.h"
#include "mitkMimeType.h"

/**
 *  Test for the class "mitkPointSetReader".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test data set for the tests (see CMakeLists.txt).
 */
int mitkPointSetReaderTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("PointSetReader")
  MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing invocation")

  mitk::FileReaderRegistry readerRegistry;

  // Get PointSet reader(s)
  std::vector<mitk::IFileReader*> readers = readerRegistry.GetReaders(mitk::FileReaderRegistry::GetMimeTypeForFile("mps"));
  MITK_TEST_CONDITION_REQUIRED(!readers.empty(), "Testing for registered readers")

  for (std::vector<mitk::IFileReader*>::const_iterator iter = readers.begin(), end = readers.end();
      iter != end; ++iter)
  {
    std::string testName = "test1";
    mitk::IFileReader* reader = *iter;
    reader->SetInput(testName);
    // testing file reading with invalid data
    MITK_TEST_CONDITION_REQUIRED(reader->GetConfidenceLevel() == mitk::IFileReader::Unsupported, "Testing confidence level with invalid input file name!");
    CPPUNIT_ASSERT_THROW(reader->Read(), mitk::Exception);

    // testing file reading with valid data
    std::string filePath = argv[1];
    reader->SetInput(filePath);
    MITK_TEST_CONDITION_REQUIRED( reader->GetConfidenceLevel() == mitk::IFileReader::Supported, "Testing confidence level with valid input file name!");
    std::vector<mitk::BaseData::Pointer> data = reader->Read();
    MITK_TEST_CONDITION_REQUIRED( !data.empty(), "Testing non-empty data with valid input file name!");

    // evaluate if the read point set is correct
    mitk::PointSet::Pointer resultPS = dynamic_cast<mitk::PointSet*>(data.front().GetPointer());
    MITK_TEST_CONDITION_REQUIRED( resultPS.IsNotNull(), "Testing correct BaseData type");
    MITK_TEST_CONDITION_REQUIRED( resultPS->GetTimeSteps() == 14, "Testing output time step generation!"); // CAVE: Only valid with the specified test data!
    MITK_TEST_CONDITION_REQUIRED( resultPS->GetPointSet(resultPS->GetTimeSteps()-1)->GetNumberOfPoints() == 0, "Testing output time step generation with empty time step!"); // CAVE: Only valid with the specified test data!
  }

  // always end with this!
  MITK_TEST_END()
}

