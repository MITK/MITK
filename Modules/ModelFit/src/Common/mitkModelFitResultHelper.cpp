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

#include "mitkModelFitResultHelper.h"

#include <mitkDataStorage.h>
#include <mitkUIDGenerator.h>

#include "mitkModelTraitsInterface.h"
#include "mitkModelFitConstants.h"
#include "mitkModelFitInfo.h"

#include <mitkDICOMPMPropertyHelper.h>
#include <mitkDICOMQIPropertyHelper.h>

namespace mitk
{
  namespace modelFit
  {
    void AdaptDataPropertyToParameter(mitk::BaseData* data, const ModelBase::ParameterNameType& name, modelFit::Parameter::Type dataType, const modelFit::ModelFitInfo* fitInfo)
    {
      assert(data);

      if (!data)
      {
        mitkThrow() << "Cannot add model or fit properties to data instance. Passed data instance is null. parameter name:" << name;
      }

      if (!fitInfo)
      {
        mitkThrow() << "Cannot add model or fit properties to data. Passed fit info instance is null. parameter name:" << name;
      }

      data->GetPropertyList()->SetStringProperty(ModelFitConstants::PARAMETER_NAME_PROPERTY_NAME().c_str(),name.c_str());

      if (dataType == modelFit::Parameter::ParameterType)
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), ModelFitConstants::PARAMETER_TYPE_VALUE_PARAMETER().c_str());
      }    
      else if (dataType == modelFit::Parameter::DerivedType)
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), ModelFitConstants::PARAMETER_TYPE_VALUE_DERIVED_PARAMETER().c_str());
      }
      else if (dataType == modelFit::Parameter::CriterionType)
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), ModelFitConstants::PARAMETER_TYPE_VALUE_CRITERION().c_str());
      } 
      else if (dataType == modelFit::Parameter::EvaluationType)
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::PARAMETER_TYPE_PROPERTY_NAME().c_str(), ModelFitConstants::PARAMETER_TYPE_VALUE_EVALUATION_PARAMETER().c_str());
      }


      if (dataType == modelFit::Parameter::ParameterType || dataType == modelFit::Parameter::DerivedType)
      {
        modelFit::Parameter::ConstPointer param = fitInfo->GetParameter(name,dataType);

        if (param.IsNull())
        {
          mitkThrow() << "Cannot generate model fit result data. Parameter name is not part of the model fit info. Parameter name: "<<name;
        }

        if (!param->unit.empty())
        {
          data->GetPropertyList()->SetStringProperty(ModelFitConstants::PARAMETER_UNIT_PROPERTY_NAME().c_str(), param->unit.c_str());
        }

        if (param->scale != 1.0)
        {
          data->GetPropertyList()->SetFloatProperty(ModelFitConstants::PARAMETER_SCALE_PROPERTY_NAME().c_str(), param->scale);
        }
      }

    };

    void AdaptDataPropertyToModelFit(mitk::BaseData* data, const modelFit::ModelFitInfo* fitInfo)
    {
      assert(data);

      if (!data)
      {
        mitkThrow() << "Cannot add model or fit properties to data. Passed data instance is null.";
      }

      if (!fitInfo)
      {
        mitkThrow() << "Cannot add model or fit properties to data. Passed model traits instance is null.";
      }

      //model section
      data->GetPropertyList()->SetStringProperty(ModelFitConstants::MODEL_TYPE_PROPERTY_NAME().c_str(), fitInfo->modelType.c_str());
      data->GetPropertyList()->SetStringProperty(ModelFitConstants::MODEL_NAME_PROPERTY_NAME().c_str(), fitInfo->modelName.c_str());

      data->GetPropertyList()->SetStringProperty(ModelFitConstants::MODEL_FUNCTION_CLASS_PROPERTY_NAME().c_str(), fitInfo->functionClassID.c_str());
      if(!(fitInfo->function.empty()))
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::MODEL_FUNCTION_PROPERTY_NAME().c_str(), fitInfo->function.c_str());
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::MODEL_X_PROPERTY_NAME().c_str(), fitInfo->x.c_str());
      }

      //axis section
      if (!fitInfo->xAxisName.empty())
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::XAXIS_NAME_PROPERTY_NAME().c_str(), fitInfo->xAxisName.c_str());
      }

      if (!fitInfo->xAxisUnit.empty())
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::XAXIS_UNIT_PROPERTY_NAME().c_str(), fitInfo->xAxisUnit.c_str());
      }

      if (!fitInfo->yAxisName.empty())
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::YAXIS_NAME_PROPERTY_NAME().c_str(), fitInfo->yAxisName.c_str());
      }

      if (!fitInfo->yAxisUnit.empty())
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::YAXIS_UNIT_PROPERTY_NAME().c_str(), fitInfo->yAxisUnit.c_str());
      }

      //fit section
      data->GetPropertyList()->SetStringProperty(ModelFitConstants::FIT_UID_PROPERTY_NAME().c_str(), fitInfo->uid.c_str());
      data->GetPropertyList()->SetStringProperty(ModelFitConstants::FIT_NAME_PROPERTY_NAME().c_str(), fitInfo->fitName.c_str());
      data->GetPropertyList()->SetStringProperty(ModelFitConstants::FIT_TYPE_PROPERTY_NAME().c_str(), fitInfo->fitType.c_str());
      data->GetPropertyList()->SetStringProperty(ModelFitConstants::FIT_INPUT_IMAGEUID_PROPERTY_NAME().c_str(), fitInfo->inputUID.c_str());

      if (fitInfo->inputData.GetLookupTable().size() > 0)
      {
        mitk::ScalarListLookupTableProperty::Pointer inputDataProp = mitk::ScalarListLookupTableProperty::New();
        inputDataProp->SetValue(fitInfo->inputData);

        data->SetProperty(ModelFitConstants::FIT_INPUT_DATA_PROPERTY_NAME().c_str(), inputDataProp);
      }

      if (!fitInfo->roiUID.empty())
      {
        data->GetPropertyList()->SetStringProperty(ModelFitConstants::FIT_INPUT_ROIUID_PROPERTY_NAME().c_str(), fitInfo->roiUID.c_str());
      }

      data->SetProperty(ModelFitConstants::FIT_STATIC_PARAMETERS_PROPERTY_NAME().c_str(), ConvertStaticParametersToProperty(fitInfo->staticParamMap));
    };

    mitk::DataNode::Pointer CreateNode(const ModelBase::ParameterNameType& name, Image* parameterImage, const ModelFitInfo* fitInfo)
    {
      if (!parameterImage)
      {
        mitkThrow() << "Cannot generate model fit result node. Passed parameterImage is null. parameter name: "<<name;
      }

      if (!fitInfo)
      {
        mitkThrow() << "Cannot generate model fit result node. Passed model traits instance is null. parameter name: "<<name;
      }

      DataNode::Pointer result = DataNode::New();

      result->SetData(parameterImage);

      std::string nodeName = name;

      if (!fitInfo->fitName.empty())
      {
        nodeName = fitInfo->fitName + "_" + nodeName;
      }

      result->SetName(nodeName);

      result->SetVisibility(false);

      return result;
    };
  }
}

