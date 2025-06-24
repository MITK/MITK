/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegSliceOperation.h"

#include <mitkImage.h>

mitk::SegSliceOperation::SegSliceOperation(MultiLabelSegmentation* segmentation,
  MultiLabelSegmentation::GroupIndexType groupID,
  const Image* slice,
  const TimeStepType timestep,
  const PlaneGeometry* planeGeometry)
  : SegChangeOperationBase(segmentation, 1), m_GroupID(groupID), m_TimeStep(timestep)

{
  m_PlaneGeometry = planeGeometry->Clone();
  /*
  Quick fix for bug 12338.
  Guard object - fix this when clone method of PlaneGeometry is cloning the reference geometry (see bug 13392)*/
  m_GuardReferenceGeometry = dynamic_cast<const PlaneGeometry *>(m_PlaneGeometry.GetPointer())->GetReferenceGeometry();
  /*---------------------------------------------------------------------------------------------------*/

  m_CompressedImageContainer.CompressImage(slice);
}

mitk::Image::Pointer mitk::SegSliceOperation::GetSlice() const
{
  return m_CompressedImageContainer.DecompressImage();
}

bool mitk::SegSliceOperation::IsValid() const
{
  return SegChangeOperationBase::IsValid() && m_PlaneGeometry.IsNotNull();
}

mitk::TimeStepType mitk::SegSliceOperation::GetTimeStep() const
{
  return this->m_TimeStep;
}

const mitk::PlaneGeometry* mitk::SegSliceOperation::GetSlicePlaneGeometry() const
{
  return this->m_PlaneGeometry;
}

mitk::MultiLabelSegmentation::GroupIndexType mitk::SegSliceOperation::GetGroupID() const
{
  return m_GroupID;
}

