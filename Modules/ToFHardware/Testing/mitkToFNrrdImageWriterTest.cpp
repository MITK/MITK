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

#include <mitkImage.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkToFNrrdImageWriter.h>
#include <mitkToFConfig.h>
#include <mitkIOUtil.h>
#include <mitkImageGenerator.h>
#include <mitkImageReadAccessor.h>

class mitkToFNrrdImageWriterTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkToFNrrdImageWriterTestSuite);
  MITK_TEST(GetExtension_DefaultValueIsNrrd);
  MITK_TEST(Add_WriteDistanceImage_OutputImageIsEqualToInput);
  //Work in progress:
//  MITK_TEST(Add_WriteDistanceAndAmplitudeImage_OutputImagesAreEqualToInput);
//  MITK_TEST(Add_WriteDistanceAndIntensityImage_OutputImagesAreEqualToInput);
//  MITK_TEST(Add_WriteDistanceAndIntensityAndAmplitudeImage_OutputImagesAreEqualToInput);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().
    * Every recorder needs a device. Here we use a player device.
    * The player device needs data to load. The ground truth is loaded via IOUtil.
    */
  mitk::ToFNrrdImageWriter::Pointer m_ToFNrrdImageWriter;
  std::string m_DistanceImageName;
  std::string m_AmplitudeImageName;
  std::string m_IntensityImageName;

  mitk::Image::Pointer m_GroundTruthDepthImage;
  mitk::Image::Pointer m_GroundTruthIntensityImage;
  mitk::Image::Pointer m_GroundTruthAmplitudeImage;

  unsigned int m_NumberOfFrames;

