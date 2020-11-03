/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNumericTwoCompartmentExchangeModelFactory.h"


#include "mitkNumericTwoCompartmentExchangeModelParameterizer.h"
#include "mitkAIFParametrizerHelper.h"

mitk::NumericTwoCompartmentExchangeModelFactory::NumericTwoCompartmentExchangeModelFactory()
{
};

mitk::NumericTwoCompartmentExchangeModelFactory::~NumericTwoCompartmentExchangeModelFactory()
{
};

mitk::ModelParameterizerBase::Pointer
mitk::NumericTwoCompartmentExchangeModelFactory::DoCreateParameterizer(
  const modelFit::ModelFitInfo* fit)
const
{
  mitk::ModelParameterizerBase::Pointer result;

  ModelParameterizerType::Pointer modelParameterizer =
    ModelParameterizerType::New();

  modelFit::StaticParameterMap::ValueType aif = fit->staticParamMap.Get(
        ModelType::NAME_STATIC_PARAMETER_AIF);

  modelParameterizer->SetAIF(mitk::convertParameterToArray(aif));

  modelFit::StaticParameterMap::ValueType aifGrid = fit->staticParamMap.Get(
        ModelType::NAME_STATIC_PARAMETER_AIFTimeGrid);
  modelParameterizer->SetAIFTimeGrid(mitk::convertParameterToArray(aifGrid));

  modelFit::StaticParameterMap::ValueType odeStepSize = fit->staticParamMap.Get(
        ModelType::NAME_STATIC_PARAMETER_ODEINTStepSize);
  modelParameterizer->SetODEINTStepSize(odeStepSize[0]);


  result = modelParameterizer.GetPointer();

  return result;
};
