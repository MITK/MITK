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

class mitkNavigationDataSetReaderWriterTestClass
{
public:

  static void TestReadWrite()
  {
    // Aim is to read an xml into a pointset, write that xml again, and compare the output

    mitk::NavigationDataSetWriterXML writer;
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();

    std::string path = "F://Build//MITK-Data//IGT-Data//NavigationDataSet.xml";
    mitk::NavigationDataSet::Pointer set = reader->Read(path);
    writer.Write("F://Build//MITK-Data//IGT-Data//NavigationDataSet2.xml", set);

    MITK_TEST_CONDITION_REQUIRED(mitkNavigationDataSetReaderWriterTestClass::CompareFiles("F://Build//MITK-Data//IGT-Data//NavigationDataSet.xml",
      "F://Build//MITK-Data//IGT-Data//NavigationDataSet2.xml"), "Testing if read/write cycle creates identical files");
  }

  static bool CompareFiles(std::string file1, std::string file2)
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
};

/**Documentation
*  test for the class "NavigationDataRecorder".
*/
int mitkNavigationDataSetReaderWriterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataRecorder");

  MITK_TEST_CONDITION_REQUIRED(mitkNavigationDataSetReaderWriterTestClass::CompareFiles("F://Build//MITK-Data//IGT-Data//NavigationDataSet.xml",
    "F://Build//MITK-Data//IGT-Data//NavigationDataSet.xml"), "Asserting that compare function for files works correctly - Positive Test");
  MITK_TEST_CONDITION_REQUIRED(! mitkNavigationDataSetReaderWriterTestClass::CompareFiles("F://Build//MITK-Data//IGT-Data//NavigationDataSet.xml",
    "F://Build//MITK-Data//IGT-Data//SROMFile.rom"), "Asserting that compare function for files works correctly - Negative Test");
  mitkNavigationDataSetReaderWriterTestClass::TestReadWrite();

  MITK_TEST_END();
};
