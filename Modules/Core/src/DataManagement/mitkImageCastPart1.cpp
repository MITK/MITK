/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkConfig.h>
#include <mitkImageCast.txx>
#include <mitkInstantiateAccessFunctions.h>

namespace mitk
{
#define InstantiateAccessFunction__CastToItkImage2Access(type1, type2)                                                 \
  template MITKCORE_EXPORT void _CastToItkImage2Access(                                                                \
    const ImageTypeTrait<BOOST_PP_TUPLE_REM(2) type1>::ImageType *,                                                     \
    itk::SmartPointer<ImageTypeTrait<BOOST_PP_TUPLE_REM(2) type2>::ImageType> &);

#define InstantiateCastToItkImage2Access(r, data, dim)                                                                 \
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(                                                                                        \
    InstantiateAccessFunctionProductImpl,                                                                              \
    ((_CastToItkImage2Access))(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dim))(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dim)))            \
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(InstantiateAccessFunctionProductImpl,                                                   \
                               ((_CastToItkImage2Access))(MITK_ACCESSBYITK_VECTOR_TYPES_DIMN_SEQ(dim))(                \
                                 MITK_ACCESSBYITK_VECTOR_TYPES_DIMN_SEQ(dim)))

  BOOST_PP_SEQ_FOR_EACH(InstantiateCastToItkImage2Access, _, MITK_ACCESSBYITK_DIMENSIONS_SEQ)
}
