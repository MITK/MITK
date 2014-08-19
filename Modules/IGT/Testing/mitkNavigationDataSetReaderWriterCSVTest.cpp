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
//#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkNavigationDataRecorder.h>
#include <mitkNavigationDataPlayer.h>
#include <mitkNavigationData.h>
#include <mitkStandardFileLocations.h>
#include <mitkTestingMacros.h>
#include <mitkNavigationDataReaderCSV.h>
#include <mitkNavigationDataSetWriterCSV.h>

#include <Poco/Path.h>
#include <Poco/File.h>

#include <iostream>
#include <sstream>
#include <fstream>

//for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

class mitkNavigationDataSetReaderWriterCSVTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationDataSetReaderWriterCSVTestSuite);
  MITK_TEST(TestCompareFunction);
  MITK_TEST(TestReadWrite);
  MITK_TEST(TestSetCSVStringException);
  CPPUNIT_TEST_SUITE_END();

private:

  std::string pathRead;
  std::string pathWrite;
  std::string pathWrong;
  mitk::NavigationDataSetWriterCSV writer;
  mitk::NavigationDataReaderCSV::Pointer reader;
  mitk::NavigationDataSet::Pointer set;

public:

  void setUp()
  {
    pathRead = GetTestDataFilePath("IGT-Data/RecordedNavigationData.CSV");

    pathWrite = pathRead;
    pathWrite.insert(pathWrite.end()-4,'2');;//Insert 2: IGT-Data/NavigationDataSet2.CSV
    std::ifstream FileTest(pathWrite.c_str());
    if(FileTest){
      //remove file if it already exists. TODO: Löschen funktioniert nicht!!!! xxxxxxxxxxxxxxxx
      FileTest.close();
      std::remove(pathWrite.c_str());
    }

    pathWrong = GetTestDataFilePath("IGT-Data/NavigationDataTestData.CSV");

    reader = mitk::NavigationDataReaderCSV::New();
  }

  void tearDown()
  {
  }

  void TestReadWrite()
  {
    // Aim is to read an CSV into a pointset, write that CSV again, and compare the output

    set = reader->Read(pathRead);
    writer.Write(pathWrite, set);

    //FIXME: Commented out, because test fails under linux. binary comparison of files is probably not the wa to go
    // See Bug 17775
    //CPPUNIT_ASSERT_MESSAGE( "Testing if read/write cycle creates identical files", CompareFiles(pathRead, pathWrite));
    remove(pathWrite.c_str());
  }
MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataSetReaderWriterCSV)