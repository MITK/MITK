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

#include <mitkConfig.h>
#include <mitkImageCast.txx>
#include <mitkInstantiateAccessFunctions.h>

namespace mitk {


#define InstantiateAccessFunction__CastToItkImage2Access(type1, type2) \
  template MITK_CORE_EXPORT void _CastToItkImage2Access(const ImageTypeTrait<MITK_PP_TUPLE_REM(2)type1>::ImageType*, itk::SmartPointer<ImageTypeTrait<MITK_PP_TUPLE_REM(2)type2>::ImageType>&);

#define InstantiateCastToItkImage2Access(r, data, dim) \
  MITK_PP_SEQ_FOR_EACH_PRODUCT(InstantiateAccessFunctionProductImpl, ((_CastToItkImage2Access))(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dim))(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dim))) \
  MITK_PP_SEQ_FOR_EACH_PRODUCT(InstantiateAccessFunctionProductImpl, ((_CastToItkImage2Access))(MITK_ACCESSBYITK_VECTOR_TYPES_DIMN_SEQ(dim))(MITK_ACCESSBYITK_VECTOR_TYPES_DIMN_SEQ(dim)))

MITK_PP_SEQ_FOR_EACH(InstantiateCastToItkImage2Access, _, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

}
