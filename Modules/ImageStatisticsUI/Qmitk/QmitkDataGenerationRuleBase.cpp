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

#include "QmitkDataGenerationRuleBase.h"

#include "mitkProperties.h"
#include "mitkImage.h"

bool QmitkDataGenerationRuleBase::IsResultAvailable(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const {
  auto resultNode = GetLatestResult(storage, doseNode, structNode);
  if (!resultNode) {
    return false;
  }
  else {
    //use BaseData as DataNode may have different MTime due to visualization changes
    auto resultNeedsUpdate = resultNode && (doseNode->GetData()->GetMTime() > resultNode->GetData()->GetMTime() || structNode->GetData()->GetMTime() > resultNode->GetData()->GetMTime());
    return !resultNeedsUpdate;
  }
}

mitk::DataNode::Pointer QmitkDataGenerationRuleBase::CreateDataNodeInterimResults(mitk::BaseData::Pointer data) const {
  if (!data) {
    mitkThrow() << "data is nullptr";
  }

  auto interimResultsNode = mitk::DataNode::New();
	interimResultsNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  data->SetProperty(mitk::STATS_GENERATION_STATUS_PROPERTY_NAME.c_str(), mitk::StringProperty::New(mitk::STATS_GENERATION_STATUS_VALUE_WORK_IN_PROGRESS));
  interimResultsNode->SetVisibility(false);
	interimResultsNode->SetData(data);
	return interimResultsNode;
}
