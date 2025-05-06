/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegSliceOperationApplier.h"

#include "mitkSegSliceOperation.h"
#include "mitkRenderingManager.h"
#include "mitkSegTool2D.h"
#include <mitkExtractSliceFilter.h>
#include <mitkVtkImageOverwrite.h>

// VTK
#include <vtkSmartPointer.h>

mitk::SegSliceOperationApplier::SegSliceOperationApplier()
{
}

mitk::SegSliceOperationApplier::~SegSliceOperationApplier()
{
}

void mitk::SegSliceOperationApplier::ExecuteOperation(Operation *operation)
{
  auto *segSliceOperation = dynamic_cast<SegSliceOperation *>(operation);

  // as we only support SegSliceOperation return if operation is not type of SegSliceOperation
  if (!segSliceOperation)
    return;

  // check if the operation is valid
  if (segSliceOperation->IsValid())
  {
    auto relevantGroupImage = segSliceOperation->GetSegmentation()->GetGroupImage(segSliceOperation->GetGroupID());
    SegTool2D::WriteSliceToVolume(relevantGroupImage, segSliceOperation->GetSlicePlaneGeometry(), segSliceOperation->GetSlice(), segSliceOperation->GetTimeStep());
    SegTool2D::UpdateAllSurfaceInterpolations(segSliceOperation->GetSegmentation(), segSliceOperation->GetTimeStep(), segSliceOperation->GetSlicePlaneGeometry(), true);
  }
}

mitk::SegSliceOperationApplier *mitk::SegSliceOperationApplier::GetInstance()
{
  static auto *s_Instance = new SegSliceOperationApplier();
  return s_Instance;
}
