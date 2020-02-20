/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSImageLoggingFilter.h"
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>

#include <mitkIOMimeTypes.h>
#include <mitkCoreServices.h>
#include <mitkIMimeTypeProvider.h>

#include "mitkImageGenerator.h"

#include "itksys/SystemTools.hxx"

#include "Poco/File.h"

class mitkUSImageLoggingFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkUSImageLoggingFilterTestSuite);
  MITK_TEST(TestInstantiation);
  MITK_TEST(TestSetFileExtension);
  MITK_TEST(TestSetWrongFileExtension);
  MITK_TEST(TestSavingValidTestImage);
  MITK_TEST(TestSavingAfterMupltipleUpdateCalls);
  MITK_TEST(TestFilterWithEmptyImages);
  MITK_TEST(TestFilterWithInvalidPath);
  //MITK_TEST(TestJpgFileExtension); //bug 19614
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::USImageLoggingFilter::Pointer m_TestFilter;
  std::string m_TemporaryTestDirectory;
  mitk::Image::Pointer m_RandomRestImage1;
  mitk::Image::Pointer m_RandomRestImage2;
  mitk::Image::Pointer m_RandomSingleSliceImage;
  mitk::Image::Pointer m_RealTestImage;

public:

  void setUp() override
  {
    m_TestFilter = mitk::USImageLoggingFilter::New();
    m_TemporaryTestDirectory = mitk::IOUtil::GetTempPath();
    m_RandomRestImage1 = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 100, 1, 0.2, 0.3, 0.4);
    m_RandomRestImage2 = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 100, 1, 0.2, 0.3, 0.4);
    m_RandomSingleSliceImage = mitk::ImageGenerator::GenerateRandomImage<float>(100, 100, 1, 1, 0.2, 0.3, 0.4);
    m_RealTestImage = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
  }

  void tearDown() override
  {
    m_TestFilter = nullptr;
    m_RandomRestImage1 = nullptr;
    m_RandomRestImage2 = nullptr;
    m_RealTestImage = nullptr;
    m_RandomSingleSliceImage = nullptr;
  }

  void TestInstantiation()
  {
  CPPUNIT_ASSERT_MESSAGE("Testing instantiation",m_TestFilter.IsNotNull());
  }

  void TestSavingValidTestImage()
  {
  //######################## Test with valid test images ################################
  m_TestFilter->SetInput(m_RandomRestImage1);
  m_TestFilter->SetInput("secondImage",m_RandomRestImage2);
  m_TestFilter->Update();
  MITK_TEST_OUTPUT(<<"Tested method Update() with valid data.");
  std::vector<std::string> filenames;
  std::string csvFileName;
  m_TestFilter->SaveImages(m_TemporaryTestDirectory,filenames,csvFileName);
  MITK_TEST_OUTPUT(<<"Tested method SaveImages(...).");
  CPPUNIT_ASSERT_MESSAGE("Testing if correct number of images was saved",filenames.size() == 1);
  CPPUNIT_ASSERT_MESSAGE("Testing if image file exists",Poco::File(filenames.at(0).c_str()).exists());
  CPPUNIT_ASSERT_MESSAGE("Testing if csv file exists",Poco::File(csvFileName.c_str()).exists());

  //clean up
  std::remove(filenames.at(0).c_str());
  std::remove(csvFileName.c_str());
  }

  void TestSavingAfterMupltipleUpdateCalls()
  {
  //######################## Test multiple calls of update ################################
  m_TestFilter->SetInput(m_RandomRestImage1);
  m_TestFilter->SetInput("secondImage",m_RandomRestImage2);

  for(int i=0; i<5; i++)
    {
    m_TestFilter->Update();
    std::stringstream testmessage;
    testmessage << "testmessage" << i;
    m_TestFilter->AddMessageToCurrentImage(testmessage.str());
    itksys::SystemTools::Delay(50);
    }
  MITK_TEST_OUTPUT(<<"Call Update() 5 times.");

  std::vector<std::string> filenames;
  std::string csvFileName;
  m_TestFilter->SaveImages(m_TemporaryTestDirectory,filenames,csvFileName);
  MITK_TEST_OUTPUT(<<"Tested method SaveImages(...).");
  CPPUNIT_ASSERT_MESSAGE("Testing if correct number of images was saved",filenames.size() == 5);
  CPPUNIT_ASSERT_MESSAGE("Testing if file 1 exists",Poco::File(filenames.at(0).c_str()).exists());
  CPPUNIT_ASSERT_MESSAGE("Testing if file 2 exists",Poco::File(filenames.at(1).c_str()).exists());
  CPPUNIT_ASSERT_MESSAGE("Testing if file 3 exists",Poco::File(filenames.at(2).c_str()).exists());
  CPPUNIT_ASSERT_MESSAGE("Testing if file 4 exists",Poco::File(filenames.at(3).c_str()).exists());
  CPPUNIT_ASSERT_MESSAGE("Testing if file 5 exists",Poco::File(filenames.at(4).c_str()).exists());
  CPPUNIT_ASSERT_MESSAGE("Testing if csv file exists",Poco::File(csvFileName.c_str()).exists());

  //clean up
  for(size_t i=0; i<filenames.size(); i++) std::remove(filenames.at(i).c_str());
  std::remove(csvFileName.c_str());
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

  void TestFilterWithInvalidPath()
  {
  #ifdef WIN32
  std::string filename = "XV:/342INVALID<>"; //invalid filename for windows
  #else
  std::string filename = "/dsfdsf:$�$342INVALID"; //invalid filename for linux
  #endif

  m_TestFilter->SetInput(m_RealTestImage);
  m_TestFilter->Update();
  CPPUNIT_ASSERT_THROW_MESSAGE("Testing if correct exception if thrown if an invalid path is given.",
                               m_TestFilter->SaveImages(filename),
                               mitk::Exception);
  }

  void TestJpgFileExtension()
  {
  CPPUNIT_ASSERT_MESSAGE("Testing setting of jpg extension.",m_TestFilter->SetImageFilesExtension(".jpg"));
  m_TestFilter->SetInput(m_RandomSingleSliceImage);
  m_TestFilter->Update();

  std::vector<std::string> filenames;
  std::string csvFileName;
  m_TestFilter->SaveImages(m_TemporaryTestDirectory,filenames,csvFileName);
  CPPUNIT_ASSERT_MESSAGE("Testing if correct number of images was saved",filenames.size() == 1);
  CPPUNIT_ASSERT_MESSAGE("Testing if jpg image file exists",Poco::File(filenames.at(0).c_str()).exists());
  CPPUNIT_ASSERT_MESSAGE("Testing if csv file exists",Poco::File(csvFileName.c_str()).exists());

  //clean up
  std::remove(filenames.at(0).c_str());
  std::remove(csvFileName.c_str());
  }

  void TestSetFileExtension()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing if PIC extension can be set.",m_TestFilter->SetImageFilesExtension("PIC"));
    CPPUNIT_ASSERT_MESSAGE("Testing if bmp extension can be set.",m_TestFilter->SetImageFilesExtension("bmp"));
    CPPUNIT_ASSERT_MESSAGE("Testing if gdc extension can be set.",m_TestFilter->SetImageFilesExtension("gdcm"));
    CPPUNIT_ASSERT_MESSAGE("Testing if dcm extension can be set.",m_TestFilter->SetImageFilesExtension("dcm"));
    CPPUNIT_ASSERT_MESSAGE("Testing if dc3 extension can be set.",m_TestFilter->SetImageFilesExtension("dc3"));
    CPPUNIT_ASSERT_MESSAGE("Testing if ima extension can be set.",m_TestFilter->SetImageFilesExtension(".ima"));
    CPPUNIT_ASSERT_MESSAGE("Testing if img extension can be set.",m_TestFilter->SetImageFilesExtension("img"));
    CPPUNIT_ASSERT_MESSAGE("Testing if gip extension can be set.",m_TestFilter->SetImageFilesExtension("gipl"));
    CPPUNIT_ASSERT_MESSAGE("Testing if gipl.gz extension can be set.",m_TestFilter->SetImageFilesExtension(".gipl.gz"));
    CPPUNIT_ASSERT_MESSAGE("Testing if jpg extension can be set.",m_TestFilter->SetImageFilesExtension("jpg"));
    CPPUNIT_ASSERT_MESSAGE("Testing if jpe extension can be set.",m_TestFilter->SetImageFilesExtension("jpeg"));
    CPPUNIT_ASSERT_MESSAGE("Testing if pic extension can be set.",m_TestFilter->SetImageFilesExtension("pic"));
  }

  void TestSetWrongFileExtension()
  {

    CPPUNIT_ASSERT_MESSAGE("Testing if wrong obj  extension is recognized",!m_TestFilter->SetImageFilesExtension("obj "));
    CPPUNIT_ASSERT_MESSAGE("Testing if wrong stl  extension is recognized",!m_TestFilter->SetImageFilesExtension("stl "));
    CPPUNIT_ASSERT_MESSAGE("Testing if wrong pvtp extension is recognized",!m_TestFilter->SetImageFilesExtension("pvtp"));
    CPPUNIT_ASSERT_MESSAGE("Testing if wrong vtp  extension is recognized",!m_TestFilter->SetImageFilesExtension("vtp "));
    CPPUNIT_ASSERT_MESSAGE("Testing if wrong vtk  extension is recognized",!m_TestFilter->SetImageFilesExtension("vtk "));

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkUSImageLoggingFilter)
