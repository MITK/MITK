/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegGroupModifyOperationApplier.h"

#include "mitkSegGroupModifyOperation.h"
#include "mitkRenderingManager.h"
#include "mitkSegTool2D.h"
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>

// VTK
#include <vtkSmartPointer.h>

mitk::SegGroupModifyOperationApplier::SegGroupModifyOperationApplier()
{
}

mitk::SegGroupModifyOperationApplier::~SegGroupModifyOperationApplier()
{
}

void mitk::SegGroupModifyOperationApplier::ExecuteOperation(Operation *operation)
{
  auto *segGroupModOperation = dynamic_cast<SegGroupModifyOperation *>(operation);

  // as we only support SegGroupModifyOperation return if operation is not type of SegGroupModifyOperation
  if (nullptr == segGroupModOperation)
    return;

  // check if the operation is valid
  if (segGroupModOperation->IsValid())
  {
    auto segmentation = segGroupModOperation->GetSegmentation();
    bool modified = false;

    for (auto modifiedImageGroupID : segGroupModOperation->GetImageGroupIDs())
    {
      for (auto timeStep : segGroupModOperation->GetImageTimeSteps(modifiedImageGroupID))
      {
        auto modifiedImage = segGroupModOperation->GetModifiedGroupImage(modifiedImageGroupID, timeStep);
        segmentation->UpdateGroupImage(modifiedImageGroupID, modifiedImage, timeStep);
        modified = true;
      }
    }

    for (auto modifiedGroupID : segGroupModOperation->GetLabelGroupIDs())
    {
        segmentation->ReplaceGroupLabels(modifiedGroupID, segGroupModOperation->GetModifiedLabels(modifiedGroupID));
        modified = true;
    }

    if (modified)
    {
      segmentation->Modified();
    }
  }
}

mitk::SegGroupModifyOperationApplier *mitk::SegGroupModifyOperationApplier::GetInstance()
{
  static auto *s_Instance = new SegGroupModifyOperationApplier();
  return s_Instance;
}
