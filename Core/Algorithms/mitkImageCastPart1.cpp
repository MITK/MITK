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

InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageD2>& );
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageF2>& );
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageSI2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageUI2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageSS2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageUS2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageSC2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageUC2>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageRGBUC2*,  itk::SmartPointer<itkImageRGBUC2>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageDTIF2*,  itk::SmartPointer<itkImageDTIF2>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageDTID2*,  itk::SmartPointer<itkImageDTID2>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageCDTIF2*,  itk::SmartPointer<itkImageCDTIF2>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageCDTID2*,  itk::SmartPointer<itkImageCDTID2>&);


InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageD3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageF3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageSI3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageUI3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageSS3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageUS3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageSC3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageUC3>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageRGBUC3*,  itk::SmartPointer<itkImageRGBUC3>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageDTIF3*,  itk::SmartPointer<itkImageDTIF3>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageDTID3*,  itk::SmartPointer<itkImageDTID3>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageCDTIF3*,  itk::SmartPointer<itkImageCDTIF3>&);
template void MITK_CORE_EXPORT _CastToItkImage2Access(itkImageCDTID3*,  itk::SmartPointer<itkImageCDTID3>&);

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

#define _CAST_N_VEC(N_DIRS,PIXTYPE)                                                                         \
  template void MITK_CORE_EXPORT _CastToItkImage2Access(itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 2> *,  itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 2> >&);  \
  template void MITK_CORE_EXPORT _CastToItkImage2Access(itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 3> *,  itk::SmartPointer<itk::Image<itk::Vector<PIXTYPE,N_DIRS>, 3> >&);  \

// the following lines allow for fixed-size vector images up to a certain size limit
// (commented out for shorter compile times)
//CAST_HUNDRED_VECS(0)
//CAST_HUNDRED_VECS(100)
//CAST_HUNDRED_VECS(200)
//CAST_HUNDRED_VECS(300)

// allow for fixed-size vectors of specific length
// (inspired by itkPointshell.cpp, precompiled q-ball configs)
CAST_TEN_VECS(0)
CAST_N_VEC(11)
CAST_N_VEC(12)
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