mitk::ScalarListLookupTableProperty::Pointer mitk::modelFit::ConvertStaticParametersToProperty(const mitk::modelFit::StaticParameterMap& params)
{
  mitk::ScalarListLookupTableProperty::Pointer result = mitk::ScalarListLookupTableProperty::New();

  ScalarListLookupTable table;

  for(mitk::modelFit::StaticParameterMap::const_iterator pos = params.begin(); pos != params.end(); ++pos)
  {
    table.SetTableValue(pos->first,pos->second);
  }
  
  result->SetValue(table);

  return result;
};

MITKMODELFIT_EXPORT void mitk::modelFit::SetModelFitDataProperties(mitk::BaseData* data, const ModelBase::ParameterNameType& name, modelFit::Parameter::Type dataType, const modelFit::ModelFitInfo* fitInfo)
{
  AdaptDataPropertyToModelFit(data, fitInfo);
  AdaptDataPropertyToParameter(data, name, dataType, fitInfo);
};

MITKMODELFIT_EXPORT mitk::DataNode::Pointer mitk::modelFit::CreateResultNode( const ModelBase::ParameterNameType& name, modelFit::Parameter::Type nodeType, Image* parameterImage, const ModelFitInfo* modelFitInfo)
{
  if (!parameterImage)
  {
    mitkThrow() << "Cannot generate model fit result node. Passed parameterImage is null. parameter name: "<<name;
  }

  if (!modelFitInfo)
  {
    mitkThrow() << "Cannot generate model fit result node. Passed model fit info instance is null. parameter name: "<<name;
  }

  DataNode::Pointer result = CreateNode(name, parameterImage, modelFitInfo);
  SetModelFitDataProperties(parameterImage, name, nodeType, modelFitInfo);

  return result;
};

