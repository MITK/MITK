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
#include <itkVector.h>
#include <itkDiffusionTensor3D.h>

namespace mitk
{

typedef itk::Image<itk::RGBPixel<unsigned char>, 2>  itkImageRGBUC2;
typedef itk::Image<itk::DiffusionTensor3D<float>, 2>  itkImageDTIF2;
typedef itk::Image<itk::DiffusionTensor3D<double>, 2>  itkImageDTID2;

template void MITKCORE_EXPORT _CastToItkImage2Access(const itkImageRGBUC2*,  itk::SmartPointer<itkImageRGBUC2>&);
template void MITKCORE_EXPORT _CastToItkImage2Access(const itkImageDTIF2*,  itk::SmartPointer<itkImageDTIF2>&);
template void MITKCORE_EXPORT _CastToItkImage2Access(const itkImageDTID2*,  itk::SmartPointer<itkImageDTID2>&);

typedef itk::Image<itk::RGBPixel<unsigned char>, 3>  itkImageRGBUC3;
typedef itk::Image<itk::DiffusionTensor3D<float>, 3>  itkImageDTIF3;
typedef itk::Image<itk::DiffusionTensor3D<double>, 3>  itkImageDTID3;

template void MITKCORE_EXPORT _CastToItkImage2Access(const itkImageRGBUC3*,  itk::SmartPointer<itkImageRGBUC3>&);
template void MITKCORE_EXPORT _CastToItkImage2Access(const itkImageDTIF3*,  itk::SmartPointer<itkImageDTIF3>&);
template void MITKCORE_EXPORT _CastToItkImage2Access(const itkImageDTID3*,  itk::SmartPointer<itkImageDTID3>&);

#define CAST_HUNDRED_VECS(HUN)   \
  CAST_TEN_VECS(HUN)              \
  CAST_TEN_VECS(HUN+10)              \
  CAST_TEN_VECS(HUN+20)              \
  CAST_TEN_VECS(HUN+30)              \
  CAST_TEN_VECS(HUN+40)              \
  CAST_TEN_VECS(HUN+50)              \
  CAST_TEN_VECS(HUN+60)              \
  CAST_TEN_VECS(HUN+70)              \
  CAST_TEN_VECS(HUN+80)              \
  CAST_TEN_VECS(HUN+90)              \

#define CAST_TEN_VECS(TEN)   \
  CAST_N_VEC(TEN+ 1)              \
  CAST_N_VEC(TEN+ 2)              \
  CAST_N_VEC(TEN+ 3)              \
  CAST_N_VEC(TEN+ 4)              \
  CAST_N_VEC(TEN+ 5)              \
  CAST_N_VEC(TEN+ 6)              \
  CAST_N_VEC(TEN+ 7)              \
  CAST_N_VEC(TEN+ 8)              \
  CAST_N_VEC(TEN+ 9)              \
  CAST_N_VEC(TEN+10)              \

#define CAST_N_VEC(N_DIRS)                                                                   \
  _CAST_N_VEC(N_DIRS,double)                                                                 \
  _CAST_N_VEC(N_DIRS,float)                                                                  \
  _CAST_N_VEC(N_DIRS,short)                                                                  \

#define _CAST_N_VEC(N_DIRS,PIXTYPE)                                                                         \
  template void MITKCORE_EXPORT _CastToItkImage2Access(const itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 2> *,  itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 2> >&);  \
  template void MITKCORE_EXPORT _CastToItkImage2Access(const itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 3> *,  itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 3> >&);

// the following lines allow for fixed-size vector images up to a certain size limit
// (commented out for shorter compile times)

//CAST_HUNDRED_VECS(0)
//CAST_HUNDRED_VECS(100)
//CAST_HUNDRED_VECS(200)
//CAST_HUNDRED_VECS(300)

// allow for fixed-size vectors of specific length
// (inspired by itkPointshell.cpp, precompiled q-ball configs)

//CAST_TEN_VECS(0)
//CAST_N_VEC(11)
//CAST_N_VEC(12)
CAST_N_VEC(2)
CAST_N_VEC(3)
CAST_N_VEC(6)
CAST_N_VEC(42)
CAST_N_VEC(92)
CAST_N_VEC(162)
CAST_N_VEC(252)
CAST_N_VEC(362)
CAST_N_VEC(492)
CAST_N_VEC(642)
CAST_N_VEC(812)
CAST_N_VEC(1002)

}
