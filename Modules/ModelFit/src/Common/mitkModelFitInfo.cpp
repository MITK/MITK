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

#include <mitkNodePredicateDataProperty.h>
#include <mitkUIDGenerator.h>
#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkModelFitInfo.h"
#include "mitkScalarListLookupTableProperty.h"
#include "mitkModelFitException.h"

void mitk::modelFit::ModelFitInfo::AddParameter(Parameter::Pointer p)
{
  if (p.IsNull())
  {
    mitkThrow() << "Given parameter must not be NULL";
  }

  if (GetParameter(p->name, p->type).IsNull())
  {
    MITK_DEBUG << "Adding parameter '" << p->name << "with type " << p->type
               << "' to modelFit '" << uid << "'.";

    LockType lock(mutex);
    parameterList.push_back(p);
  }
  else
  {
    MITK_DEBUG << "Parameter '" << p->name << "' of modelFit '" << uid
               << "' already exists. Aborting.";
  }
}

mitk::modelFit::Parameter::ConstPointer
mitk::modelFit::ModelFitInfo::GetParameter(const std::string& name,
    const Parameter::Type& type) const
{
  for (ConstIterType iter = parameterList.begin(); iter != parameterList.end(); ++iter)
  {
    Parameter::ConstPointer p = static_cast<Parameter::ConstPointer>(*iter);

    if (p->name == name && p->type == type)
    {
      return p;
    }
  }

  return NULL;
}


const mitk::modelFit::ModelFitInfo::ParamListType& mitk::modelFit::ModelFitInfo::GetParameters()
const
{
  return this->parameterList;
};

void mitk::modelFit::ModelFitInfo::DeleteParameter(const std::string& name,
    const Parameter::Type& type)
{
  for (IterType iter = parameterList.begin(); iter != parameterList.end(); ++iter)
  {
    Parameter::ConstPointer p = static_cast<Parameter::ConstPointer>(*iter);

    if (p->name == name && p->type == type)
    {
      MITK_DEBUG << "Deleting parameter '" << name << " with type " << type
                 << "' from modelFit '" << uid << "'.";

      LockType lock(mutex);
      parameterList.erase(iter);
      return;
    }
  }
}


const std::string mitk::modelFit::GetMandatoryProperty(const mitk::DataNode* node,
    const std::string& prop)
{
  std::string result;

  if (!node || !node->GetData() ||
      !node->GetData()->GetPropertyList()->GetStringProperty(prop.c_str(), result) || result.empty())
  {
    mitkThrowException(mitk::modelFit::ModelFitException) << "Node " << node->GetName()
        << " is lacking the required "
        << "property '" << prop
        << "' or contains an empty string.";
  }

  return result;
}

const std::string mitk::modelFit::GetMandatoryProperty(const mitk::BaseData* data,
  const std::string& prop)
{
  std::string result;

  if (!data || !data->GetPropertyList()->GetStringProperty(prop.c_str(), result) || result.empty())
  {
    mitkThrowException(mitk::modelFit::ModelFitException) << "Data is lacking the required "
      << "property '" << prop
      << "' or contains an empty string.";
  }

  return result;
}

