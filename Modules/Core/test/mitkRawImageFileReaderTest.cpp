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
#include "mitkIOConstants.h"

class mitkRawImageFileReaderTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkRawImageFileReaderTestSuite);
  MITK_TEST(testReadFile);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different test methods. All members are initialized via setUp().*/
  std::string m_ImagePath;
  std::string m_ImagePathNrrdRef; //corresponding mhd path for comparision

public:

  /**
   * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
   */
  void setUp()
  {
    m_ImagePath = GetTestDataFilePath("brain.raw");
    m_ImagePathNrrdRef = GetTestDataFilePath("brainHalfSize.nrrd"); //we need half size because the brain file has spacing 2 and this reader doesn't support spacing
  }

  void tearDown()
  {
  }

  void testReadFile()
  {
    mitk::IFileReader::Options options;
    options[mitk::IOConstants::DIMENSION()] = 3;
    options[mitk::IOConstants::PIXEL_TYPE()] = mitk::IOConstants::PIXEL_TYPE_FLOAT();
    options[mitk::IOConstants::SIZE_X()] = 91;
    options[mitk::IOConstants::SIZE_Y()] = 109;
    options[mitk::IOConstants::SIZE_Z()] = 91;
    options[mitk::IOConstants::ENDIANNESS()] = mitk::IOConstants::ENDIANNESS_LITTLE();
    mitk::Image::Pointer readFile = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(m_ImagePath, options).front().GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Testing reading a raw file.",readFile.IsNotNull());

    //compare with the reference image
    mitk::Image::Pointer compareImage = mitk::IOUtil::LoadImage(m_ImagePathNrrdRef);
    MITK_ASSERT_EQUAL(compareImage, readFile, "Testing if image is equal to the same image as reference file loaded with mitk");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRawImageFileReader)
