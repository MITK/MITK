/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelMaskGenerator.h>
#include <mitkImageTimeSelector.h>

unsigned int mitk::MultiLabelMaskGenerator::GetNumberOfMasks() const
{
  if (m_MultiLabelSegmentation.IsNull()) mitkThrow() << "Invalid state. Cannot get number of masks. MultiLabelSegmentation is not set.";
  return m_MultiLabelSegmentation->GetNumberOfGroups();
}

mitk::Image::ConstPointer mitk::MultiLabelMaskGenerator::DoGetMask(unsigned int maskID)
{
  if (m_MultiLabelSegmentation.IsNull()) mitkThrow() << "Invalid state. Cannot get number of masks. MultiLabelSegmentation is not set.";

  auto groupImage = m_MultiLabelSegmentation->GetGroupImage(maskID);
  return SelectImageByTimePoint(groupImage, m_TimePoint);
}
