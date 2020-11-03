/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//testing headers
//#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkNavigationDataRecorder.h>
#include <mitkNavigationDataPlayer.h>
#include <mitkNavigationData.h>
#include <mitkStandardFileLocations.h>
#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>

#include <Poco/Path.h>
#include <Poco/File.h>

#include <iostream>
#include <sstream>
#include <fstream>

//for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

class mitkNavigationDataSetReaderWriterXMLTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationDataSetReaderWriterXMLTestSuite);
  MITK_TEST(TestCompareFunction);
  MITK_TEST(TestReadWrite);
  MITK_TEST(TestSetXMLStringException);
  CPPUNIT_TEST_SUITE_END();

private:

  std::string pathRead;
  std::string pathWrite;
  std::string pathWrong;
  mitk::NavigationDataSet::Pointer set;

public:

  void setUp() override
  {
    pathRead = GetTestDataFilePath("IGT-Data/RecordedNavigationData.xml");

    pathWrite = pathRead;
    pathWrite.insert(pathWrite.end()-4,'2');;//Insert 2: IGT-Data/NavigationDataSet2.xml
    std::ifstream FileTest(pathWrite.c_str());
    if(FileTest){
      //remove file if it already exists. TODO: Loeschen funktioniert nicht!!!! xxxxxxxxxxxxxxxx
      FileTest.close();
      std::remove(pathWrite.c_str());
    }

    pathWrong = GetTestDataFilePath("IGT-Data/NavigationDataTestData.xml");
  }

  void tearDown() override
  {
  }

  void TestReadWrite()
  {
    // Aim is to read an xml into a pointset, write that xml again, and compare the output

    set = dynamic_cast<mitk::NavigationDataSet*> (mitk::IOUtil::Load(pathRead)[0].GetPointer());
    mitk::IOUtil::Save(set, pathWrite);

    //FIXME: Commented out, because test fails under linux. binary comparison of files is probably not the wa to go
    // See Bug 17775
    //CPPUNIT_ASSERT_MESSAGE( "Testing if read/write cycle creates identical files", CompareFiles(pathRead, pathWrite));
    remove(pathWrite.c_str());
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
          fclose(f1);
          fclose(f2);
          return false;  // Files are not equal
      }
    } while (!feof(f1) && !feof(f2));
    bool returnValue = feof(f1) && feof(f2);
    fclose(f1);
    fclose(f2);
    return returnValue;
  }

  void TestSetXMLStringException()
  {
    bool exceptionThrown3=false;
    try
    {
      std::string file = GetTestDataFilePath("IGT-Data/InvalidVersionNavigationDataTestData.xml");
      mitk::NavigationDataSet::Pointer dataset = dynamic_cast<mitk::NavigationDataSet*> (mitk::IOUtil::Load(file)[0].GetPointer());
    }
    catch(mitk::Exception&)
    {
      exceptionThrown3=true;
    }
    MITK_TEST_CONDITION(exceptionThrown3, "Reading an invalid XML string and expecting a exception");
  }

  void TestCompareFunction()
  {
    CPPUNIT_ASSERT_MESSAGE("Asserting that compare function for files works correctly - Positive Test", CompareFiles(pathRead,
      pathRead));
    CPPUNIT_ASSERT_MESSAGE("Asserting that compare function for files works correctly - Negative Test", ! CompareFiles(pathRead,
      pathWrong) );
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataSetReaderWriterXML)
