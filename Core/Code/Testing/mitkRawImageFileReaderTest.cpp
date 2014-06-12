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


#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"
#include "mitkIOUtil.h"

class mitkRawImageFileReaderTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkRawImageFileReaderTestSuite);
  MITK_TEST(testInstantiation);
  MITK_TEST(testReadFile);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::IFileReader* m_FileReader;

  std::string m_ImagePath;
  std::string m_ImagePathNrrdRef; //corresponding mhd path for comparision

public:

  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp()
  {
    m_FileReader = mitk::RawImageFileReader::New();
    m_ImagePath = GetTestDataFilePath("brain.raw");
    m_ImagePathNrrdRef = GetTestDataFilePath("brainHalfSize.nrrd"); //we need half size because the brain file has spacing 2 and this reader doesn't support spacing
  }

  void tearDown()
  {
    m_FileReader = NULL;
  }

  void testInstantiation()
  {
    m_FileReader = mitk::RawImageFileReader::New();
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of RawImageFileReader.",m_FileReader.IsNotNull());
  }

  void testReadFile()
  {
    m_FileReader->SetFileName(m_ImagePath.c_str());
    m_FileReader->SetPixelType(mitk::RawImageFileReader::FLOAT);
    m_FileReader->SetDimensionality(3);
    m_FileReader->SetDimensions(0,91);
    m_FileReader->SetDimensions(1,109);
    m_FileReader->SetDimensions(2,91);
    m_FileReader->SetEndianity(mitk::RawImageFileReader::LITTLE);
    m_FileReader->Update();
    mitk::Image::Pointer readFile = m_FileReader->GetOutput();
    CPPUNIT_ASSERT_MESSAGE("Testing reading a raw file.",readFile.IsNotNull());

    //compare with the reference image
    mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(m_ImagePathNrrdRef);
    CPPUNIT_ASSERT_MESSAGE("Testing if image is equal to the same image as reference file loaded with mitk",mitk::Equal(compareImage,readFile,mitk::eps,true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRawImageFileReader)
