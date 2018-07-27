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

#include <mitkImageAccessByItk.h>

#include "mitkDescriptivePharmacokineticBrixModel.h"
#include "mitkDescriptivePharmacokineticBrixModelParameterizer.h"

mitk::DescriptivePharmacokineticBrixModelParameterizer::StaticParameterMapType
mitk::DescriptivePharmacokineticBrixModelParameterizer::GetGlobalStaticParameters() const
{
  StaticParameterMapType result;
  StaticParameterValuesType values;
  values.push_back(m_Tau);
  result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_Tau, values));
  return result;
};

mitk::DescriptivePharmacokineticBrixModelParameterizer::StaticParameterMapType
mitk::DescriptivePharmacokineticBrixModelParameterizer::GetLocalStaticParameters(
  const IndexType& currentPosition) const
{
  StaticParameterMapType result;

  double s0 = m_BaseImage->GetPixel(currentPosition);

  StaticParameterValuesType values;
  values.push_back(s0);
  result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_S0, values));

  return result;
};

mitk::DescriptivePharmacokineticBrixModelParameterizer::ParametersType
mitk::DescriptivePharmacokineticBrixModelParameterizer::GetDefaultInitialParameterization() const
{
  ParametersType initialParameters;
  initialParameters.SetSize(4);
  initialParameters[mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_A] = 1.0;
  initialParameters[mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kep] = 4.0;
  initialParameters[mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_kel] = 0.2;
  initialParameters[mitk::DescriptivePharmacokineticBrixModel::POSITION_PARAMETER_tlag] = 1.0;

  return initialParameters;
}

mitk::DescriptivePharmacokineticBrixModelParameterizer::DescriptivePharmacokineticBrixModelParameterizer()
{
};

mitk::DescriptivePharmacokineticBrixModelParameterizer::~DescriptivePharmacokineticBrixModelParameterizer()
{
};
