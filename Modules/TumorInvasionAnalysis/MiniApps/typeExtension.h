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

// MITK
#include <mitkPixelType.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkImageAccessByItk.h>

// ITK
#include <itkRGBPixel.h>


namespace mitk {

typedef itk::Image<itk::RGBPixel<unsigned short>, 3>  itkImageRGBUS3;

template void  _CastToItkImage2Access(const itkImageRGBUS3*,  itk::SmartPointer<itkImageRGBUS3>&);

template < typename TPixel, unsigned int VImageDimension, class ItkOutputImageType >
void _CastToItkImage2Access(const itk::Image<TPixel, VImageDimension>* itkInputImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage)
{
  typedef itk::Image<TPixel, VImageDimension> ItkInputImageType;
  if(typeid(ItkInputImageType) == typeid(ItkOutputImageType))
  {
    itkOutputImage = const_cast<ItkOutputImageType*>(reinterpret_cast<const ItkOutputImageType*>(itkInputImage));
    return;
  }
  typedef itk::CastImageFilter< ItkInputImageType, ItkOutputImageType > CastImageFilterType;
  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
  castImageFilter->SetInput( itkInputImage );
  castImageFilter->Update();
  itkOutputImage = castImageFilter->GetOutput();
}


template <> void  CastToItkImage<itkImageRGBUS3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUS3>& itkOutputImage)
{
  typedef itkImageRGBUS3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (itk::RGBPixel<unsigned short>), (ItkOutputImageType::ImageDimension), itkOutputImage);
}


}
