/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImageCast.txx>
#include <mitkInstantiateAccessFunctions.h>

namespace mitk
{
#define InstantiateAccessFunction_CastToItkImage(pixelType, dim)                                                       \
                                                                                                                       \
  template MITKCORE_EXPORT void CastToItkImage(const mitk::Image *,                                                    \
                                               itk::SmartPointer<ImageTypeTrait<pixelType, dim>::ImageType> &);

  InstantiateAccessFunctionForFixedType(CastToItkImage,
                                        MITK_ACCESSBYITK_PIXEL_TYPES_SEQ MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ,
                                        MITK_ACCESSBYITK_DIMENSIONS_SEQ)
}
