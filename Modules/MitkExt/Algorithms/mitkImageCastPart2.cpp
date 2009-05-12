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

template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageD2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageF2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSI2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUI2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSS2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUS2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSC2>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUC2>&);

typedef itk::Image<double, 3>         itkImageD3;
typedef itk::Image<float, 3>          itkImageF3;
typedef itk::Image<int, 3>            itkImageSI3;
typedef itk::Image<unsigned int, 3>   itkImageUI3;
typedef itk::Image<short, 3>          itkImageSS3;
typedef itk::Image<unsigned short, 3> itkImageUS3;
typedef itk::Image<char, 3>           itkImageSC3;
typedef itk::Image<unsigned char, 3>  itkImageUC3;

template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageD3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageF3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSI3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUI3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSS3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUS3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageSC3>&);
template void MITK_CORE_EXPORT CastToItkImage(const mitk::Image *, itk::SmartPointer<itkImageUC3>&);

#ifndef DOXYGEN_SKIP

#endif //DOXYGEN_SKIP

}
