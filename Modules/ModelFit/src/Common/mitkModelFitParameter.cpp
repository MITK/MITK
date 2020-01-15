/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkModelFitParameter.h"
#include "mitkModelFitConstants.h"

mitk::modelFit::Parameter::Parameter() :
  name (""), type(Parameter::ParameterType), unit(""), scale(1.0f)
{
}

mitk::modelFit::Parameter::Pointer mitk::modelFit::ExtractParameterFromData(const mitk::BaseData* data)
{
  if (!data)
  {
    return nullptr;
  }

  mitk::modelFit::Parameter::Pointer param = mitk::modelFit::Parameter::New();

  if (!data->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), param->name))
  {
    return nullptr;
  };

  param->image = dynamic_cast<const mitk::Image*>(data);

  if(!(param->image))
  {
    return nullptr;
  }

  std::string typeString;
  data->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), typeString);

  if(typeString == mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER())
  {
    param->type = Parameter::DerivedType;
  }
  else if(typeString == mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_CRITERION())
  {
    param->type = Parameter::CriterionType;
  }
  else if(typeString == mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_EVALUATION_PARAMETER())
  {
    param->type = Parameter::EvaluationType;
  }
  else
  {
    param->type = Parameter::ParameterType;
  }

  data->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(),
    param->unit);
  data->GetPropertyList()->GetFloatProperty(mitk::ModelFitConstants::PARAMETER_SCALE_PROPERTY_NAME().c_str(),
    param->scale);

  return param;
}

