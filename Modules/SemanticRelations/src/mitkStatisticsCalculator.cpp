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

// semantic relations module
#include "mitkStatisticsCalculator.h"

// mitk core module
#include <mitkImageAccessByItk.h>

double mitk::StatisticsCalculator::GetSegmentationMaskVolume(mitk::DataNode::Pointer segmentationNode)
{
  m_MaskVolume = 0.0;
  if (segmentationNode.IsNull())
  {
    return m_MaskVolume;
  }

  m_Segmentation = dynamic_cast<mitk::Image*>(segmentationNode->GetData());
  if (nullptr == m_Segmentation)
  {
    return m_MaskVolume;
  }

  AccessByItk(m_Segmentation, InternalGetSegmentationMaskVolume);

  return m_MaskVolume;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::StatisticsCalculator::InternalGetSegmentationMaskVolume(typename itk::Image <TPixel, VImageDimension>* segmentation)
{
  // simple statistics for the beginning
  auto voxelVolume = GetVoxelVolume<TPixel, VImageDimension>(segmentation);
  auto numberOfPixels = m_Segmentation->GetLargestPossibleRegion().GetNumberOfPixels();
  m_MaskVolume = static_cast<double>(numberOfPixels) * voxelVolume;
}

template <typename TPixel, unsigned int VImageDimension>
double mitk::StatisticsCalculator::GetVoxelVolume(typename itk::Image<TPixel, VImageDimension> * segmentation) const
{
  auto spacing = segmentation->GetSpacing();
  double voxelVolume = 1.0;
  for (unsigned int i = 0; i < segmentation->GetImageDimension(); ++i)
  {
    voxelVolume *= spacing[i];
  }
  return voxelVolume;
}
