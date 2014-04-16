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

//testing headers
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkNavigationDataRecorder.h>
#include <mitkNavigationDataPlayer.h>
#include <mitkNavigationData.h>
#include <mitkStandardFileLocations.h>
#include <mitkTestingMacros.h>
#include <mitkNavigationDataReaderXML.h>
#include <mitkNavigationDataSetWriterXML.h>

#include <Poco/Path.h>
#include <Poco/File.h>

#include <iostream>
#include <sstream>
#include <fstream>

//for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

class mitkNavigationDataSetReaderWriterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationDataSetReaderWriterTestSuite);
  MITK_TEST(TestCompareFunction);
  MITK_TEST(TestReadWrite);
  CPPUNIT_TEST_SUITE_END();
private:
  std::string pathRead;
  std::string pathWrite;
  std::string pathWrong;
  mitk::NavigationDataSetWriterXML writer;
  mitk::NavigationDataReaderXML::Pointer reader;
  mitk::NavigationDataSet::Pointer set;
public:
  void setUp()
  {
    pathRead = GetTestDataFilePath("IGT-Data/NavigationDataSet.xml");

    pathWrite = pathRead;
    pathWrite.insert(pathWrite.end()-4,'2');;//Insert X: IGT-Data/NavigationDataSet2.xml

    pathWrong = pathRead;
    pathWrong.insert(pathWrong.end()-4,'X');;//Insert X: IGT-Data/NavigationDataSetX.xml

    reader = mitk::NavigationDataReaderXML::New();
  }

  void tearDown()
  {
  }
  void TestReadWrite()
  {
    // Aim is to read an xml into a pointset, write that xml again, and compare the output

    set = reader->Read(pathRead);
    writer.Write(pathWrite, set);

    CPPUNIT_ASSERT_MESSAGE( "Testing if read/write cycle creates identical files", CompareFiles(pathRead, pathWrite));
  }

  bool CompareFiles(std::string file1, std::string file2)
  {
    FILE* f1 = fopen (file1.c_str() , "r");
    FILE* f2 = fopen (file2.c_str() , "r");
    char buf1[10000];
    char buf2[10000];

    do {
      size_t r1 = fread(buf1, 1, 10000, f1);
      size_t r2 = fread(buf2, 1, 10000, f2);

      if (r1 != r2 ||
        memcmp(buf1, buf2, r1)) {
          return false;  // Files are not equal
      }
    } while (!feof(f1) && !feof(f2));

    return feof(f1) && feof(f2);
  }

  void TestCompareFunction()
  {
    CPPUNIT_ASSERT_MESSAGE( "Asserting that compare function for files works correctly - Positive Test", CompareFiles(pathRead,
      pathRead));
    CPPUNIT_ASSERT_MESSAGE("Asserting that compare function for files works correctly - Negative Test", ! CompareFiles(pathRead,
      pathWrong) );
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataSetReaderWriter)
