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

#include "QmitkTaskGenerationRuleBase.h"

#include "mitkRTHelper.h"
#include "mitkProperties.h"
#include "mitkRTPropertyConstants.h"
#include "mitkImage.h"
#include "mitkContourModelSet.h"
#include "mitkRTPredicates.h"
#include "QmitkVoxelizationJob.h"

bool QmitkTaskGenerationRuleBase::IsResultAvailable(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const {
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

mitk::DataNode::Pointer QmitkTaskGenerationRuleBase::CreateDataNodeInterimResults(mitk::BaseData::Pointer data) const {
  if (!data) {
    mitkThrow() << "data is nullptr";
  }

  auto interimResultsNode = mitk::DataNode::New();
	interimResultsNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  data->SetProperty(mitk::RT_CACHE_STATUS_NAME.c_str(), mitk::StringProperty::New(mitk::BASE_DATA_WORK_IN_PROGRESS_VALUE));
  interimResultsNode->SetVisibility(false);
	interimResultsNode->SetData(data);
	return interimResultsNode;
}

QmitkRTJobBase* QmitkTaskGenerationRuleBase::GetVoxelizationJob(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const {
	auto doseImage = dynamic_cast<mitk::Image*>(doseNode->GetData());
	auto structContourModelSet = dynamic_cast<mitk::ContourModelSet*>(structNode->GetData());
	if (!doseImage || !structContourModelSet) {
		mitkThrow() << "could not cast dose nodes to baseData";
	}
  //QmitkVoxelizationJob needs image as input, but will be changed to geometry input
	auto referenceGeometry = doseImage->GetSlicedGeometry();

  //image as data placeholder (prevents rendering problems as GetData() never returns nullptr)
  mitk::Image::Pointer imageBaseData = mitk::Image::New();
  mitk::PixelType pt = mitk::MakeScalarPixelType<double>();
  unsigned int dim[] = { 1,1,1 };
  imageBaseData->Initialize(pt, 3, dim);
  auto voxelizationNode = CreateDataNodeInterimResults(imageBaseData.GetPointer());
  voxelizationNode->SetName(doseNode->GetName()+"_"+structNode->GetName());
	auto voxelizationJob = new QmitkVoxelizationJob(doseImage, structContourModelSet, voxelizationNode);
  voxelizationJob->connectDataToInputData(imageBaseData);
	return voxelizationJob;
}

const mitk::DataNode* QmitkTaskGenerationRuleBase::GetLatestVoxelization(mitk::DataStorage::Pointer storage, mitk::DataNode::ConstPointer doseNode, mitk::DataNode::ConstPointer structNode) const {
  auto voxelizationPredicate = mitk::GetVoxelizationPredicate(doseNode->GetData(), structNode->GetData());

  auto resultNodes = storage->GetSubset(voxelizationPredicate);
  if (resultNodes->empty()) {
    return nullptr;
  }
  if (resultNodes->size() > 1) {
    MITK_INFO << "multiple voxelization nodes found. Return only the newest one.";
  }
  //get newest element
  auto latestNode = mitk::GetLatestDataNode(resultNodes);
  return latestNode;
}

mitk::BaseData::Pointer QmitkTaskGenerationRuleBase::GetVoxelizedResultTypeBaseData() const
{
  return mitk::Image::New().GetPointer();
}
