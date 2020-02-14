/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include "itkMaskedStatisticsImageFilter.h"

#include "mitkTestingMacros.h"
#include "mitkVector.h"

#include "mitkTestDynamicImageGenerator.h"

int itkMaskedStatisticsImageFilterTest(int  /*argc*/, char*[] /*argv[]*/)
{
  // always start with this!
  MITK_TEST_BEGIN("itkMaskedStatisticsImageFilterTest")

    //Prepare test artifacts and helper

  mitk::TestImageType::Pointer img1 = mitk::GenerateTestImage();

  typedef itk::MaskedStatisticsImageFilter<mitk::TestImageType> FilterType;
  FilterType::Pointer testFilter = FilterType::New();

  testFilter->SetInput(img1);

  testFilter->SetNumberOfThreads(2);

  testFilter->Update();

  FilterType::PixelType max = testFilter->GetMaximum();
  FilterType::PixelType min = testFilter->GetMinimum();
  FilterType::RealType mean = testFilter->GetMean();
  FilterType::RealType sig = testFilter->GetSigma();
  FilterType::RealType variance = testFilter->GetVariance();
  FilterType::RealType sum = testFilter->GetSum();

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum",9 == max);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum",1 == min);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean",5 == mean);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma",sqrt(7.5) == sig);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance",7.5 == variance);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum",45 == sum);

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

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum",4 == max);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum",2 == min);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean",3 == mean);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma",1 == sig);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance",1 == variance);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum",9 == sum);

  MITK_TEST_END()
}
