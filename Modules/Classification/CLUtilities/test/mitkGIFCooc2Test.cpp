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
  MITK_TEST(ImageDescription_PhantomTest_2D);

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

    // These values are obtained with IBSI (3D Comb)
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Joint Maximum with Large IBSI Phantom Image", 0.509, results["Co-occurenced Based Features::Overall Joint Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Joint Average with Large IBSI Phantom Image", 2.149, results["Co-occurenced Based Features::Overall Joint Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Joint Variance with Large IBSI Phantom Image", 3.132, results["Co-occurenced Based Features::Overall Joint Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Joint Entropy with Large IBSI Phantom Image", 2.574, results["Co-occurenced Based Features::Overall Joint Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Difference Average with Large IBSI Phantom Image", 1.379, results["Co-occurenced Based Features::Overall Difference Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Difference Variance with Large IBSI Phantom Image", 3.215, results["Co-occurenced Based Features::Overall Difference Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Difference Entropy with Large IBSI Phantom Image", 1.641, results["Co-occurenced Based Features::Overall Difference Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Sum Average with Large IBSI Phantom Image", 4.298, results["Co-occurenced Based Features::Overall Sum Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Sum Variance with Large IBSI Phantom Image", 7.412, results["Co-occurenced Based Features::Overall Sum Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Sum Entropy with Large IBSI Phantom Image", 2.110, results["Co-occurenced Based Features::Overall Sum Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Angular Second Moment with Large IBSI Phantom Image", 0.291, results["Co-occurenced Based Features::Overall Angular Second Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Contrast with Large IBSI Phantom Image", 5.118, results["Co-occurenced Based Features::Overall Contrast"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Dissimilarity with Large IBSI Phantom Image", 1.380, results["Co-occurenced Based Features::Overall Dissimilarity"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Inverse Difference with Large IBSI Phantom Image", 0.688, results["Co-occurenced Based Features::Overall Inverse Difference"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Inverse Difference Normalized with Large IBSI Phantom Image", 0.856, results["Co-occurenced Based Features::Overall Inverse Difference Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Inverse Difference Moment with Large IBSI Phantom Image", 0.631, results["Co-occurenced Based Features::Overall Inverse Difference Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Inverse Difference Moment Normalized with Large IBSI Phantom Image", 0.902, results["Co-occurenced Based Features::Overall Inverse Difference Moment Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Inverse Variance with Large IBSI Phantom Image", 0.057, results["Co-occurenced Based Features::Overall Inverse Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Correlation with Large IBSI Phantom Image", 0.183, results["Co-occurenced Based Features::Overall Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Autocorrelation with Large IBSI Phantom Image", 5.192, results["Co-occurenced Based Features::Overall Autocorrelation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Cluster Tendency with Large IBSI Phantom Image", 7.412, results["Co-occurenced Based Features::Overall Cluster Tendency"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Cluster Shade with Large IBSI Phantom Image", 17.419, results["Co-occurenced Based Features::Overall Cluster Shade"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Cluster Prominence with Large IBSI Phantom Image", 147.464, results["Co-occurenced Based Features::Overall Cluster Prominence"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall First Measure of Information Correlation with Large IBSI Phantom Image", -0.0288, results["Co-occurenced Based Features::Overall First Measure of Information Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Second Measure of Information Correlation with Large IBSI Phantom Image", 0.269, results["Co-occurenced Based Features::Overall Second Measure of Information Correlation"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Row Maximum with Large IBSI Phantom Image", 0.679, results["Co-occurenced Based Features::Overall Row Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Row Average with Large IBSI Phantom Image", 2.149, results["Co-occurenced Based Features::Overall Row Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Row Variance with Large IBSI Phantom Image", 3.132, results["Co-occurenced Based Features::Overall Row Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Row Entropy with Large IBSI Phantom Image", 1.306, results["Co-occurenced Based Features::Overall Row Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall First Row-Column Entropy with Large IBSI Phantom Image", 2.611, results["Co-occurenced Based Features::Overall First Row-Column Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Overall Second Row-Column Entropy with Large IBSI Phantom Image", 2.611, results["Co-occurenced Based Features::Overall Second Row-Column Entropy"], 0.001);




    // These values are obtained with IBSI (3D Avg)
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Joint Maximum with Large IBSI Phantom Image", 0.503, results["Co-occurenced Based Features::Mean Joint Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Joint Average with Large IBSI Phantom Image", 2.143, results["Co-occurenced Based Features::Mean Joint Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Joint Variance with Large IBSI Phantom Image", 3.099, results["Co-occurenced Based Features::Mean Joint Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Joint Entropy with Large IBSI Phantom Image", 2.399, results["Co-occurenced Based Features::Mean Joint Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Difference Average with Large IBSI Phantom Image", 1.431, results["Co-occurenced Based Features::Mean Difference Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Difference Variance with Large IBSI Phantom Image", 3.056, results["Co-occurenced Based Features::Mean Difference Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Difference Entropy with Large IBSI Phantom Image", 1.563, results["Co-occurenced Based Features::Mean Difference Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Sum Average with Large IBSI Phantom Image", 4.2859, results["Co-occurenced Based Features::Mean Sum Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Sum Variance with Large IBSI Phantom Image", 7.072, results["Co-occurenced Based Features::Mean Sum Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Sum Entropy with Large IBSI Phantom Image", 1.9226, results["Co-occurenced Based Features::Mean Sum Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Angular Second Moment with Large IBSI Phantom Image", 0.303, results["Co-occurenced Based Features::Mean Angular Second Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Contrast with Large IBSI Phantom Image", 5.3245, results["Co-occurenced Based Features::Mean Contrast"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Dissimilarity with Large IBSI Phantom Image", 1.431, results["Co-occurenced Based Features::Mean Dissimilarity"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Inverse Difference with Large IBSI Phantom Image", 0.6766, results["Co-occurenced Based Features::Mean Inverse Difference"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Inverse Difference Normalized with Large IBSI Phantom Image", 0.8506, results["Co-occurenced Based Features::Mean Inverse Difference Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Inverse Difference Moment with Large IBSI Phantom Image", 0.6177, results["Co-occurenced Based Features::Mean Inverse Difference Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Inverse Difference Moment Normalized with Large IBSI Phantom Image", 0.8984, results["Co-occurenced Based Features::Mean Inverse Difference Moment Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Inverse Variance with Large IBSI Phantom Image", 0.0604, results["Co-occurenced Based Features::Mean Inverse Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Correlation with Large IBSI Phantom Image", 0.157, results["Co-occurenced Based Features::Mean Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Autocorrelation with Large IBSI Phantom Image", 5.05544, results["Co-occurenced Based Features::Mean Autocorrelation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Cluster Tendency with Large IBSI Phantom Image", 7.0728, results["Co-occurenced Based Features::Mean Cluster Tendency"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Cluster Shade with Large IBSI Phantom Image", 16.6441, results["Co-occurenced Based Features::Mean Cluster Shade"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Cluster Prominence with Large IBSI Phantom Image", 144.703, results["Co-occurenced Based Features::Mean Cluster Prominence"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean First Measure of Information Correlation with Large IBSI Phantom Image", -0.15684, results["Co-occurenced Based Features::Mean First Measure of Information Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Second Measure of Information Correlation with Large IBSI Phantom Image", 0.519588, results["Co-occurenced Based Features::Mean Second Measure of Information Correlation"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Row Maximum with Large IBSI Phantom Image", 0.6808, results["Co-occurenced Based Features::Mean Row Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Row Average with Large IBSI Phantom Image", 2.143, results["Co-occurenced Based Features::Mean Row Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Row Variance with Large IBSI Phantom Image", 3.0993, results["Co-occurenced Based Features::Mean Row Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Row Entropy with Large IBSI Phantom Image", 1.29699, results["Co-occurenced Based Features::Mean Row Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean First Row-Column Entropy with Large IBSI Phantom Image", 2.5939, results["Co-occurenced Based Features::Mean First Row-Column Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Co-occurenced Based Features::Mean Second Row-Column Entropy with Large IBSI Phantom Image", 2.5939, results["Co-occurenced Based Features::Mean Second Row-Column Entropy"], 0.001);
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
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Image Diagnostics should calculate 558 features.", std::size_t(558), featureList.size());

    // These values are obtained with IBSI
    // Standard accuracy is 0.01
    // These values are obtained with IBSI (2D Comb)
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Joint Maximum with Large IBSI Phantom Image", 0.512, results["SliceWise Mean Co-occurenced Based Features::Overall Joint Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Joint Average with Large IBSI Phantom Image", 2.143, results["SliceWise Mean Co-occurenced Based Features::Overall Joint Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Joint Variance with Large IBSI Phantom Image", 2.7115, results["SliceWise Mean Co-occurenced Based Features::Overall Joint Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Joint Entropy with Large IBSI Phantom Image", 2.2383, results["SliceWise Mean Co-occurenced Based Features::Overall Joint Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Difference Average with Large IBSI Phantom Image", 1.3990, results["SliceWise Mean Co-occurenced Based Features::Overall Difference Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Difference Variance with Large IBSI Phantom Image", 3.06426, results["SliceWise Mean Co-occurenced Based Features::Overall Difference Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Difference Entropy with Large IBSI Phantom Image", 1.49262, results["SliceWise Mean Co-occurenced Based Features::Overall Difference Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Sum Average with Large IBSI Phantom Image", 4.28686, results["SliceWise Mean Co-occurenced Based Features::Overall Sum Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Sum Variance with Large IBSI Phantom Image", 5.65615, results["SliceWise Mean Co-occurenced Based Features::Overall Sum Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Sum Entropy with Large IBSI Phantom Image", 1.79494, results["SliceWise Mean Co-occurenced Based Features::Overall Sum Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Angular Second Moment with Large IBSI Phantom Image", 0.351678, results["SliceWise Mean Co-occurenced Based Features::Overall Angular Second Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Contrast with Large IBSI Phantom Image", 5.19019, results["SliceWise Mean Co-occurenced Based Features::Overall Contrast"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Dissimilarity with Large IBSI Phantom Image", 1.3990, results["SliceWise Mean Co-occurenced Based Features::Overall Dissimilarity"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference with Large IBSI Phantom Image", 0.683294, results["SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference Normalized with Large IBSI Phantom Image", 0.8538, results["SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference Moment with Large IBSI Phantom Image", 0.625003, results["SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference Moment Normalized with Large IBSI Phantom Image", 0.90088, results["SliceWise Mean Co-occurenced Based Features::Overall Inverse Difference Moment Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Inverse Variance with Large IBSI Phantom Image", 0.055286, results["SliceWise Mean Co-occurenced Based Features::Overall Inverse Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Correlation with Large IBSI Phantom Image", 0.0173072, results["SliceWise Mean Co-occurenced Based Features::Overall Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Autocorrelation with Large IBSI Phantom Image", 5.13953, results["SliceWise Mean Co-occurenced Based Features::Overall Autocorrelation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Cluster Tendency with Large IBSI Phantom Image", 5.6561, results["SliceWise Mean Co-occurenced Based Features::Overall Cluster Tendency"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Cluster Shade with Large IBSI Phantom Image", 6.97661, results["SliceWise Mean Co-occurenced Based Features::Overall Cluster Shade"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Cluster Prominence with Large IBSI Phantom Image", 80.3855, results["SliceWise Mean Co-occurenced Based Features::Overall Cluster Prominence"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall First Measure of Information Correlation with Large IBSI Phantom Image", -0.0340891, results["SliceWise Mean Co-occurenced Based Features::Overall First Measure of Information Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Second Measure of Information Correlation with Large IBSI Phantom Image", 0.2625, results["SliceWise Mean Co-occurenced Based Features::Overall Second Measure of Information Correlation"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Row Maximum with Large IBSI Phantom Image", 0.682689, results["SliceWise Mean Co-occurenced Based Features::Overall Row Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Row Average with Large IBSI Phantom Image", 2.14343, results["SliceWise Mean Co-occurenced Based Features::Overall Row Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Row Variance with Large IBSI Phantom Image", 2.71158, results["SliceWise Mean Co-occurenced Based Features::Overall Row Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Row Entropy with Large IBSI Phantom Image", 1.13773, results["SliceWise Mean Co-occurenced Based Features::Overall Row Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall First Row-Column Entropy with Large IBSI Phantom Image", 2.27545, results["SliceWise Mean Co-occurenced Based Features::Overall First Row-Column Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Overall Second Row-Column Entropy with Large IBSI Phantom Image", 2.27545, results["SliceWise Mean Co-occurenced Based Features::Overall Second Row-Column Entropy"], 0.001);




    // These values are obtained with IBSI (2D Avg)
    // Standard accuracy is 0.01
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Joint Maximum with Large IBSI Phantom Image", 0.5188, results["SliceWise Mean Co-occurenced Based Features::Mean Joint Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Joint Average with Large IBSI Phantom Image", 2.14242, results["SliceWise Mean Co-occurenced Based Features::Mean Joint Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Joint Variance with Large IBSI Phantom Image", 2.6877, results["SliceWise Mean Co-occurenced Based Features::Mean Joint Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Joint Entropy with Large IBSI Phantom Image", 2.04966, results["SliceWise Mean Co-occurenced Based Features::Mean Joint Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Difference Average with Large IBSI Phantom Image", 1.42247, results["SliceWise Mean Co-occurenced Based Features::Mean Difference Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Difference Variance with Large IBSI Phantom Image", 2.90159, results["SliceWise Mean Co-occurenced Based Features::Mean Difference Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Difference Entropy with Large IBSI Phantom Image", 1.39615, results["SliceWise Mean Co-occurenced Based Features::Mean Difference Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Sum Average with Large IBSI Phantom Image", 4.28484, results["SliceWise Mean Co-occurenced Based Features::Mean Sum Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Sum Variance with Large IBSI Phantom Image", 5.47293, results["SliceWise Mean Co-occurenced Based Features::Mean Sum Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Sum Entropy with Large IBSI Phantom Image", 1.60319, results["SliceWise Mean Co-occurenced Based Features::Mean Sum Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Angular Second Moment with Large IBSI Phantom Image", 0.367529, results["SliceWise Mean Co-occurenced Based Features::Mean Angular Second Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Contrast with Large IBSI Phantom Image", 5.27785, results["SliceWise Mean Co-occurenced Based Features::Mean Contrast"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Dissimilarity with Large IBSI Phantom Image", 1.42247, results["SliceWise Mean Co-occurenced Based Features::Mean Dissimilarity"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference with Large IBSI Phantom Image", 0.677949, results["SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference Normalized with Large IBSI Phantom Image", 0.851399, results["SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference Moment with Large IBSI Phantom Image", 0.618737, results["SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference Moment"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference Moment Normalized with Large IBSI Phantom Image", 0.899219, results["SliceWise Mean Co-occurenced Based Features::Mean Inverse Difference Moment Normalized"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Inverse Variance with Large IBSI Phantom Image", 0.0566983, results["SliceWise Mean Co-occurenced Based Features::Mean Inverse Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Correlation with Large IBSI Phantom Image", -.012107, results["SliceWise Mean Co-occurenced Based Features::Mean Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Autocorrelation with Large IBSI Phantom Image", 5.09437, results["SliceWise Mean Co-occurenced Based Features::Mean Autocorrelation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Cluster Tendency with Large IBSI Phantom Image", 5.47293, results["SliceWise Mean Co-occurenced Based Features::Mean Cluster Tendency"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Cluster Shade with Large IBSI Phantom Image", 6.99782, results["SliceWise Mean Co-occurenced Based Features::Mean Cluster Shade"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Cluster Prominence with Large IBSI Phantom Image", 79.1126, results["SliceWise Mean Co-occurenced Based Features::Mean Cluster Prominence"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean First Measure of Information Correlation with Large IBSI Phantom Image", -0.15512, results["SliceWise Mean Co-occurenced Based Features::Mean First Measure of Information Correlation"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Second Measure of Information Correlation with Large IBSI Phantom Image", 0.487457, results["SliceWise Mean Co-occurenced Based Features::Mean Second Measure of Information Correlation"], 0.001);

    // These values are obtained by manually running the tool
    // Values might be wrong.
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Row Maximum with Large IBSI Phantom Image", 0.689717, results["SliceWise Mean Co-occurenced Based Features::Mean Row Maximum"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Row Average with Large IBSI Phantom Image", 2.14242, results["SliceWise Mean Co-occurenced Based Features::Mean Row Average"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Row Variance with Large IBSI Phantom Image", 2.6877, results["SliceWise Mean Co-occurenced Based Features::Mean Row Variance"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Row Entropy with Large IBSI Phantom Image", 1.1238, results["SliceWise Mean Co-occurenced Based Features::Mean Row Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean First Row-Column Entropy with Large IBSI Phantom Image", 2.24761, results["SliceWise Mean Co-occurenced Based Features::Mean First Row-Column Entropy"], 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("SliceWise Mean Co-occurenced Based Features::Mean Second Row-Column Entropy with Large IBSI Phantom Image", 2.24761, results["SliceWise Mean Co-occurenced Based Features::Mean Second Row-Column Entropy"], 0.001);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkGIFCooc2 )