mitk::modelFit::ModelFitInfo::Pointer
mitk::modelFit::CreateFitInfoFromNode(const ModelFitInfo::UIDType& uid,
                                      const mitk::DataStorage* storage)
{
  if (!storage)
  {
    return NULL;
  }

  mitk::DataStorage::SetOfObjects::ConstPointer nodes = GetNodesOfFit(uid, storage);

  if (nodes.IsNull() || nodes->empty())
  {
    return NULL;
  }

  mitk::DataNode::ConstPointer node = nodes->GetElement(
                                        0).GetPointer(); //take one of the nodes as template

  if (!node->GetData())
  {
    return NULL;
  }

  ModelFitInfo::Pointer fit = ModelFitInfo::New();
  fit->uid = uid;

  // Mandatory properties
  try
  {
    fit->fitType = GetMandatoryProperty(node, mitk::ModelFitConstants::FIT_TYPE_PROPERTY_NAME());
    fit->modelType = GetMandatoryProperty(node, mitk::ModelFitConstants::MODEL_TYPE_PROPERTY_NAME());
    fit->modelName = GetMandatoryProperty(node, mitk::ModelFitConstants::MODEL_NAME_PROPERTY_NAME());
  }
  catch (const ModelFitException& e)
  {
    MITK_ERROR << e.what();
    return NULL;
  }

  // Either a function string or a function class must exist
  if (!node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(),
                               fit->function))
  {
    fit->function = "";
  }

  try
  {
    fit->functionClassID =
      GetMandatoryProperty(node, mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME());
  }
  catch (const ModelFitException&)
  {
    if (fit->function.empty())
    {
      MITK_ERROR << "The properties '"
                 << mitk::ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME()
                 << "'and '" << mitk::ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME()
                 << "' are both empty or missing. One of these is required.";
      return NULL;
    }
  }

  node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), fit->fitName);
  node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), fit->x);
  node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), fit->xAxisName);
  node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), fit->xAxisUnit);
  node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), fit->yAxisName);
  node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), fit->yAxisUnit);

  // Parameter
  for (DataStorage::SetOfObjects::ConstIterator pos = nodes->Begin(); pos != nodes->End(); ++pos)
  {
    modelFit::Parameter::Pointer param = ExtractParameterFromData(pos->Value()->GetData());

    if (param.IsNotNull())
    {
      fit->AddParameter(param);
    }
  }

  // Static parameters
  mitk::ScalarListLookupTableProperty::ConstPointer varProp = dynamic_cast<const mitk::ScalarListLookupTableProperty*>(node->GetData()->GetProperty(mitk::ModelFitConstants::FIT_STATIC_PARAMETERS_PROPERTY_NAME().c_str()).GetPointer());

  if (varProp.IsNotNull())
  {
    const mitk::ScalarListLookupTable lut = varProp->GetValue();
    const mitk::ScalarListLookupTable::LookupTableType& varMap = lut.GetLookupTable();

    for (mitk::ScalarListLookupTable::LookupTableType::const_iterator mapIter =
           varMap.begin(); mapIter != varMap.end(); ++mapIter)
    {
      fit->staticParamMap.Add(mapIter->first, mapIter->second);
    }
  }

  //fit input and ROI
  try
  {
    fit->inputUID = GetMandatoryProperty(node,
                                         mitk::ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME());
  }
  catch (const ModelFitException& e)
  {
    MITK_ERROR << e.what();
    return NULL;
  }

  if (storage)
  {
    mitk::DataNode::Pointer inputNode = GetNodeByModelFitUID(storage, fit->inputUID);

    if (inputNode.IsNull())
    {
      MITK_ERROR << "Cannot create valid model fit info. input node cannot be found.";
      return NULL;
    }

    mitk::Image::Pointer inputImage = dynamic_cast<mitk::Image*>(inputNode->GetData());

    if (inputImage.IsNull())
    {
      MITK_ERROR << "Cannot create valid model fit info. input node does not contain an image.";
      return NULL;
    }

    fit->inputImage = inputImage;
  }

  node->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::FIT_INPUT_ROIUID_PROPERTY_NAME().c_str(),
                          fit->roiUID);

  mitk::ScalarListLookupTableProperty::ConstPointer inputDataProp = dynamic_cast<const mitk::ScalarListLookupTableProperty*>(node->GetData()->GetProperty(mitk::ModelFitConstants::FIT_INPUT_DATA_PROPERTY_NAME().c_str()).GetPointer());
  if (inputDataProp.IsNotNull())
  {
    fit->inputData = inputDataProp->GetValue();
  }

  return fit;
}