public:

  /**
    * @brief Setup a recorder including a device. Here, we use a player, because in an automatic test
    * hardware is not useful.
    */
  void setUp()
  {
    m_ToFNrrdImageWriter = mitk::ToFNrrdImageWriter::New();
    m_ToFNrrdImageWriter->SetToFImageType(mitk::ToFNrrdImageWriter::ToFImageType3D); //we test the ToFImageType3D since 3D is deprecated?

    //Generate some random test data
    unsigned int dimX = 255;
    unsigned int dimY = 178;
    m_NumberOfFrames = 23;
    m_GroundTruthDepthImage= mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, m_NumberOfFrames,1.0, 1.0f, 1.0f);
    m_GroundTruthAmplitudeImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, m_NumberOfFrames,1.0, 1.0f, 2000.0f);
    m_GroundTruthIntensityImage = mitk::ImageGenerator::GenerateRandomImage<float>(dimX, dimY, m_NumberOfFrames,1.0, 1.0f, 100000.0f);

    m_ToFNrrdImageWriter->SetToFCaptureWidth(dimX);
    m_ToFNrrdImageWriter->SetToFCaptureHeight(dimY);

    m_DistanceImageName = "test_DistanceImage.nrrd";
    m_AmplitudeImageName = "test_AmplitudeImage.nrrd";
    m_IntensityImageName = "test_IntensityImage.nrrd";
  }

  void tearDown()
  {
  }

  void GetExtension_DefaultValueIsNrrd()
  {
    MITK_TEST_CONDITION_REQUIRED(m_ToFNrrdImageWriter->GetExtension() == ".nrrd", "Is .nrrd the default extension?");
  }

  void Add_WriteDistanceImage_OutputImageIsEqualToInput()
  {
    m_ToFNrrdImageWriter->SetDistanceImageFileName(m_DistanceImageName);

    //buffer for each slice
    float* distanceArray;

    m_ToFNrrdImageWriter->Open(); //open file/stream
    for(unsigned int i = 0; i < m_NumberOfFrames ; ++i)
    {
      mitk::ImageReadAccessor distAcc(m_GroundTruthDepthImage, m_GroundTruthDepthImage->GetSliceData(i, 0, 0));
      distanceArray = (float*)distAcc.GetData();

      //write (or add) the three slices to the file
      m_ToFNrrdImageWriter->Add(distanceArray, NULL, NULL);
    }
    m_ToFNrrdImageWriter->Close(); //close file

    mitk::Image::Pointer writtenImage = mitk::IOUtil::LoadImage( m_DistanceImageName );
    MITK_ASSERT_EQUAL( m_GroundTruthDepthImage, writtenImage, "Written image should be equal to the test data.");

    //clean up tmp written image
    remove( m_DistanceImageName.c_str() );
  }

  void Add_WriteDistanceAndAmplitudeImage_OutputImagesAreEqualToInput()
  {
    m_ToFNrrdImageWriter->SetDistanceImageFileName(m_DistanceImageName);
    m_ToFNrrdImageWriter->SetAmplitudeImageFileName(m_AmplitudeImageName);
    m_ToFNrrdImageWriter->SetAmplitudeImageSelected(true);


    //buffer for each slice
    float* distanceArray;
    float* amplitudeArray;

    m_ToFNrrdImageWriter->Open(); //open file/stream
    for(unsigned int i = 0; i < m_NumberOfFrames ; ++i)
    {
      mitk::ImageReadAccessor distAcc(m_GroundTruthDepthImage, m_GroundTruthDepthImage->GetSliceData(i, 0, 0));
      mitk::ImageReadAccessor amplAcc(m_GroundTruthAmplitudeImage, m_GroundTruthAmplitudeImage->GetSliceData(i, 0, 0));
      distanceArray = (float*)distAcc.GetData();
      amplitudeArray = (float*)amplAcc.GetData();

      //write (or add) the three slices to the file
      m_ToFNrrdImageWriter->Add(distanceArray, amplitudeArray, NULL);
    }
    m_ToFNrrdImageWriter->Close(); //close file

    mitk::Image::Pointer writtenDepthImage = mitk::IOUtil::LoadImage( m_DistanceImageName );
    mitk::Image::Pointer writtenAmplitudeImage = mitk::IOUtil::LoadImage( m_AmplitudeImageName );

    MITK_ASSERT_EQUAL( m_GroundTruthDepthImage, writtenDepthImage, "Written depth image should be equal to the test data.");
    MITK_ASSERT_EQUAL( m_GroundTruthAmplitudeImage, writtenAmplitudeImage, "Written amplitude image should be equal to the test data.");

    //clean up tmp written image
    remove( m_DistanceImageName.c_str() );
    remove( m_AmplitudeImageName.c_str() );
  }

  void Add_WriteDistanceAndIntensityImage_OutputImagesAreEqualToInput()
  {
    m_ToFNrrdImageWriter->SetDistanceImageFileName(m_DistanceImageName);
    m_ToFNrrdImageWriter->SetIntensityImageFileName(m_IntensityImageName);
    m_ToFNrrdImageWriter->SetIntensityImageSelected(true);

    //buffer for each slice
    float* distanceArray;
    float* intensityArray;

    m_ToFNrrdImageWriter->Open(); //open file/stream
    for(unsigned int i = 0; i < m_NumberOfFrames ; ++i)
    {
      mitk::ImageReadAccessor distAcc(m_GroundTruthDepthImage, m_GroundTruthDepthImage->GetSliceData(i, 0, 0));
      mitk::ImageReadAccessor intensityAcc(m_GroundTruthIntensityImage, m_GroundTruthIntensityImage->GetSliceData(i, 0, 0));
      distanceArray = (float*)distAcc.GetData();
      intensityArray = (float*)intensityAcc.GetData();

      //write (or add) the three slices to the file
      m_ToFNrrdImageWriter->Add(distanceArray, NULL, intensityArray);
    }
    m_ToFNrrdImageWriter->Close(); //close file

    mitk::Image::Pointer writtenDepthImage = mitk::IOUtil::LoadImage( m_DistanceImageName );
    mitk::Image::Pointer writtenIntensityImage = mitk::IOUtil::LoadImage( m_IntensityImageName );

    MITK_ASSERT_EQUAL( m_GroundTruthDepthImage, writtenDepthImage, "Written depth image should be equal to the test data.");
    MITK_ASSERT_EQUAL( m_GroundTruthIntensityImage, writtenIntensityImage, "Written amplitude image should be equal to the test data.");

    //clean up tmp written image
    remove( m_DistanceImageName.c_str() );
    remove( m_IntensityImageName.c_str() );
  }

  void Add_WriteDistanceAndIntensityAndAmplitudeImage_OutputImagesAreEqualToInput()
  {
    m_ToFNrrdImageWriter->SetDistanceImageFileName(m_DistanceImageName);
    m_ToFNrrdImageWriter->SetIntensityImageFileName(m_IntensityImageName);
    m_ToFNrrdImageWriter->SetIntensityImageSelected(true);
    m_ToFNrrdImageWriter->SetAmplitudeImageFileName(m_AmplitudeImageName);
    m_ToFNrrdImageWriter->SetAmplitudeImageSelected(true);

    //buffer for each slice
    float* distanceArray;
    float* intensityArray;
    float* amplitudeArray;

    m_ToFNrrdImageWriter->Open(); //open file/stream
    for(unsigned int i = 0; i < m_NumberOfFrames ; ++i)
    {
      mitk::ImageReadAccessor distAcc(m_GroundTruthDepthImage, m_GroundTruthDepthImage->GetSliceData(i, 0, 0));
      mitk::ImageReadAccessor intensityAcc(m_GroundTruthIntensityImage, m_GroundTruthIntensityImage->GetSliceData(i, 0, 0));
      mitk::ImageReadAccessor amplAcc(m_GroundTruthAmplitudeImage, m_GroundTruthAmplitudeImage->GetSliceData(i, 0, 0));

      distanceArray = (float*)distAcc.GetData();
      intensityArray = (float*)intensityAcc.GetData();
      amplitudeArray = (float*)amplAcc.GetData();

      //write (or add) the three slices to the file
      m_ToFNrrdImageWriter->Add(distanceArray, amplitudeArray, intensityArray);
    }
    m_ToFNrrdImageWriter->Close(); //close file

    mitk::Image::Pointer writtenDepthImage = mitk::IOUtil::LoadImage( m_DistanceImageName );
    mitk::Image::Pointer writtenIntensityImage = mitk::IOUtil::LoadImage( m_IntensityImageName );
    mitk::Image::Pointer writtenAmplitudeImage = mitk::IOUtil::LoadImage( m_AmplitudeImageName );

    MITK_ASSERT_EQUAL( m_GroundTruthDepthImage, writtenDepthImage, "Written depth image should be equal to the test data.");
    MITK_ASSERT_EQUAL( m_GroundTruthAmplitudeImage, writtenAmplitudeImage, "Written amplitude image should be equal to the test data.");
    MITK_ASSERT_EQUAL( m_GroundTruthIntensityImage, writtenIntensityImage, "Written amplitude image should be equal to the test data.");

    //clean up tmp written image
    remove( m_DistanceImageName.c_str() );
    remove( m_IntensityImageName.c_str() );
    remove( m_AmplitudeImageName.c_str() );
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToFNrrdImageWriter)
