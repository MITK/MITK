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

#include "mitkUIDHelper.h"


// Mitk
#include <mitkUIDGenerator.h>
#include <mitkExceptionMacro.h>
#include <mitkDataNode.h>
#include <mitkBaseData.h>

mitk::NodeUIDType mitk::EnsureUID(mitk::DataNode* node)
{
  if (!node)
  {
    mitkThrow() << "Cannot ensure node UID. Passed node pointer is nullptr.";
  }

  std::string propUID = "";
  if (!node->GetStringProperty(mitk::nodeProp_UID,propUID))
  {
    mitk::UIDGenerator generator;
    propUID = generator.GetUID();

    node->SetStringProperty(mitk::nodeProp_UID,propUID.c_str());
  }

  return propUID;
};

bool mitk::CheckUID(const mitk::DataNode* node, const NodeUIDType& uid)
{
  bool result = false;

  if (node)
  {
    std::string propUID = "";
    if (node->GetStringProperty(mitk::nodeProp_UID,propUID))
    {
      result = propUID == uid;
    }
  }

  return result;
};

mitk::NodeUIDType mitk::EnsureUID(mitk::BaseData* data)
{
  if (!data)
  {
    mitkThrow() << "Cannot ensure node UID. Passed node pointer is nullptr.";
  }

  BaseProperty::Pointer uidProp = data->GetProperty(mitk::Prop_UID);
  std::string propUID = "";

  if (uidProp.IsNotNull())
  {
    propUID = uidProp->GetValueAsString();
  }
  else
  {
    mitk::UIDGenerator generator;
    propUID = generator.GetUID();

    data->SetProperty(mitk::Prop_UID, mitk::StringProperty::New(propUID));
  }

  return propUID;
};

bool mitk::CheckUID(const mitk::BaseData* data, const NodeUIDType& uid)
{
  bool result = false;

  BaseProperty::Pointer uidProp = data->GetProperty(mitk::Prop_UID);

  if (uidProp.IsNotNull())
  {
    result = uidProp->GetValueAsString() == uid;
  }

  return result;
};
