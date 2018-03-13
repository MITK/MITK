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

#include <mitkImageCast.h>
#include <cmath>



class mitkGIFImageDescriptionFeaturesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGIFImageDescriptionFeaturesTestSuite );

  MITK_TEST(FirstOrder_SinglePoint);
  MITK_TEST(FirstOrder_QubicArea);
  MITK_TEST(Coocurrence_QubicArea);

  CPPUNIT_TEST_SUITE_END();

private:
	mitk::Image::Pointer m_IBSI_Phantom_Image_Small;
	mitk::Image::Pointer m_IBSI_Phantom_Image_Large;
	mitk::Image::Pointer m_IBSI_Phantom_Mask_Small;
	mitk::Image::Pointer m_IBSI_Phantom_Mask_Large;

public:

  void setUp(void) override
  {
	  m_IBSI_Phantom_Image_Small = mitk::IOUtil::LoadImage(GetTestDataFilePath("Radiomics/IBSI_Phantom_Image_Small.nrrd"));
  }

  void FirstOrder_SinglePoint()
  {

    //mitk::GIFFirstOrderStatistics::Pointer calculator = mitk::GIFFirstOrderStatistics::New();
    ////calculator->SetHistogramSize(4096);
    ////calculator->SetUseCtRange(true);
    //auto features = calculator->CalculateFeatures(m_Image, m_Mask);

    //std::map<std::string, double> results;
    //for (auto iter=features.begin(); iter!=features.end();++iter)
    //{
    //  results[(*iter).first]=(*iter).second;
    //  MITK_INFO << (*iter).first << " : " << (*iter).second;
    //}

    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The range of a single pixel should  be 0",0.0, results["FirstOrder Range"], 0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The uniformity of a single pixel should  be 1",1.0, results["FirstOrder Uniformity"], 0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The entropy of a single pixel should  be 0",0.0, results["FirstOrder Entropy"], 0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Root-Means-Square of a single pixel with (-352) should  be 352",352.0, results["FirstOrder RMS"], 0.01);
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("The Kurtosis of a single pixel should be undefined",results["FirstOrder Kurtosis"]==results["FirstOrder Kurtosis"], false);
    //CPPUNIT_ASSERT_EQUAL_MESSAGE("The Skewness of a single pixel should be undefined",results["FirstOrder Skewness"]==results["FirstOrder Skewness"], false);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean absolute deviation of a single pixel with (-352) should  be 0",0, results["FirstOrder Mean absolute deviation"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Covered image intensity range of a single pixel with (-352) should be 0",0, results["FirstOrder Covered Image Intensity Range"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Minimum of a single pixel with (-352) should be -352",-352, results["FirstOrder Minimum"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Maximum of a single pixel with (-352) should be -352",-352, results["FirstOrder Maximum"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean of a single pixel with (-352) should be -352",-352, results["FirstOrder Mean"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Variance (corrected) of a single pixel with (-352) should be 0",0, results["FirstOrder Variance"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Sum of a single pixel with (-352) should be -352",-352, results["FirstOrder Sum"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Median of a single pixel with (-352) should be -352",-352, results["FirstOrder Median"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Standard deviation (corrected) of a single pixel with (-352) should be -352",0, results["FirstOrder Standard deviation"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The number of voxels of a single pixel should be 1",1, results["FirstOrder No. of Voxel"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Energy of a single pixel should be 352*352",352*352, results["FirstOrder Energy"], 0.0);
    ////  MITK_ASSERT_EQUAL(results["FirstOrder Range"]==0.0,true,"The range of a single pixel should  be 0");
  }

  void FirstOrder_QubicArea()
  {
    //mitk::GIFFirstOrderStatistics::Pointer calculator = mitk::GIFFirstOrderStatistics::New();
    ////calculator->SetHistogramSize(4096);
    ////calculator->SetUseCtRange(true);
    //auto features = calculator->CalculateFeatures(m_Image, m_Mask1);

    //std::map<std::string, double> results;
    //for (auto iter=features.begin(); iter!=features.end();++iter)
    //{
    //  results[(*iter).first]=(*iter).second;
    //  MITK_INFO << (*iter).first << " : " << (*iter).second;
    //}

    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The range should be 981",981, results["FirstOrder Range"], 0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Root-Means-Square of a single pixel with (-352) should  be 352",402.895778, results["FirstOrder RMS"], 0.01);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Minimum of a single pixel with (-352) should be -352",-937, results["FirstOrder Minimum"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Maximum of a single pixel with (-352) should be -352",44, results["FirstOrder Maximum"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean of a single pixel with (-352) should be -352",-304.448, results["FirstOrder Mean"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Sum of a single pixel with (-352) should be -352",-38056, results["FirstOrder Sum"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Median of a single pixel with (-352) should be -352",-202, results["FirstOrder Median"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The number of voxels of a single pixel should be 1",125, results["FirstOrder No. of Voxel"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Standard deviation (corrected) of a single pixel with (-352) should be -352",264.949066, results["FirstOrder Standard deviation"], 0.000001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Energy of a single pixel should be 352*352",20290626, results["FirstOrder Energy"], 0.0);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The uniformity of a single pixel should  be 1",0.0088960, results["FirstOrder Uniformity"], 0.0000001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The entropy of a single pixel should  be 0",-6.853784285, results["FirstOrder Entropy"], 0.000000005);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Variance (corrected) of a single pixel with (-352) should be 0",70198.0074, results["FirstOrder Variance"], 0.0001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Kurtosis of a single pixel should  be 0",2.63480121, results["FirstOrder Kurtosis"], 0.0001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Skewness of a single pixel should  be 0",-0.91817318, results["FirstOrder Skewness"], 0.00001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean absolute deviation of a single pixel with (-352) should  be 0",219.348608, results["FirstOrder Mean absolute deviation"], 0.000001);
    //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Covered image intensity range of a single pixel with (-352) should be 0",0.41149329, results["FirstOrder Covered Image Intensity Range"], 0.000001);
  }

  void RunLenght_QubicArea()
  {
    //mitk::GIFGreyLevelRunLength::Pointer calculator = mitk::GIFGreyLevelRunLength::New();
    ////calculator->SetHistogramSize(4096);
    //auto features = calculator->CalculateFeatures(m_Image, m_Mask1);

    //std::map<std::string, double> results;
    //for (auto iter=features.begin(); iter!=features.end();++iter)
    //{
    //  results[(*iter).first]=(*iter).second;
    //  MITK_INFO << (*iter).first << " : " << (*iter).second;
    //}
  }

  void Coocurrence_QubicArea()
  {
  //  mitk::GIFCooccurenceMatrix::Pointer calculator = mitk::GIFCooccurenceMatrix::New();
  //  //calculator->SetHistogramSize(4096);
  //  //calculator->SetUseCtRange(true);
  //  //calculator->SetRange(981);
  //calculator->SetDirection(1);
  //  auto features = calculator->CalculateFeatures(m_GradientImage, m_GradientMask);

  //  std::map<std::string, double> results;
  //  for (auto iter=features.begin(); iter!=features.end();++iter)
  //  {
  //    results[(*iter).first]=(*iter).second;
  //    MITK_INFO << (*iter).first << " : " << (*iter).second;
  //  }
  //  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The mean energy value should be 0.2",0.2, results["co-occ. (1) Energy Means"], mitk::eps);
  //  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The mean entropy value should be 0.2",2.321928, results["co-occ. (1) Entropy Means"], 0.000001);
  //  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The mean contrast value should be 0.0",0, results["co-occ. (1) Contrast Means"], mitk::eps);
  //  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The mean dissimilarity value should be 0.0",0, results["co-occ. (1) Dissimilarity Means"], mitk::eps);
  //  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The mean homogenity1 value should be 1.0",1, results["co-occ. (1) Homogeneity1 Means"], mitk::eps);
  //  CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The mean InverseDifferenceMoment value should be 1.0",1, results["co-occ. (1) InverseDifferenceMoment Means"], mitk::eps);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkGIFImageDescriptionFeatures )