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


#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
#include <mitkInstantiateAccessFunctions.h>

#include <itkImage.h>
#include <itkCastImageFilter.h>

namespace mitk
{

#ifndef DOXYGEN_SKIP
  template <typename TPixelType, unsigned int VImageDimension>
  void CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<TPixelType, VImageDimension> >& itkOutputImage)
  {
    AccessFixedDimensionByItk_1(mitkImage, _CastToItkImage2Access, VImageDimension, itkOutputImage);
  }

  template <typename TPixelType, unsigned int VImageDimension>
  void CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::VectorImage<TPixelType, VImageDimension> >& itkOutputImage)
  {
    AccessVectorFixedDimensionByItk_n(mitkImage, _CastToItkImage2Access, VImageDimension, (itkOutputImage));
  }
#endif //DOXYGEN_SKIP

#define InstantiateAccessFunction_CastToItkImage(pixelType, dim) \
template MITK_CORE_EXPORT void CastToItkImage(const mitk::Image *, itk::SmartPointer<ImageTypeTrait<pixelType,dim>::ImageType>&);

InstantiateAccessFunctionForFixedType(CastToItkImage, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

}
