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

#include "mitkResultNodeGenerationHelper.h"

#include <mitkProperties.h>
#include <mitkMatchPointPropertyTags.h>
#include <mitkImage.h>

mitk::DataNode::Pointer
  mitk::generateRegistrationResultNode(const std::string& nodeName, mitk::MAPRegistrationWrapper::Pointer resultReg, const std::string& algorithmUID, const std::string& movingDataUID, const std::string& targetDataUID)
{
  if (resultReg.IsNull())
  {
    mitkThrow() << "Cannot generate registration result node. Passed registration wrapper points to nullptr.";
  }

  mitk::DataNode::Pointer m_spRegNode = mitk::DataNode::New();
  m_spRegNode->SetData(resultReg);
  m_spRegNode->SetName(nodeName);
  resultReg->SetProperty(mitk::Prop_RegAlgUsed, mitk::StringProperty::New(algorithmUID));
  resultReg->SetProperty(mitk::Prop_RegAlgMovingData, mitk::StringProperty::New(movingDataUID));
  resultReg->SetProperty(mitk::Prop_RegAlgTargetData, mitk::StringProperty::New(targetDataUID));
  resultReg->SetProperty(mitk::Prop_RegUID, mitk::StringProperty::New(resultReg->GetRegistration()->getRegistrationUID()));

  return m_spRegNode;
};


mitk::DataNode::Pointer
  mitk::generateMappedResultNode(const std::string& nodeName, mitk::BaseData::Pointer mappedData, const std::string& regUID, const std::string& inputDataUID, const bool refinedGeometry, const std::string& interpolator)
{
  if (mappedData.IsNull())
  {
    mitkThrow() << "Cannot generate mapping result node. Passed mapped data points to nullptr.";
  }

  mitk::DataNode::Pointer mappedDataNode = mitk::DataNode::New();

  mappedDataNode->SetData(mappedData);
  mappedDataNode->SetName(nodeName);
  if (!regUID.empty())
  {
    mappedData->SetProperty(mitk::Prop_RegUID, mitk::StringProperty::New(regUID));
  }
  mappedData->SetProperty(mitk::Prop_MappingInputData, mitk::StringProperty::New(inputDataUID));
  if (refinedGeometry)
  {
    mappedData->SetProperty(mitk::Prop_MappingInterpolator, mitk::StringProperty::New("None"));
    mappedData->SetProperty(mitk::Prop_MappingRefinedGeometry, mitk::BoolProperty::New(true));
  }
  else
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(mappedData.GetPointer());

    if(image)
    {
      mappedData->SetProperty(mitk::Prop_MappingInterpolator, mitk::StringProperty::New(interpolator));
    }
    else
    {
      mappedData->SetProperty(mitk::Prop_MappingInterpolator, mitk::StringProperty::New("None"));
      mappedDataNode->SetColor(0.0, 0.0, 1.0);
    }
  }

  return mappedDataNode;
};
