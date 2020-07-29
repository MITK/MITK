/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkModelFitResultRelationRule.h"
#include "mitkModelFitConstants.h"

#include <mitkDataNode.h>

bool mitk::ModelFitResultRelationRule::IsDestinationCandidate(const IPropertyProvider *owner) const
{
  auto node = dynamic_cast<const DataNode*>(owner);

  auto image = nullptr != node
    ? dynamic_cast<const Image*>(node->GetData())
    : dynamic_cast<const Image*>(owner);

  return image != nullptr && image->GetTimeSteps()>1;
}


mitk::ModelFitResultRelationRule::ModelFitResultRelationRule()
  : SourceImageRelationRule("Model fit input", "Model fit result relation", "fit result", "source image")
{}


mitk::ModelFitResultRelationRule::DataRelationUIDVectorType
mitk::ModelFitResultRelationRule::GetRelationUIDs_DataLayer(const IPropertyProvider* source,
  const IPropertyProvider* destination, const InstanceIDVectorType& instances_IDLayer) const
{
  DataRelationUIDVectorType result = Superclass::GetRelationUIDs_DataLayer(source, destination, instances_IDLayer);

  if (result.empty())
  { //check if there is a relation based on legacy ModelFitUID
    auto imageUIDprop = source->GetConstProperty(ModelFitConstants::LEGACY_FIT_INPUT_IMAGEUID_PROPERTY_NAME());
    if (imageUIDprop.IsNotNull())
    {
      const auto inputImageUID = imageUIDprop->GetValueAsString();
      bool isValid = nullptr == destination;
      if (nullptr != destination)
      {
        auto modelFitUIDprop = destination->GetConstProperty(ModelFitConstants::LEGACY_UID_PROPERTY_NAME());

        if (modelFitUIDprop.IsNotNull())
        {
          const auto destinationUID = modelFitUIDprop->GetValueAsString();
          isValid = destinationUID == inputImageUID;
        }
      }
      if (isValid)
      {
        result.emplace_back("model.fit.input.image.legacy.relation", this->GetRuleID());
      }
    }
  }

  return result;
}

void mitk::ModelFitResultRelationRule::Disconnect_datalayer(IPropertyOwner * source, const RelationUIDType & relationUID) const
{
  Superclass::Disconnect_datalayer(source, relationUID);
  //Legacy connection (LEGACY_FIT_INPUT_IMAGEUID_PROPERTY_NAME) cannot be
  //disconnected on purpose.
}

itk::LightObject::Pointer mitk::ModelFitResultRelationRule::InternalClone() const
{
  itk::LightObject::Pointer result = Self::New().GetPointer();

  return result;
}
