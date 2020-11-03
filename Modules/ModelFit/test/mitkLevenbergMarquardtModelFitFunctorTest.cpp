/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <iostream>
#include "mitkTestingMacros.h"

#include "itkImage.h"
#include "itkImageRegionIterator.h"

#include "mitkLevenbergMarquardtModelFitFunctor.h"

#include "mitkLinearModel.h"

int mitkLevenbergMarquardtModelFitFunctorTest(int  /*argc*/, char*[] /*argv[]*/)
{
  // always start with this!
  MITK_TEST_BEGIN("LevenbergMarquardtModelFitFunctor")

  //Prepare test artifacts and helper

  mitk::ModelBase::TimeGridType grid(10);

  typedef std::vector<double> ValueArrayType;

  ValueArrayType sample1(10);
  ValueArrayType sample2(10);

  for (int i = 0; i < 10; ++i)
  {
    grid[i] = i;
    sample1[i] = 5 * i;
    sample2[i] = 2 * i + 10;
  }

  mitk::LinearModel::Pointer model = mitk::LinearModel::New();
  model->SetTimeGrid(grid);

  mitk::LinearModel::ParametersType initParams;
  initParams.SetSize(2);
  initParams.Fill(0.0);


  mitk::LevenbergMarquardtModelFitFunctor::Pointer testFunctor =
    mitk::LevenbergMarquardtModelFitFunctor::New();

  //Test functor for sample1

  MITK_TEST_FOR_EXCEPTION(::itk::ExceptionObject, testFunctor->GetNumberOfOutputs(nullptr));

  CPPUNIT_ASSERT_MESSAGE("Check number of outputs with model set.", 4 == testFunctor->GetNumberOfOutputs(model));

  ValueArrayType output = testFunctor->Compute(sample1, model, initParams);

  CPPUNIT_ASSERT_MESSAGE("Check number of values in functor output.", 4 == output.size());

  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(5, output[0], 1e-6, true) == true,
                               "Check fitted parameter 1 (slope) for sample 1.");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0, output[1], 1e-6, true) == true,
                               "Check fitted parameter 2 (offset) for sample 1.");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(0, output[2], 1e-6, true) == true,
                               "Check derived parameter 1 (x-intercept) for sample 1.");

  //Test functor for sample2
  output = testFunctor->Compute(sample2, model, initParams);

  CPPUNIT_ASSERT_MESSAGE("Check number of values in functor output.", 4 == output.size());

  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(2, output[0], 1e-6, true) == true,
                               "Check fitted parameter 1 (slope) for sample 2.");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(10, output[1], 1e-6, true) == true,
                               "Check fitted parameter 2 (offset) for sample 2.")
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(-5, output[2], 1e-6, true) == true,
                               "Check derived parameter 1 (x-intercept) for sample 2.");

  MITK_TEST_END()
}
