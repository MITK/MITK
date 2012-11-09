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
#include <mitkImageCast.h>
#include <mitkInstantiateAccessFunctions.h>

#include <itkImage.h>
#include <itkCastImageFilter.h>

namespace mitk
{
#ifndef DOXYGEN_SKIP
template < typename TPixel, unsigned int VImageDimension, class ItkOutputImageType >
void _CastToItkImage2Access( itk::Image<TPixel, VImageDimension>* itkInputImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage)
{
  typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
  if(typeid(ItkInputImageType) == typeid(ItkOutputImageType))
  {
    itkOutputImage = reinterpret_cast<ItkOutputImageType*>(itkInputImage);
    return;
  }
  typedef itk::CastImageFilter< ItkInputImageType, ItkOutputImageType > CastImageFilterType;
  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
  castImageFilter->SetInput( itkInputImage );
  castImageFilter->Update();
  itkOutputImage = castImageFilter->GetOutput();
}
#endif //DOXYGEN_SKIP


#define InstantiateAccessFunction__CastToItkImage2Access(type1, type2) \
  template MITK_CORE_EXPORT void _CastToItkImage2Access(itk::Image<MITK_PP_TUPLE_REM(2)type1>*, itk::SmartPointer<itk::Image<MITK_PP_TUPLE_REM(2)type2> >&);

#define InstantiateCastToItkImage2Access(r, data, dim) \
  MITK_PP_SEQ_FOR_EACH_PRODUCT(InstantiateAccessFunctionProductImpl, ((_CastToItkImage2Access))(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dim))(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dim)))

MITK_PP_SEQ_FOR_EACH(InstantiateCastToItkImage2Access, _, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

}
