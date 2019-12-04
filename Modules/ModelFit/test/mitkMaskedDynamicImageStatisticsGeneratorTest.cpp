/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>

#include "mitkTestingMacros.h"
#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"

#include "mitkMaskedDynamicImageStatisticsGenerator.h"

#include "mitkTestDynamicImageGenerator.h"

int mitkMaskedDynamicImageStatisticsGeneratorTest(int  /*argc*/, char*[] /*argv[]*/)
{
  // always start with this!
  MITK_TEST_BEGIN("mitkParameterFitImageGenerator")

  mitk::Image::Pointer dynamicImage = mitk::GenerateDynamicTestImageMITK();

  //Test default usage of filter
  mitk::MaskedDynamicImageStatisticsGenerator::Pointer generator = mitk::MaskedDynamicImageStatisticsGenerator::New();
  generator->SetDynamicImage(dynamicImage);
  generator->Generate();

  mitk::MaskedDynamicImageStatisticsGenerator::ResultType max = generator->GetMaximum();
  mitk::MaskedDynamicImageStatisticsGenerator::ResultType min = generator->GetMinimum();
  mitk::MaskedDynamicImageStatisticsGenerator::ResultType mean = generator->GetMean();
  mitk::MaskedDynamicImageStatisticsGenerator::ResultType sig = generator->GetSigma();
  mitk::MaskedDynamicImageStatisticsGenerator::ResultType variance = generator->GetVariance();
  mitk::MaskedDynamicImageStatisticsGenerator::ResultType sum = generator->GetSum();

  MITK_TEST_CONDITION(10 == max.size(),"Check size of maximum");
  MITK_TEST_CONDITION(10 == min.size(),"Check size of minimum");
  MITK_TEST_CONDITION(10 == mean.size(),"Check size of mean");
  MITK_TEST_CONDITION(10 == sig.size(),"Check size of sigma");
  MITK_TEST_CONDITION(10 == variance.size(),"Check size of variance");
  MITK_TEST_CONDITION(10 == sum.size(),"Check size of sum");

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum[0]", 28 == max[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum[0]", 0 == min[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean[0]", 14 == mean[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma[0]", 8.7266171082410953 == sig[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance[0]", 76.153846153846160 == variance[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum[0]", 378 == sum[0]);

  mitk::Image::Pointer maskImage = mitk::GenerateTestMaskMITK();
  generator->SetMask(maskImage);
  generator->Generate();

  max = generator->GetMaximum();
  min = generator->GetMinimum();
  mean = generator->GetMean();
  sig = generator->GetSigma();
  variance = generator->GetVariance();
  sum = generator->GetSum();

  CPPUNIT_ASSERT_MESSAGE("Check computed maximum[0]", 14 == max[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed minimum[0]", 0 == min[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed mean[0]",6.8571428571428568 == mean[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sigma[0]",4.6053003386088953 == sig[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed variance[0]",21.208791208791204 == variance[0]);
  CPPUNIT_ASSERT_MESSAGE("Check computed sum[0]",96.000000000000000 == sum[0]);

  MITK_TEST_END()
}
