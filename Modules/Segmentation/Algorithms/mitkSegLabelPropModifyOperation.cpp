/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegLabelPropModifyOperation.h"
#include <mitkImageTimeSelector.h>
#include <mitkImage.h>

mitk::SegLabelPropModifyOperation::SegLabelPropModifyOperation(MultiLabelSegmentation* segmentation,
  const ModifyLabelsVectorType& modifiedLabels)
  : SegChangeOperationBase(segmentation, 1), m_ModifiedLabels(modifiedLabels)
{
  for (auto& label : modifiedLabels)
  {
    if (!segmentation->ExistLabel(label->GetValue()))
    {
      mitkThrow() << "Invalid usage of SegLabelPropModifyOperation. At least one modified label passed to the operation"
        " does nit exist in segmentation. Invalid label value: " << label->GetValue();
    }
  }
}

const mitk::SegLabelPropModifyOperation::ModifyLabelsVectorType& mitk::SegLabelPropModifyOperation::GetModifiedLabels() const
{
  return m_ModifiedLabels;
}

mitk::SegLabelPropModifyOperation* mitk::SegLabelPropModifyOperation::CreatFromSegmentation(
  MultiLabelSegmentation* segmentation, const MultiLabelSegmentation::LabelValueVectorType& relevantLabels)
{
  if (nullptr == segmentation)
    mitkThrow() << "Invalid call of CreatFromSegmentation. Segmentation is not valid (nullptr).";

  ModifyLabelsVectorType labelData;
  for (auto labelValue : relevantLabels)
  {
    auto label = segmentation->GetLabel(labelValue);

    if (nullptr == label)
      mitkThrow() << "Invalid call of CreatFromSegmentation. Relevant label value does not exist in passed segmentation. Invalid value: " << labelValue;

    labelData.push_back(label->Clone());
  }

  return new SegLabelPropModifyOperation(segmentation, labelData);
}

