/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include "itkMaskedNaryStatisticsImageFilter.h"

#include "mitkTestingMacros.h"
#include "mitkVector.h"

#include "mitkTestDynamicImageGenerator.h"

int itkMaskedNaryStatisticsImageFilterTest(int  /*argc*/ , char*[] /*argv[]*/)
{
  // always start with this!
  MITK_TEST_BEGIN("itkMaskedNaryStatisticsImageFilterTest")

    //Prepare test artifacts and helper

  mitk::TestImageType::Pointer img1 = mitk::GenerateTestImage();
  mitk::TestImageType::Pointer img2 = mitk::GenerateTestImage(10);

  typedef itk::MaskedNaryStatisticsImageFilter<mitk::TestImageType> FilterType;
  FilterType::Pointer testFilter = FilterType::New();

  testFilter->SetInput(0,img1);
  testFilter->SetInput(1,img2);

  testFilter->Update();

  FilterType::PixelVectorType max = testFilter->GetMaximum();
  FilterType::PixelVectorType min = testFilter->GetMinimum();
  FilterType::RealVectorType mean = testFilter->GetMean();
  FilterType::RealVectorType sig = testFilter->GetSigma();
  FilterType::RealVectorType variance = testFilter->GetVariance();
  FilterType::RealVectorType sum = testFilter->GetSum();

  MITK_TEST_CONDITION(2 == max.size(),"Check size of maximum");
  MITK_TEST_CONDITION(2 == min.size(),"Check size of minimum");
  MITK_TEST_CONDITION(2 == mean.size(),"Check size of mean");
  MITK_TEST_CONDITION(2 == sig.size(),"Check size of sigma");
  MITK_TEST_CONDITION(2 == variance.size(),"Check size of variance");
  MITK_TEST_CONDITION(2 == sum.size(),"Check size of sum");

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum[0]",9 == max[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum[0]",1 == min[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean[0]",5 == mean[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma[0]",sqrt(7.5) == sig[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance[0]",7.5 == variance[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum[0]",45 == sum[0]);

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum[1]",90 == max[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum[1]",10 == min[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean[1]",50 == mean[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma[1]",sqrt(750.0) == sig[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance[1]",750 == variance[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum[1]",450 == sum[1]);


  //Test with mask set
  mitk::TestMaskType::Pointer mask = mitk::GenerateTestMask();
  testFilter->SetMask(mask);

  testFilter->Update();

  max = testFilter->GetMaximum();
  min = testFilter->GetMinimum();
  mean = testFilter->GetMean();
  sig = testFilter->GetSigma();
  variance = testFilter->GetVariance();
  sum = testFilter->GetSum();

  MITK_TEST_CONDITION(2 == max.size(),"Check size of maximum");
  MITK_TEST_CONDITION(2 == min.size(),"Check size of minimum");
  MITK_TEST_CONDITION(2 == mean.size(),"Check size of mean");
  MITK_TEST_CONDITION(2 == sig.size(),"Check size of sigma");
  MITK_TEST_CONDITION(2 == variance.size(),"Check size of variance");
  MITK_TEST_CONDITION(2 == sum.size(),"Check size of sum");

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum[0]",4 == max[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum[0]",2 == min[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean[0]",3 == mean[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma[0]",1 == sig[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance[0]",1 == variance[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum[0]",9 == sum[0]);

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum[1]",40 == max[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum[1]",20 == min[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean[1]",30 == mean[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma[1]",10 == sig[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance[1]",100 == variance[1]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum[1]",90 == sum[1]);

  MITK_TEST_END()
}
