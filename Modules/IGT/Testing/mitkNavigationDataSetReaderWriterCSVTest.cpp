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

class mitkNavigationDataSetReaderWriterCSVTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationDataSetReaderWriterCSVTestSuite);
 // MITK_TEST(TestCompareFunction);
  MITK_TEST(TestReadWrite);
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

    pathWrong = GetTestDataFilePath("IGT-Data/NavigationDataTestData.CSV");
    pathWrite="C:\\test.csv";
  }

  void tearDown() override
  {
  }


  void TestReadWrite()
  {
    // Aim is to read an CSV into a pointset, write that CSV again, and compare the output

    set = mitk::IOUtil::Load<mitk::NavigationDataSet>(pathRead);
    CPPUNIT_ASSERT_MESSAGE("Testing whether something was read at all", set != nullptr);

    mitk::IOUtil::Save(set, pathWrite);

    //FIXME: Commented out, because test fails under linux. binary comparison of files is probably not the wa to go
    // See Bug 17775
    //CPPUNIT_ASSERT_MESSAGE( "Testing if read/write cycle creates identical files", CompareFiles(pathRead, pathWrite));
    //remove(pathWrite.c_str());
  }





  bool CompareFiles(std::string file)
  {
    set = mitk::IOUtil::Load<mitk::NavigationDataSet>(file);

    double sample[2][30] ={
      {5134019.44, 0, 0, 0,  0,  0,  0,  0,  0,  0,                                                                                       5134019.44, 0,   1,  101.2300034,  -62.63999939,  -203.2400055,  -0.3059000075,  0.5752000213,  0,  0.7585999966,                     5134019.44, 0, 0,  0,  0,  0,  0,  0,  0,  0},
      {5134082.84,  5134073.64,  1,  -172.6100006,  12.60999966,  -299.4500122,  -0.1588999927,  0.4370000064,  0,  0.8852000237,         5134082.84,  5134073.64, 1,  101.2300034,  -62.63999939,  -203.2400055,  -0.3059000075,  0.5752000213,  0,  0.7585999966,             5134082.84,  5134073.64, 0,  0,  0,  0,  0,  0,  0,  0}
    };

    bool returnValue = true;
    for(int line = 0 ; line < 2; line++)
    {

      for (int tool =0; tool < 3; tool ++)
      {
        mitk::NavigationData::Pointer testline = set->GetNavigationDataForIndex(line,tool) ;

        returnValue = returnValue &&   mitk::Equal( testline->GetIGTTimeStamp() , sample [line] [(tool*10)] );
        returnValue = returnValue &&      mitk::Equal( testline->IsDataValid() , sample [line] [(tool*10)+1] );

        mitk::NavigationData::PositionType pos = testline->GetPosition();
        returnValue = returnValue &&      mitk::Equal( pos[0] , sample [line] [(tool*10)+2] );
        returnValue = returnValue &&      mitk::Equal( pos[1] , sample [line] [(tool*10)+3] );
        returnValue = returnValue &&        mitk::Equal( pos[2] , sample [line] [(tool*10)+4] );

        mitk::NavigationData::OrientationType ori = testline->GetOrientation();
        returnValue = returnValue &&        mitk::Equal( ori[0] , sample [line] [(tool*10)+5] );
        returnValue = returnValue &&        mitk::Equal( ori[1] , sample [line] [(tool*10)+6] );
        returnValue = returnValue &&        mitk::Equal( ori[2] , sample [line] [(tool*10)+7] );
        returnValue = returnValue &&        mitk::Equal( ori[3] , sample [line] [(tool*10)+8] );
      }
    }
    return returnValue;
  }


  void TestCompareFunction()
  {
    CPPUNIT_ASSERT_MESSAGE("Checking if csv-file reader is working properly", CompareFiles(pathRead));
    //CPPUNIT_ASSERT_MESSAGE("Asserting that compare function for files works correctly - Negative Test", ! CompareFiles(pathWrong) );
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataSetReaderWriterCSV)
