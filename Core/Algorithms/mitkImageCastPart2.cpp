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
#include "itkConfidenceDiffusionTensor3D.h"

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
typedef itk::Image<itk::ConfidenceDiffusionTensor3D<float>, 2>  itkImageCDTIF2;
typedef itk::Image<itk::ConfidenceDiffusionTensor3D<double>, 2>  itkImageCDTID2;

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
typedef itk::Image<itk::ConfidenceDiffusionTensor3D<float>, 3>  itkImageCDTIF3;
typedef itk::Image<itk::ConfidenceDiffusionTensor3D<double>, 3>  itkImageCDTID3;

template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageD2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageF2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSI2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUI2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSS2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUS2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSC2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUC2>&);
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageRGBUC2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC2>& itkOutputImage)
{
  typedef itkImageRGBUC2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::RGBPixel<unsigned char>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageDTIF2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTIF2>& itkOutputImage)
{
  typedef itkImageDTIF2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<float>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageDTID2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTID2>& itkOutputImage)
{
  typedef itkImageDTID2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<double>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageCDTIF2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageCDTIF2>& itkOutputImage)
{
  typedef itkImageCDTIF2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::ConfidenceDiffusionTensor3D<float>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageCDTID2>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageCDTID2>& itkOutputImage)
{
  typedef itkImageCDTID2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::ConfidenceDiffusionTensor3D<double>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}

template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageD3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageF3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSI3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUI3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSS3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUS3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSC3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUC3>&);
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageRGBUC3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC3>& itkOutputImage)
{
  typedef itkImageRGBUC3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::RGBPixel<unsigned char>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageDTIF3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTIF3>& itkOutputImage)
{
  typedef itkImageDTIF3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<float>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageDTID3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTID3>& itkOutputImage)
{
  typedef itkImageDTID3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::DiffusionTensor3D<double>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageCDTIF3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageCDTIF3>& itkOutputImage)
{
  typedef itkImageCDTIF3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::ConfidenceDiffusionTensor3D<float>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage<itkImageCDTID3>(const mitk::Image * mitkImage, itk::SmartPointer<itkImageCDTID3>& itkOutputImage)
{
  typedef itkImageCDTID3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, itk::ConfidenceDiffusionTensor3D<double>, ::itk::GetImageDimension<ItkOutputImageType>::ImageDimension, itkOutputImage);
}

#define TYPE_VECS(HUN)        \
  TYPE_TEN_VECS(HUN)        \
  TYPE_TEN_VECS(HUN + 10)        \
  TYPE_TEN_VECS(HUN + 20)        \
  TYPE_TEN_VECS(HUN + 30)        \
  TYPE_TEN_VECS(HUN + 40)        \
  TYPE_TEN_VECS(HUN + 50)        \
  TYPE_TEN_VECS(HUN + 60)        \
  TYPE_TEN_VECS(HUN + 70)        \
  TYPE_TEN_VECS(HUN + 80)        \
  TYPE_TEN_VECS(HUN + 90)        \
  
#define TYPE_TEN_VECS(HUN)        \
  TYPE_N_VEC(HUN +  1)         \
  TYPE_N_VEC(HUN +  2)        \
  TYPE_N_VEC(HUN +  3)        \
  TYPE_N_VEC(HUN +  4)        \
  TYPE_N_VEC(HUN +  5)        \
  TYPE_N_VEC(HUN +  6)        \
  TYPE_N_VEC(HUN +  7)        \
  TYPE_N_VEC(HUN +  8)        \
  TYPE_N_VEC(HUN +  9)        \
  TYPE_N_VEC(HUN + 10)        \

#define TYPE_N_VEC(N_DIRS)                                                                   \
  _TYPE_N_VEC(N_DIRS,double)                                                                 \
  _TYPE_N_VEC(N_DIRS,float)                                                                  \

#define _TYPE_N_VEC(N_DIRS,PIXTYPE)                                                                                                                                                                \
  template <> void MITK_CORE_EXPORT CastToItkImage<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 2> >(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 2> >& itkOutputImage)                                       \
{                                                                                                                                                                                                \
  typedef itk::Vector<PIXTYPE,N_DIRS> VECTORTYPE;                                                                                                                           \
  typedef itk::Image<VECTORTYPE, 2>  ItkOutputImageType2;                                                                                                                                               \
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, VECTORTYPE, ::itk::GetImageDimension<ItkOutputImageType2>::ImageDimension, itkOutputImage);                                               \
}                                                                                                                                                                                                \
  template <> void MITK_CORE_EXPORT CastToItkImage<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 3> >(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 3> >& itkOutputImage)                                       \
{                                                                                                                                                                                                \
  typedef itk::Vector<PIXTYPE,N_DIRS> VECTORTYPE;                                                                                                                           \
  typedef itk::Image<VECTORTYPE, 3>  ItkOutputImageType3;                                                                                                                                               \
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, VECTORTYPE, ::itk::GetImageDimension<ItkOutputImageType3>::ImageDimension, itkOutputImage);                                               \
}                                                                                                                                                                                                \

// the following lines allow for fixed-size vector images up to a certain size limit
// (commented out for shorter compile times)
//TYPE_VECS(000)
//TYPE_VECS(100)
//TYPE_VECS(200)
//TYPE_VECS(300)
//TYPE_VECS(400)
//TYPE_VECS(500)
//TYPE_VECS(600)
//TYPE_VECS(700)

// allow for fixed-size vectors of specific length
// (inspired by itkPointshell.cpp, precompiled q-ball configs)
TYPE_TEN_VECS(0)
TYPE_N_VEC(11)
TYPE_N_VEC(12)
TYPE_N_VEC(42)
TYPE_N_VEC(92)
TYPE_N_VEC(162)
TYPE_N_VEC(252)
TYPE_N_VEC(362)
TYPE_N_VEC(492)
TYPE_N_VEC(642)
TYPE_N_VEC(812)
TYPE_N_VEC(1002)

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
