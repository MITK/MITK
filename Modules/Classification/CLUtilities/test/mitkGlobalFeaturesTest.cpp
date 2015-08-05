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

class mitkGlobalFeaturesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGlobalFeaturesTestSuite  );

  MITK_TEST(TestFirstOrderStatistic);
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
    mitk::CastToMitkImage(m_ItkMask, m_Mask);

    // Create a mask with a covered region
    mitk::Image::Pointer lmask1 = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
    mitk::CastToItkImage(lmask1,m_ItkMask1);
    m_ItkMask1->FillBuffer(0);
    for (int x = 88-2;x < 88+3;++x)
    {
      for (int y=81-2;y<81+3;++y)
      {
        for (int z=13-2;z<13+3;++z)
        {
          index[0] = x;
          index[1] = y;
          index[2] = z;
          m_ItkMask1->SetPixel(index, 1);
        }
      }
    }
    mitk::CastToMitkImage(m_ItkMask1, m_Mask1);
  }

  void TestFirstOrderStatistic()
  {
    mitk::GIFFirstOrderStatistics::Pointer calculator = mitk::GIFFirstOrderStatistics::New();

    {
      auto features = calculator->CalculateFeatures(m_Image, m_Mask);

      for (auto iter=features.begin(); iter!=features.end();++iter)
      {
        MITK_INFO << (*iter).first << " : " << (*iter).second;
      }
    }
    {
      auto features = calculator->CalculateFeatures(m_Image, m_Mask1);

      for (auto iter=features.begin(); iter!=features.end();++iter)
      {
        MITK_INFO << (*iter).first << " : " << (*iter).second;
      }
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkGlobalFeatures)