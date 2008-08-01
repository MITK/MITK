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

typedef itk::Image<double, 3>         itkImageD3;
typedef itk::Image<float, 3>          itkImageF3;
typedef itk::Image<int, 3>            itkImageSI3;
typedef itk::Image<unsigned int, 3>   itkImageUI3;
typedef itk::Image<short, 3>          itkImageSS3;
typedef itk::Image<unsigned short, 3> itkImageUS3;
typedef itk::Image<char, 3>           itkImageSC3;
typedef itk::Image<unsigned char, 3>  itkImageUC3;
typedef itk::Image<itk::RGBPixel<unsigned char>, 3>  itkImageRGBUC3;

InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageD2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageF2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageSI2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageUI2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageSS2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageUS2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageSC2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageUC2>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 2, itk::SmartPointer<itkImageRGBUC2>&);

InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageD3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageF3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageSI3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageUI3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageSS3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageUS3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageSC3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageUC3>&);
InstantiateAccessFunctionForFixedDimension_1(_CastToItkImage2Access, 3, itk::SmartPointer<itkImageRGBUC3>&);
}
