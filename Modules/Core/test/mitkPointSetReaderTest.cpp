/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFileReaderRegistry.h>
#include <mitkMimeType.h>
#include <mitkPointSet.h>
#include <mitkTestingMacros.h>

#include <sstream>
#include <string>

namespace
{
  const std::string DefaultPoint =
    "<point><id>0</id><specification>0</specification><x>10</x><y>20</y><z>30</z></point>";

  /** A point set file with one time series that starts with the given elements and holds the given points. */
  std::string CreatePointSetFile(const std::string &timeSeriesStart,
                                 const std::string &afterTimeSeries = "",
                                 const std::string &points = DefaultPoint,
                                 const std::string &afterPointSet = "")
  {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
           "<point_set_file><file_version>0.1</file_version><point_set><time_series>" +
           timeSeriesStart + points + "</time_series>" + afterTimeSeries + "</point_set>" + afterPointSet +
           "</point_set_file>";
  }

  std::vector<mitk::BaseData::Pointer> Read(mitk::IFileReader *reader, const std::string &content)
  {
    std::istringstream stream(content);
    reader->SetInput("test.mps", &stream);
    return reader->Read();
  }
}

/**
 *  Test for the class "mitkPointSetReader".
 *
 *  argc and argv are the command line parameters which were passed to
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test data set for the tests (see CMakeLists.txt).
 */
int mitkPointSetReaderTest(int argc, char *argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("PointSetReader")
  MITK_TEST_CONDITION_REQUIRED(argc == 2, "Testing invocation")

  mitk::FileReaderRegistry readerRegistry;

  // Get PointSet reader(s)
  std::vector<mitk::IFileReader *> readers =
    readerRegistry.GetReaders(mitk::FileReaderRegistry::GetMimeTypeForFile("test.mps"));
  MITK_TEST_CONDITION_REQUIRED(!readers.empty(), "Testing for registered readers")

  for (std::vector<mitk::IFileReader *>::const_iterator iter = readers.begin(), end = readers.end(); iter != end;
       ++iter)
  {
    std::string testName = "test1";
    mitk::IFileReader *reader = *iter;
    reader->SetInput(testName);
    // testing file reading with invalid data
    MITK_TEST_CONDITION_REQUIRED(reader->GetConfidenceLevel() == mitk::IFileReader::Unsupported,
                                 "Testing confidence level with invalid input file name!");
    CPPUNIT_ASSERT_THROW(reader->Read(), mitk::Exception);

    // testing file reading with valid data
    std::string filePath = argv[1];
    reader->SetInput(filePath);
    MITK_TEST_CONDITION_REQUIRED(reader->GetConfidenceLevel() == mitk::IFileReader::Supported,
                                 "Testing confidence level with valid input file name!");
    std::vector<mitk::BaseData::Pointer> data = reader->Read();
    MITK_TEST_CONDITION_REQUIRED(!data.empty(), "Testing non-empty data with valid input file name!");

    // evaluate if the read point set is correct
    mitk::PointSet::Pointer resultPS = dynamic_cast<mitk::PointSet *>(data.front().GetPointer());
    MITK_TEST_CONDITION_REQUIRED(resultPS.IsNotNull(), "Testing correct BaseData type");
    MITK_TEST_CONDITION_REQUIRED(
      resultPS->GetTimeSteps() == 14,
      "Testing output time step generation!"); // CAVE: Only valid with the specified test data!
    MITK_TEST_CONDITION_REQUIRED(
      resultPS->GetPointSet(resultPS->GetTimeSteps() - 1)->GetNumberOfPoints() == 0,
      "Testing output time step generation with empty time step!"); // CAVE: Only valid with the specified test data!

    // Broken time series ids are rejected instead of being guessed or crashing.
    for (const char *id : {"two", "3abc", "-1", "", "18446744073709551615"})
      CPPUNIT_ASSERT_THROW(Read(reader, CreatePointSetFile(std::string("<time_series_id>") + id + "</time_series_id>")),
                           mitk::Exception);

    CPPUNIT_ASSERT_THROW(Read(reader, CreatePointSetFile("")), mitk::Exception);

    // Unknown elements next to the time series are ignored.
    data = Read(reader, CreatePointSetFile("<time_series_id>0</time_series_id>", "<comment>edited by hand</comment>"));
    resultPS = dynamic_cast<mitk::PointSet *>(data.front().GetPointer());
    MITK_TEST_CONDITION_REQUIRED(resultPS.IsNotNull() && resultPS->GetTimeSteps() == 1 && resultPS->GetSize(0) == 1,
                                 "Testing unknown element next to the time series");

    // Broken points are rejected instead of being guessed or crashing.
    const std::string timeStep0 = "<time_series_id>0</time_series_id>";
    for (const char *point : {"<point><id>0</id><x>ten</x><y>20</y><z>30</z></point>",
                              "<point><id>0</id><x></x><y>20</y><z>30</z></point>",
                              "<point><x>10</x><y>20</y><z>30</z></point>",
                              "<point><id>0</id><y>20</y><z>30</z></point>",
                              "<point><id>-1</id><x>10</x><y>20</y><z>30</z></point>"})
      CPPUNIT_ASSERT_THROW(Read(reader, CreatePointSetFile(timeStep0, "", point)), mitk::Exception);

    // Spaces around numbers are tolerated, as atoi and atof did.
    data = Read(reader, CreatePointSetFile("<time_series_id> 0 </time_series_id>", "",
                                           "<point><id> 0 </id><x> 10 </x><y>20</y><z>30</z></point>"));
    resultPS = dynamic_cast<mitk::PointSet *>(data.front().GetPointer());
    MITK_TEST_CONDITION_REQUIRED(resultPS.IsNotNull() && resultPS->GetSize(0) == 1 && resultPS->GetPoint(0, 0)[0] == 10.0,
                                 "Testing spaces around numbers");

    // Unknown elements between points and after the point set are ignored.
    data = Read(reader, CreatePointSetFile(timeStep0, "",
                                           DefaultPoint + "<comment>edited by hand</comment>" +
                                             "<point><id>1</id><x>40</x><y>50</y><z>60</z></point>",
                                           "<comment>edited by hand</comment>"));
    resultPS = dynamic_cast<mitk::PointSet *>(data.front().GetPointer());
    MITK_TEST_CONDITION_REQUIRED(data.size() == 1 && resultPS.IsNotNull() && resultPS->GetSize(0) == 2,
                                 "Testing unknown elements between points and after the point set");
  }

  // always end with this!
  MITK_TEST_END()
}