MITKMODELFIT_EXPORT mitk::modelFit::ModelFitResultNodeVectorType mitk::modelFit::CreateResultNodeMap( const ModelFitResultImageMapType& results, const ModelFitResultImageMapType& derivedResults, const ModelFitResultImageMapType& criterionResults, const ModelFitResultImageMapType& evaluationResults, const ModelFitInfo* fitInfo)
{
  if (!fitInfo)
  {
    mitkThrow() << "Cannot store model fit results in data storage. Passed model traits instance is null.";
  }

  ModelFitResultNodeVectorType nodes;

  for (ModelFitResultImageMapType::const_iterator pos = results.begin(); pos!=results.end(); ++pos)
  {
    DataNode::Pointer newNode = CreateResultNode(pos->first, modelFit::Parameter::ParameterType, pos->second, fitInfo);
    nodes.push_back(newNode);
  }

  for (ModelFitResultImageMapType::const_iterator pos = derivedResults.begin(); pos!=derivedResults.end(); ++pos)
  {
    DataNode::Pointer newNode = CreateResultNode(pos->first, modelFit::Parameter::DerivedType, pos->second, fitInfo);
    nodes.push_back(newNode);
  }

  for (ModelFitResultImageMapType::const_iterator pos = criterionResults.begin(); pos!=criterionResults.end(); ++pos)
  {
    DataNode::Pointer newNode = CreateResultNode(pos->first, modelFit::Parameter::CriterionType, pos->second, fitInfo);
    nodes.push_back(newNode);
  }

  for (ModelFitResultImageMapType::const_iterator pos = evaluationResults.begin(); pos!=evaluationResults.end(); ++pos)
  {
    DataNode::Pointer newNode = CreateResultNode(pos->first, modelFit::Parameter::EvaluationType, pos->second, fitInfo);
    nodes.push_back(newNode);
  }

  return nodes;
};

MITKMODELFIT_EXPORT void mitk::modelFit::StoreResultsInDataStorage(DataStorage* storage, const ModelFitResultNodeVectorType& resultNodes, DataNode* parentNode)
{
  if (!storage)
  {
    mitkThrow() << "Cannot store model fit results in data storage. Passed storage is null.";
  }

  for (ModelFitResultNodeVectorType::const_iterator pos = resultNodes.begin(); pos!=resultNodes.end(); ++pos)
  {
    storage->Add(*pos,parentNode);
  }

  // Set DICOM properties, paramap-secific (DICOMPM) and general properties from source data (DICOMQI)

  for (ModelFitResultNodeVectorType::const_iterator pos = resultNodes.begin(); pos != resultNodes.end(); pos++)
  {
    mitk::DICOMQIPropertyHelper::DeriveDICOMSourceProperties(parentNode->GetData(), pos->GetPointer()->GetData());
    mitk::DICOMPMPropertyHelper::DeriveDICOMPMProperties(pos->GetPointer()->GetData());
  }
  

};

