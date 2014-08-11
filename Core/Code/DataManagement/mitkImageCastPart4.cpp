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

#include <itkRGBPixel.h>
#include <itkDiffusionTensor3D.h>

namespace mitk
{

typedef itk::Image<itk::RGBPixel<unsigned char>, 2>  itkImageRGBUC2;
typedef itk::Image<itk::DiffusionTensor3D<float>, 2>  itkImageDTIF2;
typedef itk::Image<itk::DiffusionTensor3D<double>, 2>  itkImageDTID2;

template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC2>& itkOutputImage)
{
  typedef itkImageRGBUC2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (itk::RGBPixel<unsigned char>), (ItkOutputImageType::ImageDimension), itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTIF2>& itkOutputImage)
{
  typedef itkImageDTIF2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (itk::DiffusionTensor3D<float>), (ItkOutputImageType::ImageDimension), itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTID2>& itkOutputImage)
{
  typedef itkImageDTID2 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (itk::DiffusionTensor3D<double>), (ItkOutputImageType::ImageDimension), itkOutputImage);
}

typedef itk::Image<itk::RGBPixel<unsigned char>, 3>  itkImageRGBUC3;
typedef itk::Image<itk::DiffusionTensor3D<float>, 3>  itkImageDTIF3;
typedef itk::Image<itk::DiffusionTensor3D<double>, 3>  itkImageDTID3;

template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itkImageRGBUC3>& itkOutputImage)
{
  typedef itkImageRGBUC3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (itk::RGBPixel<unsigned char>), (ItkOutputImageType::ImageDimension), itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTIF3>& itkOutputImage)
{
  typedef itkImageDTIF3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (itk::DiffusionTensor3D<float>), (ItkOutputImageType::ImageDimension), itkOutputImage);
}
template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itkImageDTID3>& itkOutputImage)
{
  typedef itkImageDTID3 ItkOutputImageType;
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (itk::DiffusionTensor3D<double>), (ItkOutputImageType::ImageDimension), itkOutputImage);
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
  _TYPE_N_VEC(N_DIRS,short)                                                                  \

#define _TYPE_N_VEC(N_DIRS,PIXTYPE)                                                                                                                                                                \
  template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 2> >& itkOutputImage)                                       \
{                                                                                                                                                                                                \
  typedef itk::Vector<PIXTYPE,N_DIRS> VECTORTYPE;                                                                                                                           \
  typedef itk::Image<VECTORTYPE, 2>  ItkOutputImageType2;                                                                                                                                               \
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (VECTORTYPE), (ItkOutputImageType2::ImageDimension), itkOutputImage);                                               \
}                                                                                                                                                                                                \
  template <> void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 3> >& itkOutputImage)                                       \
{                                                                                                                                                                                                \
  typedef itk::Vector<PIXTYPE,N_DIRS> VECTORTYPE;                                                                                                                           \
  typedef itk::Image<VECTORTYPE, 3>  ItkOutputImageType3;                                                                                                                                               \
  AccessFixedTypeByItk_1(mitkImage, _CastToItkImage2Access, (VECTORTYPE), (ItkOutputImageType3::ImageDimension), itkOutputImage);                                               \
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

//TYPE_TEN_VECS(0)
//TYPE_N_VEC(11)
//TYPE_N_VEC(12)
TYPE_N_VEC(2)
TYPE_N_VEC(3)
TYPE_N_VEC(6)
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

#endif //DOXYGEN_SKIP

}
