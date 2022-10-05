/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "itkFastGrowCut.h"
#include "mitkStringProperty.h"
#include "mitkLabel.h"
#include "mitkLabelSetImage.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkGrowCutSegmentationFilter.h"

template <typename TPixel, unsigned int VImageDimension>
void AccessItkGrowCutFilter(const itk::Image<TPixel, VImageDimension> *inputImage,
                            const itk::Image<mitk::Label::PixelType, VImageDimension> *seedImage,
                            const double distancePenalty,
                            mitk::Image::Pointer output)
{
  using ImageType = itk::Image<TPixel, VImageDimension>;
  using LabelType = itk::Image<mitk::Label::PixelType, VImageDimension>;

  using FGCType = itk::FastGrowCut<ImageType, LabelType>;
  typename FGCType::Pointer fgcFilter = FGCType::New();

  fgcFilter->SetInput(inputImage);
  fgcFilter->SetSeedImage(seedImage);
  fgcFilter->SetDistancePenalty(distancePenalty);

  try
  {
    fgcFilter->Update();
  }
  catch (...)
  {
    mitkThrow() << "An error occurred while using the itkFastGrowCut filter Update()-Method.";
  }

  mitk::CastToMitkImage<LabelType>(fgcFilter->GetOutput(), output);
  return;
}

namespace mitk
{
  GrowCutSegmentationFilter::GrowCutSegmentationFilter() : m_DistancePenalty(0)
  {
  }

  GrowCutSegmentationFilter::~GrowCutSegmentationFilter() {}

  void GrowCutSegmentationFilter::GenerateData()
  {
    if (nullptr == m_itkSeedImage)
    {
      return;
    }

    mitk::Image::ConstPointer mitkInputImage = GetInput();

    AccessFixedDimensionByItk_n(mitkInputImage,
                                AccessItkGrowCutFilter,
                                3,
                                (m_itkSeedImage.GetPointer(),
                                 m_DistancePenalty,
                                 this->GetOutput()));
  }
} // namespace mitk
