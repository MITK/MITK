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

#include <iostream>
#include "mitkTestingMacros.h"

#include "mitkTestModel.h"
#include "mitkTestModelFactory.h"
#include "mitkModelFitException.h"


int mitkConcreteModelFactoryBaseTest(int  /*argc*/, char*[] /*argv[]*/)
{
  MITK_TEST_BEGIN("mitkConcreteModelFactoryBaseTest")

  mitk::TestModelFactory::Pointer factory = mitk::TestModelFactory::New();
  mitk::TestModel::Pointer refModel = mitk::TestModel::New();

  //check pass through of model properties.

  MITK_TEST_CONDITION_REQUIRED(factory->GetParameterNames() == refModel->GetParameterNames(),
                               "Testing if GetParameterNames is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfParameters() == refModel->GetNumberOfParameters(),
                               "Testing if GetNumberOfParameters is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetParameterScales() == refModel->GetParameterScales(),
                               "Testing if GetParameterScales is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetParameterUnits() == refModel->GetParameterUnits(),
                               "Testing if GetParameterUnits is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetDerivedParameterNames() ==
                               refModel->GetDerivedParameterNames(),
                               "Testing if GetDerivedParameterNames is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetNumberOfDerivedParameters() ==
                               refModel->GetNumberOfDerivedParameters(),
                               "Testing if GetNumberOfDerivedParameters is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetDerivedParameterScales() ==
                               refModel->GetDerivedParameterScales(),
                               "Testing if GetDerivedParameterScales is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetDerivedParameterUnits() ==
                               refModel->GetDerivedParameterUnits(),
                               "Testing if GetDerivedParameterUnits is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetModelDisplayName() == refModel->GetModelDisplayName(),
                               "Testing if GetModelDisplayName is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetModelType() == refModel->GetModelType(),
                               "Testing if GetModelType is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetFunctionString() == refModel->GetFunctionString(),
                               "Testing if GetFunctionString is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetClassID() == refModel->GetClassID(),
                               "Testing if GetClassID is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetXName() == refModel->GetXName(),
                               "Testing if GetXName is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetXAxisName() == refModel->GetXAxisName(),
                               "Testing if GetXAxisName is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetXAxisUnit() == refModel->GetXAxisUnit(),
                               "Testing if GetXAxisUnit is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetYAxisName() == refModel->GetYAxisName(),
                               "Testing if GetYAxisName is successfully passed through.");
  MITK_TEST_CONDITION_REQUIRED(factory->GetYAxisUnit() == refModel->GetYAxisUnit(),
                               "Testing if GetYAxisUnit is successfully passed through.");

  MITK_TEST_END()
}
