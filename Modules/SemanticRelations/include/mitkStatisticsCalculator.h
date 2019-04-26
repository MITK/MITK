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

#ifndef MITKSTATISTICSCALCULATOR_H
#define MITKSTATISTICSCALCULATOR_H

#include <MitkSemanticRelationsExports.h>

// mitk core
#include <mitkDataNode.h>
#include <mitkImage.h>

// itk
#include <itkImage.h>

/*
* @brief Provides helper functions that are needed to work with lesions.
*
*   These functions help to generate new lesions, check for existing lesions or provide functionality
*   to find existing lesion class types.
*/
namespace mitk
{
  class MITKSEMANTICRELATIONS_EXPORT StatisticsCalculator
  {

  public:

    double GetSegmentationMaskVolume(mitk::DataNode::Pointer segmentationNode);

  private:

    template <typename TPixel, unsigned int VImageDimension>
    void InternalGetSegmentationMaskVolume(typename itk::Image <TPixel, VImageDimension>* segmentation);

    template <typename TPixel, unsigned int VImageDimension>
    double GetVoxelVolume(typename itk::Image<TPixel, VImageDimension>* image) const;

    Image::Pointer m_Segmentation;
    double m_MaskVolume;

  };

} // namespace mitk

#endif // MITKSTATISTICSCALCULATOR_H
