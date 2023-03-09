/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGrowCutSegmentationFilter_h
#define mitkGrowCutSegmentationFilter_h

#include "itkImage.h"
#include "mitkITKImageImport.h"
#include "mitkImage.h"
#include "mitkImageToImageFilter.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
    \brief A filter that performs a growcut image segmentation.

    This class being an mitk::ImageToImageFilter performs a growcut image segmentation based on a
    given seedimage.
    Internally, itk::FastGrowCut is used.

    $Author: Jan Sahrhage
  */
  class MITKSEGMENTATION_EXPORT GrowCutSegmentationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(GrowCutSegmentationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    void SetSeedImage(itk::Image<mitk::Label::PixelType, 3>::Pointer itkSeedImage) { m_itkSeedImage = itkSeedImage; }

    void SetDistancePenalty(double distancePenalty) { m_DistancePenalty = distancePenalty; }

  protected:
    GrowCutSegmentationFilter();
    ~GrowCutSegmentationFilter() override;
    void GenerateData() override;

  private:
    itk::Image<mitk::Label::PixelType, 3>::Pointer m_itkSeedImage = nullptr;
    double m_DistancePenalty;

  }; // class

} // namespace mitk

#endif
