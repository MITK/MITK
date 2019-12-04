/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKBOUNDINGOBJECTCUTANDCAST_TXX
#define MITKBOUNDINGOBJECTCUTANDCAST_TXX

#include "mitkBoundingObjectCutAndCast.h"
#include "mitkImage.h"
#include "mitkImageAccessByItk.h"

namespace mitk
{
  template <typename TPixel>
  BoundingObjectCutAndCast<TPixel>::BoundingObjectCutAndCast()
  {
  }

  template <typename TPixel>
  BoundingObjectCutAndCast<TPixel>::~BoundingObjectCutAndCast()
  {
  }

  template <typename TPixel>
  const std::type_info &BoundingObjectCutAndCast<TPixel>::GetOutputPixelType()
  {
    return typeid(TPixel);
  }

  template <typename TInputPixelType, unsigned int VImageDimension, class TBoundingObjectCutAndCastType>
  void CutImageFixedOutputType(itk::Image<TInputPixelType, VImageDimension> *inputItkImage,
                               TBoundingObjectCutAndCastType *cutter,
                               int boTimeStep)
  {
    CutImageWithOutputTypeSelect<TInputPixelType, VImageDimension, typename TBoundingObjectCutAndCastType::PixelType>(
      inputItkImage, cutter, boTimeStep);
  }

  template <typename TPixel>
  void BoundingObjectCutAndCast<TPixel>::ComputeData(mitk::Image *input3D, int boTimeStep)
  {
    AccessFixedDimensionByItk_2(input3D, CutImageFixedOutputType, 3, this, boTimeStep);
  }

} // of namespace mitk
#endif // of MITKBOUNDINGOBJECTCUTANDCAST_TXX
