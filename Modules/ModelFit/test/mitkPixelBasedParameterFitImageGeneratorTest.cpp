/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>

#include "itkImageRegionIterator.h"

#include "mitkTestingMacros.h"
#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"

#include "mitkPixelBasedParameterFitImageGenerator.h"
#include "mitkLinearModelParameterizer.h"

#include "mitkLevenbergMarquardtModelFitFunctor.h"

#include "mitkTestDynamicImageGenerator.h"

int mitkPixelBasedParameterFitImageGeneratorTest(int  /*argc*/, char*[] /*argv[]*/)
{
  // always start with this!
  MITK_TEST_BEGIN("mitkPixelBasedParameterFitImageGenerator")

    //Prepare test artifacts and helper

  itk::Index<3> testIndex1;
  testIndex1[0] =   0;
  testIndex1[1] =   0;
  testIndex1[2] =   0;

  itk::Index<3> testIndex2;
  testIndex2[0] =   2;
  testIndex2[1] =   0;
  testIndex2[2] =   1;

  itk::Index<3> testIndex3;
  testIndex3[0] =   1;
  testIndex3[1] =   1;
  testIndex3[2] =   2;

  itk::Index<3> testIndex4;
  testIndex4[0] =   2;
  testIndex4[1] =   2;
  testIndex4[2] =   0;

  itk::Index<3> testIndex5;
  testIndex5[0] =   1;
  testIndex5[1] =   1;
  testIndex5[2] =   1;

  itk::Index<3> testIndex6;
  testIndex6[0] =   2;
  testIndex6[1] =   1;
  testIndex6[2] =   1;


    mitk::Image::Pointer dynamicImage = mitk::GenerateDynamicTestImageMITK();

    mitk::LinearModel::Pointer model = mitk::LinearModel::New();
    mitk::LevenbergMarquardtModelFitFunctor::Pointer testFunctor = mitk::LevenbergMarquardtModelFitFunctor::New();

    //Test default usage of filter
    mitk::PixelBasedParameterFitImageGenerator::Pointer generator = mitk::PixelBasedParameterFitImageGenerator::New();
    mitk::LinearModelParameterizer::Pointer parameterizer = mitk::LinearModelParameterizer::New();
    generator->SetDynamicImage(dynamicImage);
    generator->SetModelParameterizer(parameterizer);
    generator->SetFitFunctor(testFunctor);

    generator->Generate();

    mitk::PixelBasedParameterFitImageGenerator::ParameterImageMapType resultImages = generator->GetParameterImages();
    mitk::PixelBasedParameterFitImageGenerator::ParameterImageMapType derivedResultImages = generator->GetDerivedParameterImages();

    CPPUNIT_ASSERT_MESSAGE("Check number of parameter images", 2 == resultImages.size());
    MITK_TEST_CONDITION(resultImages.find("slope") != resultImages.end(),"Check if \"slope\" parameter image exists.");
    MITK_TEST_CONDITION(resultImages.find("offset") != resultImages.end(),"Check if \"offset\" parameter image exists.");
    CPPUNIT_ASSERT_MESSAGE("Check number of derived parameter images", 1 == derivedResultImages.size());
    MITK_TEST_CONDITION(derivedResultImages.find("x-intercept") != derivedResultImages.end(),"Check if \"x-intercept\" derived parameter image exists.");

    mitk::ImagePixelReadAccessor<mitk::ScalarType,3> slopeAccessor(resultImages["slope"]);
    mitk::ImagePixelReadAccessor<mitk::ScalarType,3> offsetAccessor(resultImages["offset"]);

    double testValue = slopeAccessor.GetPixelByIndex(testIndex1);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #1 (slope) at index #1");
    testValue = slopeAccessor.GetPixelByIndex(testIndex2);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(2000,testValue, 1e-4, true)==true, "Check param #1 (slope) at index #2");
    testValue = slopeAccessor.GetPixelByIndex(testIndex3);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(4000,testValue, 1e-4, true)==true, "Check param #1 (slope) at index #3");
    testValue = slopeAccessor.GetPixelByIndex(testIndex4);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(8000,testValue, 1e-4, true)==true, "Check param #1 (slope) at index #4");

    testValue = offsetAccessor.GetPixelByIndex(testIndex1);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #1");
    testValue = offsetAccessor.GetPixelByIndex(testIndex2);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(10,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #2");
    testValue = offsetAccessor.GetPixelByIndex(testIndex3);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(20,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #3");
    testValue = offsetAccessor.GetPixelByIndex(testIndex4);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #4");

    //Test with mask set
    mitk::Image::Pointer maskImage = mitk::GenerateTestMaskMITK();
    generator->SetMask(maskImage);

    generator->Generate();

    resultImages = generator->GetParameterImages();
    derivedResultImages = generator->GetDerivedParameterImages();

    CPPUNIT_ASSERT_MESSAGE("Check number of parameter images", 2 == resultImages.size());
    MITK_TEST_CONDITION(resultImages.find("slope") != resultImages.end(),"Check if \"slope\" parameter image exists.");
    MITK_TEST_CONDITION(resultImages.find("offset") != resultImages.end(),"Check if \"offset\" parameter image exists.");
    CPPUNIT_ASSERT_MESSAGE("Check number of derived parameter images", 1 == derivedResultImages.size());
    MITK_TEST_CONDITION(derivedResultImages.find("x-intercept") != derivedResultImages.end(),"Check if \"x-intercept\" derived parameter image exists.");

    mitk::ImagePixelReadAccessor<mitk::ScalarType,3> slopeAccessor2(resultImages["slope"]);
    mitk::ImagePixelReadAccessor<mitk::ScalarType,3> offsetAccessor2(resultImages["offset"]);

    testValue = slopeAccessor2.GetPixelByIndex(testIndex1);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #1 (slope) at index #1");
    testValue = slopeAccessor2.GetPixelByIndex(testIndex2);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(2000,testValue, 1e-4, true)==true, "Check param #1 (slope) at index #2");
    testValue = slopeAccessor2.GetPixelByIndex(testIndex3);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #1 (slope) at index #3");
    testValue = slopeAccessor2.GetPixelByIndex(testIndex4);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(8000,testValue, 1e-4, true)==true, "Check param #1 (slope) at index #4");
    testValue = slopeAccessor2.GetPixelByIndex(testIndex5);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(4000,testValue, 1e-4, true)==true, "Check param #1 (slope) at index #5");
    testValue = slopeAccessor2.GetPixelByIndex(testIndex6);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #1 (slope) at index #6");

    testValue = offsetAccessor2.GetPixelByIndex(testIndex1);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #1");
    testValue = offsetAccessor2.GetPixelByIndex(testIndex2);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(10,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #2");
    testValue = offsetAccessor2.GetPixelByIndex(testIndex3);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #3");
    testValue = offsetAccessor2.GetPixelByIndex(testIndex4);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #4");
    testValue = offsetAccessor2.GetPixelByIndex(testIndex5);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(10,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #5");
    testValue = offsetAccessor2.GetPixelByIndex(testIndex6);
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0,testValue, 1e-5, true)==true, "Check param #2 (offset) at index #6");

  MITK_TEST_END()
}
