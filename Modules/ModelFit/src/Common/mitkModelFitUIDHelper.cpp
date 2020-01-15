/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModelFitUIDHelper.h"


// Mitk
#include <mitkExceptionMacro.h>
#include <mitkModelFitConstants.h>
#include <mitkUIDGenerator.h>
#include <mitkDataNode.h>
#include <mitkBaseData.h>
#include <mitkDataStorage.h>
#include <mitkPropertyNameHelper.h>

mitk::DataNode::Pointer mitk::GetNodeByModelFitUID(const mitk::DataStorage* storage, const NodeUIDType& uid)
{
  mitk::DataNode::Pointer result;

  if (storage)
  {
    auto nodes = storage->GetAll();
    for (auto node : *(nodes.GetPointer()))
    {
      if (CheckModelFitUID(node, uid))
      {
        result = node;
        break;
      }
    }
  }

  return result;
}

mitk::NodeUIDType mitk::EnsureModelFitUID(mitk::DataNode* node)
{
  if (!node)
  {
    mitkThrow() << "Cannot ensure node UID. Passed node pointer is NULL.";
  }

  return EnsureModelFitUID(node->GetData());
};

mitk::NodeUIDType mitk::EnsureModelFitUID(mitk::BaseData* data)
{
  if (!data)
  {
    mitkThrow() << "Cannot ensure node UID. Passed node pointer is NULL.";
  }

  BaseProperty::Pointer uidProp = data->GetProperty(mitk::ModelFitConstants::UID_PROPERTY_NAME().c_str());
  std::string propUID = "";


  if (uidProp.IsNotNull())
  {
    propUID = uidProp->GetValueAsString();
  }
  else
  {
    //if the "user" defined UID is not found we will check if there is a DICOM series instance UID and use this value if available.
    uidProp = data->GetProperty(GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str());

    if (uidProp.IsNotNull())
    {
      propUID = uidProp->GetValueAsString();
    }
    else
    {
      mitk::UIDGenerator generator;
      propUID = generator.GetUID();

      data->SetProperty(mitk::ModelFitConstants::UID_PROPERTY_NAME().c_str(), mitk::StringProperty::New(propUID));
    }
  }

  return propUID;
};

bool mitk::CheckModelFitUID(const mitk::DataNode* node, const NodeUIDType& uid)
{
  if (node)
  {
    return CheckModelFitUID(node->GetData(), uid);
  }

  return false;
};

bool mitk::CheckModelFitUID(const mitk::BaseData* data, const NodeUIDType& uid)
{
  bool result = false;

  if (data)
  {
    BaseProperty::Pointer uidProp = data->GetProperty(mitk::ModelFitConstants::UID_PROPERTY_NAME().c_str());

    if (uidProp.IsNotNull())
    {
      result = uidProp->GetValueAsString() == uid;
    }

    if (!result)
    {
      //if the "user" defined UID does not match, we will check if there is a DICOM series instance UID and use this value for the check.
      uidProp = data->GetProperty(GeneratePropertyNameForDICOMTag(0x0020, 0x000e).c_str());

      if (uidProp.IsNotNull())
      {
        result = uidProp->GetValueAsString() == uid;
      }
    }
  }

  return result;
};

