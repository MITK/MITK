/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageTimeSelector.h>
#include <mitkImageCast.h>

#include "mitkDescriptivePharmacokineticBrixModelFactory.h"
#include "mitkDescriptivePharmacokineticBrixModelParameterizer.h"
#include "mitkDescriptivePharmacokineticBrixModelValueBasedParameterizer.h"

mitk::DescriptivePharmacokineticBrixModelFactory::DescriptivePharmacokineticBrixModelFactory()
{
};

mitk::DescriptivePharmacokineticBrixModelFactory::~DescriptivePharmacokineticBrixModelFactory()
{
};

mitk::ModelParameterizerBase::ParametersType
mitk::DescriptivePharmacokineticBrixModelFactory::GetDefaultInitialParameterization() const
{
  return DescriptivePharmacokineticBrixModelParameterizer::New()->GetDefaultInitialParameterization();
};

mitk::ModelParameterizerBase::Pointer
mitk::DescriptivePharmacokineticBrixModelFactory::DoCreateParameterizer(
  const mitk::modelFit::ModelFitInfo* fit) const
{
  mitk::ModelParameterizerBase::Pointer result;

  if (fit->fitType == ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED())
  {
    DescriptivePharmacokineticBrixModelParameterizer::Pointer modelParameterizer =
      DescriptivePharmacokineticBrixModelParameterizer::New();

    modelFit::StaticParameterMap::ValueType tau = fit->staticParamMap.Get(
          DescriptivePharmacokineticBrixModel::NAME_STATIC_PARAMETER_Tau);
    modelParameterizer->SetTau(tau[0]);

    mitk::ImageTimeSelector::Pointer imageTimeSelector =	mitk::ImageTimeSelector::New();
    imageTimeSelector->SetInput(fit->inputImage);
    imageTimeSelector->SetTimeNr(0);
    imageTimeSelector->UpdateLargestPossibleRegion();

    mitk::DescriptivePharmacokineticBrixModelParameterizer::BaseImageType::Pointer baseImage;
    mitk::CastToItkImage(imageTimeSelector->GetOutput(), baseImage);

    modelParameterizer->SetBaseImage(baseImage);

    result = modelParameterizer.GetPointer();
  }
  else if (fit->fitType == ModelFitConstants::FIT_TYPE_VALUE_ROIBASED())
  {
    DescriptivePharmacokineticBrixModelValueBasedParameterizer::Pointer modelParameterizer =
      DescriptivePharmacokineticBrixModelValueBasedParameterizer::New();

    modelFit::StaticParameterMap::ValueType tau = fit->staticParamMap.Get(
          DescriptivePharmacokineticBrixModel::NAME_STATIC_PARAMETER_Tau);
    modelParameterizer->SetTau(tau[0]);

    if (!fit->inputData.ValueExists("ROI"))
    {
      mitkThrow() <<
                  "Cannot generate parameterizer for fit of type ROIbased. Input data with the lable \"ROI\" is missing in fit.";
    }

    ScalarListLookupTable::ValueType signal = fit->inputData.GetTableValue("ROI");

    if (signal.empty())
    {
      mitkThrow() <<
                  "Cannot generate parameterizer for fit of type ROIbased. Input data with the lable \"ROI\" is invalid: No values available.";
    }

    modelParameterizer->SetBaseValue(signal[0]);

    result = modelParameterizer.GetPointer();
  }

  return result;
};
