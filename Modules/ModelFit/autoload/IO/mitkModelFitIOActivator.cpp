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


#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkCoreServices.h>
#include <mitkIPropertyDescriptions.h>
#include <mitkIPropertyPersistence.h>
#include <mitkModelFitConstants.h>
#include <mitkScalarListLookupTablePropertySerializer.h>

namespace mitk
{

/*
 * This is the module activator for the IO aspects of the "ModelFit" module.
 */
class ModelFitIOActivator : public us::ModuleActivator
{
public:
  void registerProperty(const std::string& name, const std::string& key, const std::string& description)
  {
    mitk::CoreServices::GetPropertyDescriptions()->AddDescription(name, description);

    mitk::PropertyPersistenceInfo::Pointer ppi = mitk::PropertyPersistenceInfo::New();
    ppi->SetNameAndKey(name, key);

    mitk::CoreServices::GetPropertyPersistence()->AddInfo(ppi, true);
  }

  void registerProperty(const std::string& name, const std::string& key, const std::string& description, const PropertyPersistenceInfo::DeserializationFunctionType &deFnc, const PropertyPersistenceInfo::SerializationFunctionType &serFnc)
  {
    mitk::CoreServices::GetPropertyDescriptions()->AddDescription(name, description);

    mitk::PropertyPersistenceInfo::Pointer ppi = mitk::PropertyPersistenceInfo::New();
    ppi->SetNameAndKey(name, key);
    ppi->SetDeserializationFunction(deFnc);
    ppi->SetSerializationFunction(serFnc);

    mitk::CoreServices::GetPropertyPersistence()->AddInfo(ppi, true);
  }

  void Load(us::ModuleContext* /*context*/)
  {
    //register relevant properties
    registerProperty(mitk::ModelFitConstants::UID_PROPERTY_NAME(), "data_uid", "UID used to identify data in an MITK session.");

    registerProperty(mitk::ModelFitConstants::INPUT_VARIABLES_PROPERTY_NAME(), "modelfit_input_variables", "Array of input variables used in/for a model fit.", PropertyPersistenceDeserialization::deserializeXMLToScalarListLookupTableProperty, PropertyPersistenceSerialization::serializeScalarListLookupTablePropertyToXML);

    registerProperty(mitk::ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME(), "modelfit_parameter_name", "Name of the parameter, that is represented by the data and how it is used in the function string (modelfit.model.function).");
    registerProperty(mitk::ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME(), "modelfit_parameter_unit", "Unit string of the Parameter. Is only used for display. Default value: \"\" (no unit)");
    registerProperty(mitk::ModelFitConstants::PARAMETER_SCALE_PROPERTY_NAME(), "modelfit_parameter_scale", "Scaling of the parameter. Default value: 1.");
    registerProperty(mitk::ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME(), "modelfit_parameter_type", "Type of the parameters. Default value: parameter. Other options: derived, criterion, evaluation.");

    registerProperty(mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME(), "modelfit_model_type", "Value specifies the type of model (helpfull for classification; e.g. MR perfusion)");
    registerProperty(mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME(), "modelfit_model_name", "Name of the specific fit. Only used for display.");
    registerProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME(), "modelfit_model_function", "Functions string, that specifies the model and will be parsed, to plot the curves based on the parameter. Optional parameter that must not be set.");
    registerProperty(mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME(), "modelfit_model_functionClass", "ID of the model class implementation.");
    registerProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME(), "modelfit_model_x", "Value identifies the model type.");

    registerProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME(), "modelfit_xaxis_name", "Value identifies the model type.");
    registerProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME(), "modelfit_xaxis_unit", "Value identifies the model type.");

    registerProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME(), "modelfit_yaxis_name", "Value identifies the model type.");
    registerProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME(), "modelfit_yaxis_unit", "Value identifies the model type.");

    registerProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME(), "modelfit_fit_uid", "Value identifies the model type.");
    registerProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME(), "modelfit_fit_name", "Human readable name for the fit.");
    registerProperty(mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME(), "modelfit_fit_type", "Value identifies the model type.");
    registerProperty(mitk::ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME(), "modelfit_fit_input_imageUID", "Value identifies the model type.");
    registerProperty(mitk::ModelFitConstants::FIT_INPUT_ROIUID_PROPERTY_NAME(), "modelfit_fit_input_roiUID", "Value identifies the model type.");
    registerProperty(mitk::ModelFitConstants::FIT_INPUT_DATA_PROPERTY_NAME(), "modelfit_fit_input_data", "Value identifies the model type.");
    registerProperty(mitk::ModelFitConstants::FIT_STATIC_PARAMETERS_PROPERTY_NAME(), "modelfit_fit_staticParameters", "Value identifies the model type.", PropertyPersistenceDeserialization::deserializeXMLToScalarListLookupTableProperty, PropertyPersistenceSerialization::serializeScalarListLookupTablePropertyToXML);
  }

  void Unload(us::ModuleContext* )
  {
  }

private:

};
}

US_EXPORT_MODULE_ACTIVATOR(mitk::ModelFitIOActivator)
