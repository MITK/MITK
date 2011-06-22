/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include <mitkImageCast.h>
#include <itkImage.h>
#include <itkCastImageFilter.h>

namespace mitk 
{

#ifndef DOXYGEN_SKIP
  template<typename ItkOutputImageType>
  struct CastToItkImageFunctor
  {
    typedef CastToItkImageFunctor Self;

    void operator()(const mitk::Image* mitkImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage)
    {
      AccessFixedDimensionDefaultPixelTypesByItk_1(mitkImage, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itk::SmartPointer<ItkOutputImageType>&, itkOutputImage);
    }

    template < typename TPixel, unsigned int VImageDimension>
    void AccessItkImage( itk::Image<TPixel, VImageDimension>* itkInputImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage)
    {
      _CastToItkImage2Access(itkInputImage, itkOutputImage);
    }
  };

  template <typename ItkOutputImageType> void CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage)
  {
    CastToItkImageFunctor<ItkOutputImageType>()(mitkImage, itkOutputImage);
  }
#endif //DOXYGEN_SKIP

#define InstantiateAccessFunction_CastToItkImage(pixelType, dim) \
template MITK_CORE_EXPORT void CastToItkImage(const mitk::Image *, itk::SmartPointer<itk::Image<pixelType,dim> >&);

InstantiateAccessFunction(CastToItkImage)

}
