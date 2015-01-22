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

// mitk includes
#include "mitkTestingMacros.h"
#include "mitkMultiComponentImageDataComparisonFilter.h"
#include "mitkImageReadAccessor.h"
#include "mitkIOUtil.h"

#include "itkNumericTraits.h"

int mitkMultiComponentImageDataComparisonFilterTest(int /*argc*/, char* argv[])
{
  MITK_TEST_BEGIN("MultiComponentImageDataComparisonFilter");

  // instantiation
  mitk::MultiComponentImageDataComparisonFilter::Pointer testObject = mitk::MultiComponentImageDataComparisonFilter::New();
  MITK_TEST_CONDITION_REQUIRED(testObject.IsNotNull(), "Testing instantiation of test class!");

  MITK_TEST_CONDITION_REQUIRED(testObject->GetCompareFilterResult() == NULL, "Testing initialization of result struct" );
  MITK_TEST_CONDITION_REQUIRED(testObject->GetTolerance() == 0.0f, "Testing initialization of tolerance member");
  MITK_TEST_CONDITION_REQUIRED(testObject->GetResult() == false, "Testing initialization of CompareResult member");

  // initialize compare result struct and pass it to the filter
  mitk::CompareFilterResults compareResult;
  compareResult.m_MaximumDifference = 0.0f;
  compareResult.m_MinimumDifference = itk::NumericTraits<double>::max();
  compareResult.m_MeanDifference = 0.0f;
  compareResult.m_FilterCompleted = false;
  compareResult.m_TotalDifference = 0.0f;
  compareResult.m_PixelsWithDifference = 0;
  testObject->SetCompareFilterResult(&compareResult);

  MITK_TEST_CONDITION_REQUIRED(testObject->GetCompareFilterResult() != NULL, "Testing set/get of compare result struct" );
  MITK_TEST_CONDITION_REQUIRED(testObject->GetResult() == false, "CompareResult still false" );

  //now load an image with several components and present it to the filter
  mitk::Image::Pointer testImg = mitk::IOUtil::LoadImage(argv[1]);
  mitk::Image::Pointer testImg2 = testImg->Clone();

  testObject->SetValidImage(testImg);
  testObject->SetTestImage(testImg2);

  MITK_TEST_CONDITION_REQUIRED(testObject->GetNumberOfIndexedInputs() == 2, "Testing correct handling of input images");

  testObject->Update();

  MITK_TEST_CONDITION_REQUIRED(testObject->GetResult(), "Testing filter processing with equal image data");

  // now change some of the data and check if the response is correct
  mitk::ImageReadAccessor imgAcc(testImg2);
  unsigned char* imgData = (unsigned char*) imgAcc.GetData();
  imgData[10] += 1;
  imgData[20] += 2;
  imgData[30] += 3;

  testObject->Update();

  MITK_TEST_CONDITION_REQUIRED(testObject->GetResult() == false, "Testing filter processing with unequal image data");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal((int)testObject->GetCompareFilterResult()->m_PixelsWithDifference, (int) 3) &&
    mitk::Equal((double)testObject->GetCompareFilterResult()->m_MaximumDifference, (double) 3.0) &&
    mitk::Equal((double)testObject->GetCompareFilterResult()->m_MeanDifference, (double) 2.0), "Assessing calculated image differences");
  MITK_TEST_END();
}
