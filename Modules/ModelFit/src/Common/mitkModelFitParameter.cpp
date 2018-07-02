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
    return NULL;
  }

  mitk::modelFit::Parameter::Pointer param = mitk::modelFit::Parameter::New();

  if (!data->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(), param->name))
  {
    return NULL;
  };

  param->image = dynamic_cast<const mitk::Image*>(data);

  if(!(param->image))
  {
    return NULL;
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

