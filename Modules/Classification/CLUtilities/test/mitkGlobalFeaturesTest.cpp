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
#include <mitkGIFFirstOrderStatistics.h>
#include <mitkGIFCooccurenceMatrix.h>
#include <mitkGIFGrayLevelRunLength.h>
#include <math.h>

class mitkGlobalFeaturesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGlobalFeaturesTestSuite  );

  MITK_TEST(FirstOrder_SinglePoint);
  MITK_TEST(FirstOrder_QubicArea);

  //MITK_TEST(TestFirstOrderStatistic);
  //  MITK_TEST(TestThreadedDecisionForest);

  CPPUNIT_TEST_SUITE_END();

private:

  typedef itk::Image<double,3> ImageType;
  typedef itk::Image<unsigned char,3> MaskType;

  mitk::Image::Pointer m_Image,m_Mask,m_Mask1;
  ImageType::Pointer m_ItkImage;
  MaskType::Pointer m_ItkMask,m_ItkMask1;

public:

  void setUp(void)
  {
    // Load Image Data
    m_Image = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
    mitk::CastToItkImage(m_Image,m_ItkImage);

    // Create a single mask with only one pixel within the regions
    mitk::Image::Pointer mask1 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
    mitk::CastToItkImage(mask1,m_ItkMask);
    m_ItkMask->FillBuffer(0);
    MaskType::IndexType index;
    index[0]=88;index[1]=81;index[2]=13;
    m_ItkMask->SetPixel(index, 1);
    MITK_INFO << "Pixel Value: "<<m_ItkImage->GetPixel(index);
    mitk::CastToMitkImage(m_ItkMask, m_Mask);

    // Create a mask with a covered region
    mitk::Image::Pointer lmask1 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
    mitk::CastToItkImage(lmask1,m_ItkMask1);
    m_ItkMask1->FillBuffer(0);
    int range=2;
    for (int x = 88-range;x < 88+range+1;++x)
    {
      for (int y=81-range;y<81+range+1;++y)
      {
        for (int z=13-range;z<13+range+1;++z)
        {
          index[0] = x;
          index[1] = y;
          index[2] = z;
          //MITK_INFO << "Pixel: " <<m_ItkImage->GetPixel(index);
          m_ItkMask1->SetPixel(index, 1);
        }
      }
    }
    mitk::CastToMitkImage(m_ItkMask1, m_Mask1);
  }

  void FirstOrder_SinglePoint()
  {
    mitk::GIFFirstOrderStatistics::Pointer calculator = mitk::GIFFirstOrderStatistics::New();
    calculator->SetHistogramSize(4096);
    calculator->SetUseCtRange(true);
    auto features = calculator->CalculateFeatures(m_Image, m_Mask);

    std::map<std::string, double> results;
    for (auto iter=features.begin(); iter!=features.end();++iter)
    {
      results[(*iter).first]=(*iter).second;
      MITK_INFO << (*iter).first << " : " << (*iter).second;
    }

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The range of a single pixel should  be 0",0.0, results["FirstOrder Range"], 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The uniformity of a single pixel should  be 1",1.0, results["FirstOrder Uniformity"], 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The entropy of a single pixel should  be 0",0.0, results["FirstOrder Entropy"], 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Root-Means-Square of a single pixel with (-352) should  be 352",352.0, results["FirstOrder RMS"], 0.01);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The Kurtosis of a single pixel should be undefined",results["FirstOrder Kurtosis"]==results["FirstOrder Kurtosis"], false);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The Skewness of a single pixel should be undefined",results["FirstOrder Skewness"]==results["FirstOrder Skewness"], false);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean absolute deviation of a single pixel with (-352) should  be 0",0, results["FirstOrder Mean absolute deviation"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Covered image intensity range of a single pixel with (-352) should be 0",0, results["FirstOrder Covered Image Intensity Range"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Minimum of a single pixel with (-352) should be -352",-352, results["FirstOrder Minimum"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Maximum of a single pixel with (-352) should be -352",-352, results["FirstOrder Maximum"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean of a single pixel with (-352) should be -352",-352, results["FirstOrder Mean"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Variance (corrected) of a single pixel with (-352) should be 0",0, results["FirstOrder Variance"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Sum of a single pixel with (-352) should be -352",-352, results["FirstOrder Sum"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Median of a single pixel with (-352) should be -352",-352, results["FirstOrder Median"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Standard deviation (corrected) of a single pixel with (-352) should be -352",0, results["FirstOrder Standard deviation"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The number of voxels of a single pixel should be 1",1, results["FirstOrder No. of Voxel"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Energy of a single pixel should be 352*352",352*352, results["FirstOrder Energy"], 0.0);
    //  MITK_ASSERT_EQUAL(results["FirstOrder Range"]==0.0,true,"The range of a single pixel should  be 0");
  }

  void FirstOrder_QubicArea()
  {
    mitk::GIFFirstOrderStatistics::Pointer calculator = mitk::GIFFirstOrderStatistics::New();
    calculator->SetHistogramSize(4096);
    calculator->SetUseCtRange(true);
    auto features = calculator->CalculateFeatures(m_Image, m_Mask1);

    std::map<std::string, double> results;
    for (auto iter=features.begin(); iter!=features.end();++iter)
    {
      results[(*iter).first]=(*iter).second;
      MITK_INFO << (*iter).first << " : " << (*iter).second;
    }

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The range should be 981",981, results["FirstOrder Range"], 0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Root-Means-Square of a single pixel with (-352) should  be 352",402.895778, results["FirstOrder RMS"], 0.01);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Minimum of a single pixel with (-352) should be -352",-937, results["FirstOrder Minimum"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Maximum of a single pixel with (-352) should be -352",44, results["FirstOrder Maximum"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean of a single pixel with (-352) should be -352",-304.448, results["FirstOrder Mean"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Sum of a single pixel with (-352) should be -352",-38056, results["FirstOrder Sum"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Median of a single pixel with (-352) should be -352",-202, results["FirstOrder Median"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The number of voxels of a single pixel should be 1",125, results["FirstOrder No. of Voxel"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Standard deviation (corrected) of a single pixel with (-352) should be -352",264.949066, results["FirstOrder Standard deviation"], 0.000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Energy of a single pixel should be 352*352",20290626, results["FirstOrder Energy"], 0.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The uniformity of a single pixel should  be 1",0.0088960, results["FirstOrder Uniformity"], 0.0000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The entropy of a single pixel should  be 0",-6.853784285, results["FirstOrder Entropy"], 0.000000005);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Variance (corrected) of a single pixel with (-352) should be 0",70198.0074, results["FirstOrder Variance"], 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Kurtosis of a single pixel should  be 0",2.63480121, results["FirstOrder Kurtosis"], 0.0001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Skewness of a single pixel should  be 0",-0.91817318, results["FirstOrder Skewness"], 0.00001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Mean absolute deviation of a single pixel with (-352) should  be 0",219.348608, results["FirstOrder Mean absolute deviation"], 0.000001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The Covered image intensity range of a single pixel with (-352) should be 0",0.41149329, results["FirstOrder Covered Image Intensity Range"], 0.000001);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkGlobalFeatures)