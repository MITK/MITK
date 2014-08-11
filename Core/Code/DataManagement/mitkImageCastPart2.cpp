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


#include <mitkImageCast.txx>
#include <mitkInstantiateAccessFunctions.h>

namespace mitk
{

#define InstantiateAccessFunction_CastToItkImage(pixelType, dim) \
template MITK_CORE_EXPORT void CastToItkImage(const mitk::Image *, itk::SmartPointer<ImageTypeTrait<pixelType,dim>::ImageType>&);

InstantiateAccessFunctionForFixedType(CastToItkImage, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

}
