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

#include "mitkModelFitConstants.h"


const std::string mitk::ModelFitConstants::MODEL_FIT_PROPERTY_NAME() { return "modelfit";}
const std::string mitk::ModelFitConstants::UID_PROPERTY_NAME() { return "data.uid";}
const std::string mitk::ModelFitConstants::INPUT_VARIABLES_PROPERTY_NAME() { return "modelfit.input.variables";}
const std::string mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME() { return "modelfit.parameter.name";}
const std::string mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME() { return "modelfit.parameter.type";}
const std::string mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER() { return "parameter";}
const std::string mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER() { return "derived";}
const std::string mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_CRITERION() { return "criterion";}
const std::string mitk::ModelFitConstants::PARAMETER_TYPE_VALUE_EVALUATION_PARAMETER() { return "evaluation";}
const std::string mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME() { return "modelfit.parameter.unit";}
const std::string mitk::ModelFitConstants::PARAMETER_SCALE_PROPERTY_NAME() { return "modelfit.parameter.scale";}
const std::string mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME() { return "modelfit.model.type";}
const std::string mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME() { return "modelfit.model.name";}
const std::string mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME() { return "modelfit.model.function";}
const std::string mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME() { return "modelfit.model.functionClass";}
const std::string mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME() { return "modelfit.model.x";}
const std::string mitk::ModelFitConstants::MODEL_X_VALUE_DEFAULT() { return "x";}
const std::string mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME() { return "modelfit.xaxis.name";}
const std::string mitk::ModelFitConstants::XAXIS_NAME_VALUE_DEFAULT() { return "Time";}
const std::string mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME() { return "modelfit.xaxis.unit";}
const std::string mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME() { return "modelfit.yaxis.name";}
const std::string mitk::ModelFitConstants::YAXIS_NAME_VALUE_DEFAULT() { return "Intensity";}
const std::string mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME() { return "modelfit.yaxis.unit";}
const std::string mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME() { return "modelfit.fit.uid"; }
const std::string mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME() { return "modelfit.fit.name"; }
const std::string mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME() { return "modelfit.fit.type";}
const std::string mitk::ModelFitConstants::FIT_TYPE_VALUE_PIXELBASED() { return "pixelbased";}
const std::string mitk::ModelFitConstants::FIT_TYPE_VALUE_ROIBASED() { return "ROIbased";}
const std::string mitk::ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME() { return "modelfit.fit.input.imageUID";}
const std::string mitk::ModelFitConstants::FIT_INPUT_ROIUID_PROPERTY_NAME() { return "modelfit.fit.input.roiUID";}
const std::string mitk::ModelFitConstants::FIT_INPUT_DATA_PROPERTY_NAME() { return "modelfit.fit.input.data";}
const std::string mitk::ModelFitConstants::FIT_STATIC_PARAMETERS_PROPERTY_NAME() { return "modelfit.fit.staticParameters";}