mitk::modelFit::ModelFitInfo::Pointer
mitk::modelFit::CreateFitInfoFromModelParameterizer(const ModelParameterizerBase* usedParameterizer,
mitk::BaseData* inputImage, const std::string& fitType, const std::string& fitName,
    const NodeUIDType roiUID)
{
  if (!usedParameterizer)
  {
    return NULL;
  }

  UIDGenerator generator("FitUID_");
  std::string uid = generator.GetUID();

  ModelFitInfo::Pointer fit = ModelFitInfo::New();
  fit->uid = uid;
  fit->fitType = fitType;
  fit->fitName = fitName;
  fit->inputImage = dynamic_cast<Image*>(inputImage);
  fit->inputUID = EnsureModelFitUID(inputImage);

  if (fit->inputImage.IsNull())
  {
    mitkThrow() << "Cannot generate model fit info. Input node does not contain an image.";
  }

  fit->modelType = usedParameterizer->GetModelType();
  fit->modelName = usedParameterizer->GetModelDisplayName();

  fit->function = usedParameterizer->GetFunctionString();
  fit->x = usedParameterizer->GetXName();
  fit->functionClassID = usedParameterizer->GetClassID();

  fit->xAxisName = usedParameterizer->GetXAxisName();
  fit->xAxisUnit = usedParameterizer->GetXAxisUnit();
  fit->yAxisName = usedParameterizer->GetYAxisName();
  fit->yAxisUnit = usedParameterizer->GetYAxisUnit();

  // Parameter
  ModelTraitsInterface::ParameterNamesType paramNames = usedParameterizer->GetParameterNames();
  ModelTraitsInterface::ParamterScaleMapType paramScales = usedParameterizer->GetParameterScales();
  ModelTraitsInterface::ParamterUnitMapType paramUnits = usedParameterizer->GetParameterUnits();

  for (ModelTraitsInterface::ParameterNamesType::iterator pos = paramNames.begin();
       pos != paramNames.end(); ++pos)
  {
    modelFit::Parameter::Pointer param = modelFit::Parameter::New();
    param->name = *pos;
    param->type = Parameter::ParameterType;

    if (paramScales.find(*pos) == paramScales.end())
    {
      mitkThrow() <<
                  "Cannot generate model fit info. Model traits invalid (scales do not include parameter). Parameter name: "
                  << *pos;
    }

    if (paramUnits.find(*pos) == paramUnits.end())
    {
      mitkThrow() <<
                  "Cannot generate model fit info. Model traits invalid (units do not include parameter). Parameter name: "
                  << *pos;
    }

    param->scale = paramScales[*pos];
    param->unit = paramUnits[*pos];
    fit->AddParameter(param);
  }

  //derived parameter
  ModelTraitsInterface::DerivedParameterNamesType derivedNames =
    usedParameterizer->GetDerivedParameterNames();
  ModelTraitsInterface::DerivedParamterScaleMapType derivedScales =
    usedParameterizer->GetDerivedParameterScales();
  ModelTraitsInterface::DerivedParamterUnitMapType derivedUnits =
    usedParameterizer->GetDerivedParameterUnits();

  for (ModelTraitsInterface::ParameterNamesType::iterator pos = derivedNames.begin();
       pos != derivedNames.end(); ++pos)
  {
    modelFit::Parameter::Pointer param = modelFit::Parameter::New();
    param->name = *pos;
    param->type = Parameter::DerivedType;

    if (derivedScales.find(*pos) == derivedScales.end())
    {
      mitkThrow() <<
                  "Cannot generate model fit info. Model traits invalid (scales do not include parameter). Parameter name: "
                  << *pos;
    }

    if (derivedUnits.find(*pos) == derivedUnits.end())
    {
      mitkThrow() <<
                  "Cannot generate model fit info. Model traits invalid (units do not include parameter). Parameter name: "
                  << *pos;
    }

    param->scale = derivedScales[*pos];
    param->unit = derivedUnits[*pos];
    fit->AddParameter(param);
  }

  // Static parameters (but transfer only the global ones)
  ModelParameterizerBase::StaticParameterMapType staticParamMap =
    usedParameterizer->GetGlobalStaticParameters();

  for (ModelParameterizerBase::StaticParameterMapType::const_iterator pos = staticParamMap.begin();
       pos != staticParamMap.end(); ++pos)
  {
    fit->staticParamMap.Add(pos->first, pos->second);
  }


  fit->roiUID = roiUID;

  return fit;
}

mitk::modelFit::ModelFitInfo::Pointer
mitk::modelFit::CreateFitInfoFromModelParameterizer(const ModelParameterizerBase* usedParameterizer,
mitk::BaseData* inputImage, const std::string& fitType, const ScalarListLookupTable& inputData,
const std::string& fitName, const NodeUIDType roiUID)
{
  mitk::modelFit::ModelFitInfo::Pointer info = CreateFitInfoFromModelParameterizer(usedParameterizer,
    inputImage, fitType, fitName, roiUID);

  info->inputData = inputData;

  return info;
}

mitk::DataStorage::SetOfObjects::ConstPointer
mitk::modelFit::GetNodesOfFit(const ModelFitInfo::UIDType& fitUID, const mitk::DataStorage* storage)
{
  if (!storage)
  {
    return NULL;
  }

  mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(
        mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(fitUID));
  return storage->GetSubset(predicate);
};


mitk::modelFit::NodeUIDSetType
mitk::modelFit::GetFitUIDsOfNode(const mitk::DataNode* node,	const mitk::DataStorage* storage)
{
  mitk::modelFit::NodeUIDSetType result;

  if (node && storage)
  {
    mitk::NodePredicateDataProperty::Pointer predicate = mitk::NodePredicateDataProperty::New(
          mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str());
    mitk::DataStorage::SetOfObjects::ConstPointer nodes = storage->GetDerivations(node, predicate,
        false);

    for (mitk::DataStorage::SetOfObjects::ConstIterator pos = nodes->Begin(); pos != nodes->End();
         ++pos)
    {
      mitk::modelFit::ModelFitInfo::UIDType uid;
      pos->Value()->GetData()->GetPropertyList()->GetStringProperty(mitk::ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), uid);
      result.insert(uid);
    }

  }

  return result;
};
