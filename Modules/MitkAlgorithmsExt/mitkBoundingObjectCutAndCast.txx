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

#ifndef MITKBOUNDINGOBJECTCUTANDCAST_TXX
#define MITKBOUNDINGOBJECTCUTANDCAST_TXX

#include "mitkBoundingObjectCutAndCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImage.h"

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
const std::type_info& BoundingObjectCutAndCast<TPixel>::GetOutputPixelType()
{
  return typeid(TPixel);
}

template < typename TInputPixelType, unsigned int VImageDimension, class TBoundingObjectCutAndCastType >
void CutImageFixedOutputType( itk::Image< TInputPixelType, VImageDimension>* inputItkImage, TBoundingObjectCutAndCastType* cutter, int boTimeStep)
{
  CutImageWithOutputTypeSelect<TInputPixelType, VImageDimension, typename TBoundingObjectCutAndCastType::PixelType>(inputItkImage, cutter, boTimeStep);
}

template <typename TPixel>
void BoundingObjectCutAndCast<TPixel>::ComputeData(mitk::Image* input3D, int boTimeStep)
{
  AccessFixedDimensionByItk_2(input3D, CutImageFixedOutputType, 3, this, boTimeStep);
}

} // of namespace mitk
#endif // of MITKBOUNDINGOBJECTCUTANDCAST_TXX
