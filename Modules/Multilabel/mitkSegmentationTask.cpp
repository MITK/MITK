/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationTask.h"
#include <mitkProperties.h>

mitk::SegmentationTask::SegmentationTask()
{
  // A base data cannot be serialized if empty. To be not considered empty its
  // geometry must consist of at least one time step. However, a segmentation
  // task would then appear as invisible spacial object in a scene. This can
  // be prevented by excluding it from the scene's bounding box calculations.
  this->GetTimeGeometry()->Expand(1);
  this->SetProperty("includeInBoundingBox", BoolProperty::New(false));
}

mitk::SegmentationTask::SegmentationTask(const Self& other)
  : BaseData(other)
{
}

mitk::SegmentationTask::~SegmentationTask()
{
}

void mitk::SegmentationTask::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::SegmentationTask::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::SegmentationTask::VerifyRequestedRegion()
{
  return true;
}

void mitk::SegmentationTask::SetRequestedRegion(const itk::DataObject*)
{
}
