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

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include "mitkIOUtil.h"
#include <cmath>

#include <mitkGIFCooccurenceMatrix2.h>

class mitkGIFCooc2TestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFCooc2TestSuite);

  MITK_TEST(ImageDescription_PhantomTest_3D);
  //MITK_TEST(ImageDescription_PhantomTest_2D);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Image::Pointer m_IBSI_Phantom_Image_Small;
  mitk::Image::Pointer m_IBSI_Phantom_Image_Large;
  mitk::Image::Pointer m_IBSI_Phantom_Mask_Small;
  mitk::Image::Pointer m_IBSI_Phantom_Mask_Large;

public:

  void setUp(void) override
  {
    m_IBSI_Phantom_Image_Small = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Small.nrrd"));
    m_IBSI_Phantom_Image_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Large.nrrd"));
    m_IBSI_Phantom_Mask_Small = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Small.nrrd"));
    m_IBSI_Phantom_Mask_Large = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Radiomics/IBSI_Phantom_Mask_Large.nrrd"));
  }

  void ImageDescription_PhantomTest_3D()
  {
    mitk::GIFCooccurenceMatrix2::Pointer featureCalculator = mitk::GIFCooccurenceMatrix2::New();

    featureCalculator->SetUseBinsize(true);
    featureCalculator->SetBinsize(1.0);
    featureCalculator->SetUseMinimumIntensity(true);
    featureCalculator->SetUseMaximumIntensity(true);
    featureCalculator->SetMinimumIntensity(0.5);
    featureCalculator->SetMaximumIntensity(6.5);

    auto featureList = featureCalculator->CalculateFeatures(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 18 features.", std::size_t(93), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.509, results["Co-occurenced Based Features::Overall Joint Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 2.149, results["Co-occurenced Based Features::Overall Joint Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 3.132, results["Co-occurenced Based Features::Overall Joint Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 2.574, results["Co-occurenced Based Features::Overall Joint Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 1.379, results["Co-occurenced Based Features::Overall Diiference Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 3.215, results["Co-occurenced Based Features::Overall Difference Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 1.641, results["Co-occurenced Based Features::Overall Difference Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 4.298, results["Co-occurenced Based Features::Overall Sum Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 7.412, results["Co-occurenced Based Features::Overall Sum Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 2.110, results["Co-occurenced Based Features::Overall Sum Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.291, results["Co-occurenced Based Features::Overall Angular Second Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 5.118, results["Co-occurenced Based Features::Overall Contrast"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 1.380, results["Co-occurenced Based Features::Overall Dissimilarity"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.688, results["Co-occurenced Based Features::Overall Inverse Difference"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.856, results["Co-occurenced Based Features::Overall Inverse Difference Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.631, results["Co-occurenced Based Features::Overall Inverse Difference Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.902, results["Co-occurenced Based Features::Overall Inverse Difference Moment Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.057, results["Co-occurenced Based Features::Overall Inverse Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.183, results["Co-occurenced Based Features::Overall Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 5.192, results["Co-occurenced Based Features::Overall Autocorrelation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 7.412, results["Co-occurenced Based Features::Overall Cluster Tendendcy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 17.419, results["Co-occurenced Based Features::Overall Cluster Shade"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 147.464, results["Co-occurenced Based Features::Overall Cluster Prominence"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", -0.0288, results["Co-occurenced Based Features::Overall First Measure of Information Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.269, results["Co-occurenced Based Features::Overall Second Measure of Information Correlation"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 0.679, results["Co-occurenced Based Features::Overall Row Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 2.149, results["Co-occurenced Based Features::Overall Row Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 3.132, results["Co-occurenced Based Features::Overall Row Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 1.306, results["Co-occurenced Based Features::Overall Row Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 2.611, results["Co-occurenced Based Features::Overall First Row-Column Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features:: with Large IBSI Phantom Image", 2.611, results["Co-occurenced Based Features::Overall Second Row-Column Entropy"], 0.001);
  }

  void ImageDescription_PhantomTest_2D()
  {
    mitk::GIFCooccurenceMatrix2::Pointer featureCalculator = mitk::GIFCooccurenceMatrix2::New();

    featureCalculator->SetUseBinsize(true);
    featureCalculator->SetBinsize(1.0);
    featureCalculator->SetUseMinimumIntensity(true);
    featureCalculator->SetUseMaximumIntensity(true);
    featureCalculator->SetMinimumIntensity(0.5);
    featureCalculator->SetMaximumIntensity(6.5);

    auto featureList = featureCalculator->CalculateFeaturesSlicewise(m_IBSI_Phantom_Image_Large, m_IBSI_Phantom_Mask_Large, 2);

    std::map<std::string, double> results;
    for (auto valuePair : featureList)
    {
      MITK_INFO << valuePair.first << " : " << valuePair.second;
      results[valuePair.first] = valuePair.second;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 108 features.", std::size_t(108), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Small Zone Emphasis with Large IBSI Phantom Image", 0.363, results["SliceWise Mean Grey Level Size Zone::Small Zone Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Large Zone Emphasis with Large IBSI Phantom Image", 43.9, results["SliceWise Mean Grey Level Size Zone::Large Zone Emphasis"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Low Grey Level Emphasis with Large IBSI Phantom Image", 0.371, results["SliceWise Mean Grey Level Size Zone::Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::High Grey Level Emphasis with Large IBSI Phantom Image", 16.4, results["SliceWise Mean Grey Level Size Zone::High Grey Level Emphasis"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Small Zone Low Grey Level Emphasis with Large IBSI Phantom Image", 0.0259, results["SliceWise Mean Grey Level Size Zone::Small Zone Low Grey Level Emphasis"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Small Zone High Grey Level Emphasis with Large IBSI Phantom Image", 10.3, results["SliceWise Mean Grey Level Size Zone::Small Zone High Grey Level Emphasis"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Large Zone Low Grey Level Emphasis with Large IBSI Phantom Image", 40.4, results["SliceWise Mean Grey Level Size Zone::Large Zone Low Grey Level Emphasis"], 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Large Zone High Grey Level Emphasis with Large IBSI Phantom Image", 113, results["SliceWise Mean Grey Level Size Zone::Large Zone High Grey Level Emphasis"], 1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Grey Level Non-Uniformity with Large IBSI Phantom Image", 1.41, results["SliceWise Mean Grey Level Size Zone::Grey Level Non-Uniformity"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Grey Level Non-Uniformity Normalized with Large IBSI Phantom Image", 0.323, results["SliceWise Mean Grey Level Size Zone::Grey Level Non-Uniformity Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Zone Size Non-Uniformity with Large IBSI Phantom Image", 1.49, results["SliceWise Mean Grey Level Size Zone::Zone Size Non-Uniformity"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Zone Size Non-Uniformity Normalized with Large IBSI Phantom Image", 0.333, results["SliceWise Mean Grey Level Size Zone::Zone Size Non-Uniformity Normalized"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Zone Percentage with Large IBSI Phantom Image", 0.24, results["SliceWise Mean Grey Level Size Zone::Zone Percentage"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Grey Level Variance with Large IBSI Phantom Image", 3.97, results["SliceWise Mean Grey Level Size Zone::Grey Level Variance"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Zone Size Variance with Large IBSI Phantom Image", 21, results["SliceWise Mean Grey Level Size Zone::Zone Size Variance"], 0.1);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Zone Size Entropy with Large IBSI Phantom Image", 1.93, results["SliceWise Mean Grey Level Size Zone::Zone Size Entropy"], 0.01);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone:: with Large IBSI Phantom Image", 0.045, results["SliceWise Mean Grey Level Size Zone::"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Grey Level Mean with Large IBSI Phantom Image", 3.526, results["SliceWise Mean Grey Level Size Zone::Grey Level Mean"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Grey Level Size Zone::Zone Size Mean with Large IBSI Phantom Image", 4.59524, results["SliceWise Mean Grey Level Size Zone::Zone Size Mean"], 0.001);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFCooc2 )