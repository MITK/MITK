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

#include "mitkUSImageLoggingFilter.h"
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>

#include "mitkImageGenerator.h"

#include "Poco/File.h"

class mitkUSImageLoggingFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkUSImageLoggingFilterTestSuite);
  MITK_TEST(TestInstantiation);
  MITK_TEST(TestFilterWithEmptyImages);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::USImageLoggingFilter::Pointer m_TestFilter;
  std::string m_TemporaryTestDirectory;

public:

  void setUp()
  {
    m_TestFilter = mitk::USImageLoggingFilter::New();
    m_TemporaryTestDirectory = mitk::IOUtil::GetTempPath();
    //m_ImagePath = GetTestDataFilePath("Pic3D.nrrd");
    //m_SurfacePath = GetTestDataFilePath("binary.stl");
    //m_PointSetPath = GetTestDataFilePath("pointSet.mps");
  }

  void tearDown()
  {
    m_TestFilter = NULL;
  }

  void TestInstantiation()
  {
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation",m_TestFilter.IsNotNull());
  }

  void TestFilterWithEmptyImages()
  {
  //create empty test images
  mitk::Image::Pointer testImage = mitk::Image::New();
  mitk::Image::Pointer testImage2 = mitk::Image::New();

  //set both as input for the filter
  m_TestFilter->SetInput(testImage);
  CPPUNIT_ASSERT_MESSAGE("Testing SetInput(...) for first input.",m_TestFilter->GetNumberOfInputs()==1);
  m_TestFilter->SetInput("secondImage",testImage2);
  CPPUNIT_ASSERT_MESSAGE("Testing SetInput(...) for second input.",m_TestFilter->GetNumberOfInputs()==2);

  //images are empty, but update method should not crash
  CPPUNIT_ASSERT_NO_THROW_MESSAGE("Tested method Update() with invalid data.",m_TestFilter->Update());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkUSImageLoggingFilter)

///**
//* This function is testing methods of the class USDevice.
//*/
//int mitkUSImageLoggingFilterTest(int /* argc */, char* /*argv*/[])
//{
//  MITK_TEST_BEGIN("mitkUSDeviceTest");
//
//  std::string temporaryTestDirectory = MITK_TEST_OUTPUT_DIR;
//
//  //######################## Test with empty test images ################################
//
//
//  //######################## Test with valid test images ################################
//  testFilter = mitk::USImageLoggingFilter::New();
//  testImage = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 100, 1, 0.2, 0.3, 0.4);
//  testImage2 = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 100, 1, 0.2, 0.3, 0.4);
//  testFilter->SetInput(testImage);
//  testFilter->SetInput("secondImage",testImage2);
//  testFilter->Update();
//  MITK_TEST_OUTPUT(<<"Tested method Update() with valid data.");
//  std::vector<std::string> filenames;
//  std::string csvFileName;
//  testFilter->SaveImages(temporaryTestDirectory,filenames,csvFileName);
//  MITK_TEST_OUTPUT(<<"Tested method SaveImages(...).");
//  MITK_TEST_CONDITION_REQUIRED(filenames.size() == 1,"Testing if correct number of images was saved");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(filenames.at(0).c_str()).exists(),"Testing if image file exists");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(csvFileName.c_str()).exists(),"Testing if csv file exists");
//
//  //clean up
//  std::remove(filenames.at(0).c_str());
//  std::remove(csvFileName.c_str());
//
//
//  //######################## Test multiple calls of update ################################
//  for(int i=0; i<5; i++)
//    {
//    testFilter->Update();
//    std::stringstream testmessage;
//    testmessage << "testmessage" << i;
//    testFilter->AddMessageToCurrentImage(testmessage.str());
//    Sleep(50);
//    }
//  MITK_TEST_OUTPUT(<<"Call Update() 5 times.");
//
//  testFilter->SaveImages(temporaryTestDirectory,filenames,csvFileName);
//  MITK_TEST_OUTPUT(<<"Tested method SaveImages(...).");
//  MITK_TEST_CONDITION_REQUIRED(filenames.size() == 6,"Testing if correct number of images was saved");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(filenames.at(0).c_str()).exists(),"Testing if file 1 exists");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(filenames.at(1).c_str()).exists(),"Testing if file 2 exists");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(filenames.at(2).c_str()).exists(),"Testing if file 3 exists");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(filenames.at(3).c_str()).exists(),"Testing if file 4 exists");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(filenames.at(4).c_str()).exists(),"Testing if file 5 exists");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(filenames.at(5).c_str()).exists(),"Testing if file 6 exists");
//  MITK_TEST_CONDITION_REQUIRED(Poco::File(csvFileName.c_str()).exists(),"Testing if csv file exists");
//
//  //clean up
//  for(int i=0; i<filenames.size(); i++) std::remove(filenames.at(i).c_str());
//  std::remove(csvFileName.c_str());
//
//  MITK_TEST_END();
//}
