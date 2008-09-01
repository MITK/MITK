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
#include "itkDiffusionTensor3D.h"

namespace mitk 
{
#ifndef DOXYGEN_SKIP
  template <typename ItkOutputImageType> void CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage)
  {
    AccessFixedDimensionByItk_1(mitkImage, _CastToItkImage2Access, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
  }
#endif //DOXYGEN_SKIP

typedef itk::Image<double, 2>         itkImageD2;
typedef itk::Image<float, 2>          itkImageF2;
typedef itk::Image<int, 2>            itkImageSI2;
typedef itk::Image<unsigned int, 2>   itkImageUI2;
typedef itk::Image<short, 2>          itkImageSS2;
typedef itk::Image<unsigned short, 2> itkImageUS2;
typedef itk::Image<char, 2>           itkImageSC2;
typedef itk::Image<unsigned char, 2>  itkImageUC2;
typedef itk::Image<itk::RGBPixel<unsigned char>, 2>  itkImageRGBUC2;
typedef itk::Image<itk::DiffusionTensor3D<float>, 2>  itkImageDTIF2;
typedef itk::Image<itk::DiffusionTensor3D<double>, 2>  itkImageDTID2;

typedef itk::Image<double, 3>         itkImageD3;
typedef itk::Image<float, 3>          itkImageF3;
typedef itk::Image<int, 3>            itkImageSI3;
typedef itk::Image<unsigned int, 3>   itkImageUI3;
typedef itk::Image<short, 3>          itkImageSS3;
typedef itk::Image<unsigned short, 3> itkImageUS3;
typedef itk::Image<char, 3>           itkImageSC3;
typedef itk::Image<unsigned char, 3>  itkImageUC3;
typedef itk::Image<itk::RGBPixel<unsigned char>, 3>  itkImageRGBUC3;
typedef itk::Image<itk::DiffusionTensor3D<float>, 3>  itkImageDTIF3;
typedef itk::Image<itk::DiffusionTensor3D<double>, 3>  itkImageDTID3;

template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageD2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageF2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSI2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUI2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSS2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUS2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSC2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUC2>&);
//template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageDTIF2>&);
//template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageDTID2>&);
template <> void CastToItkImage<itkImageRGBUC2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC2>& itkOutputImage)
{
  typedef itkImageRGBUC2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::RGBPixel<unsigned char>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void CastToItkImage<itkImageDTIF2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTIF2>& itkOutputImage)
{
  typedef itkImageDTIF2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<float>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void CastToItkImage<itkImageDTID2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTID2>& itkOutputImage)
{
  typedef itkImageRGBUC2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<double>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}

template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageD3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageF3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSI3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUI3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSS3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUS3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSC3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUC3>&);
//template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageDTIF3>&);
//template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageDTID3>&);
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageRGBUC3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC3>& itkOutputImage)
{
  typedef itkImageRGBUC3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::RGBPixel<unsigned char>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageDTIF3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTIF3>& itkOutputImage)
{
  typedef itkImageRGBUC3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<float>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageDTID3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTID3>& itkOutputImage)
{
  typedef itkImageRGBUC3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<double>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}


#ifndef DOXYGEN_SKIP
//// Extension for RGB (and maybe also for vector types)
//// Does not work yet, see comment below and bug 320
//template <typename ItkOutputImageType> void RGBCastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage)
//{
//  assert(mitkImage->GetDimension()==::itk::GetImageDimension<ItkOutputImageType>::ImageDimension);
//
//  assert(mitkImage->GetPixelType().GetNumberOfComponents() == 3);
//
//  const std::type_info& typeId=*mitkImage->GetPixelType().GetTypeId();
//
//  // Currently, the following line always fails, see bug 320
//  assert( typeId == typeid(ItkOutputImageType::PixelType) );
//
//  const mitk::Image* constImage = mitkImage;
//  const_cast<mitk::Image*>(constImage)->Update();
//
//  typedef mitk::ImageToItk<ItkOutputImageType> ImageToItkType;
//  itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();
//  imagetoitk->SetInput(mitkImage);
//  imagetoitk->Update();
//  itkOutputImage = imagetoitk->GetOutput();
//}
//
//typedef itk::Image<itk::RGBPixel<unsigned char>, 2>  itkImageRGBUC2;
//typedef itk::Image<itk::RGBPixel<unsigned char>, 3>  itkImageRGBUC3;
//
//template <> void CastToItkImage<itkImageRGBUC2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC2>& itkOutputImage)
//{
//  typedef itkImageRGBUC2 ItkOutputImageType;
//  RGBCastToItkImage<ItkOutputImageType>(mitkImage, itkOutputImage);
//}
//
//template <> void CastToItkImage<itkImageRGBUC3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC3>& itkOutputImage)
//{
//  typedef itkImageRGBUC3 ItkOutputImageType;
//  RGBCastToItkImage<ItkOutputImageType>(mitkImage, itkOutputImage);
//}
#endif //DOXYGEN_SKIP

}
