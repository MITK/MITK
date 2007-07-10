/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKALTINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
#define MITKALTINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
 
#include <itkCastImageFilter.h>
#include <mitkImageToItk.h>

#ifndef DOXYGEN_SKIP
//--------------------------------- instantiation functions  ------------------------------
//##Documentation
//## @brief Instantiate access function without additional parammeters
//## for all datatypes and dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunction_1
//## \sa AltInstantiateAccessFunction_2
//## @ingroup Adaptor
#define AltInstantiateAccessFunction(itkImgFunc)                       \
  AltInstantiateAccessFunctionForFixedDimension(itkImgFunc, 2)         \
  AltInstantiateAccessFunctionForFixedDimension(itkImgFunc, 3)         

//##Documentation
//## @brief Instantiate access function with one additional parammeter
//## for all datatypes and dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunction
//## \sa AltInstantiateAccessFunction_2
//## @ingroup Adaptor
#define AltInstantiateAccessFunction_1(itkImgFunc, param1Type)                       \
  AltInstantiateAccessFunctionForFixedDimension_1(itkImgFunc, 2, param1Type)         \
  AltInstantiateAccessFunctionForFixedDimension_1(itkImgFunc, 3, param1Type)         

//##Documentation
//## @brief Instantiate access function with two additional parammeters
//## for all datatypes and dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunction
//## \sa AltInstantiateAccessFunction_1
//## @ingroup Adaptor
#define AltInstantiateAccessFunction_2(itkImgFunc, param1Type, param2Type)                \
  AltInstantiateAccessFunctionForFixedDimension_2(itkImgFunc, 2, param1Type, param2Type)  \
  AltInstantiateAccessFunctionForFixedDimension_2(itkImgFunc, 3, param1Type, param2Type)         

//##Documentation
//## @brief Instantiate access function without additional parammeters
//## for all datatypes, but fixed dimension
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunctionForFixedDimension_1
//## \sa AltInstantiateAccessFunctionForFixedDimension_2
//## @ingroup Adaptor
#define AltInstantiateAccessFunctionForFixedDimension(itkImgFunc, dimension)  \
template void itkImgFunc(itk::Image<double, dimension>*);                  \
template void itkImgFunc(itk::Image<float, dimension>*);                   \
template void itkImgFunc(itk::Image<int, dimension>*);                     \
template void itkImgFunc(itk::Image<unsigned int, dimension>*);            \
template void itkImgFunc(itk::Image<short, dimension>*);                   \
template void itkImgFunc(itk::Image<unsigned short, dimension>*);          \
template void itkImgFunc(itk::Image<char, dimension>*);                    \
template void itkImgFunc(itk::Image<unsigned char, dimension>*);           

//##Documentation
//## @brief Instantiate access function with one additional parammeter
//## for all datatypes, but fixed dimension
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunctionForFixedDimension
//## \sa AltInstantiateAccessFunctionForFixedDimension_2
//## @ingroup Adaptor
#define AltInstantiateAccessFunctionForFixedDimension_1(itkImgFunc, dimension, param1Type)\
template void itkImgFunc(itk::Image<double, dimension>*, param1Type);                  \
template void itkImgFunc(itk::Image<float, dimension>*, param1Type);                   \
template void itkImgFunc(itk::Image<int, dimension>*, param1Type);                     \
template void itkImgFunc(itk::Image<unsigned int, dimension>*, param1Type);            \
template void itkImgFunc(itk::Image<short, dimension>*, param1Type);                   \
template void itkImgFunc(itk::Image<unsigned short, dimension>*, param1Type);          \
template void itkImgFunc(itk::Image<char, dimension>*, param1Type);                    \
template void itkImgFunc(itk::Image<unsigned char, dimension>*, param1Type);           

//##Documentation
//## @brief Instantiate access function with two additional parammeters
//## for all datatypes, but fixed dimension
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunctionForFixedDimension
//## \sa AltInstantiateAccessFunctionForFixedDimension_1
//## @ingroup Adaptor
#define AltInstantiateAccessFunctionForFixedDimension_2(itkImgFunc, dimension, param1Type, param2Type)\
template void itkImgFunc(itk::Image<double, dimension>*, param1Type, param2Type);                  \
template void itkImgFunc(itk::Image<float, dimension>*, param1Type, param2Type);                   \
template void itkImgFunc(itk::Image<int, dimension>*, param1Type, param2Type);                     \
template void itkImgFunc(itk::Image<unsigned int, dimension>*, param1Type, param2Type);            \
template void itkImgFunc(itk::Image<short, dimension>*, param1Type, param2Type);                   \
template void itkImgFunc(itk::Image<unsigned short, dimension>*, param1Type, param2Type);          \
template void itkImgFunc(itk::Image<char, dimension>*, param1Type, param2Type);                    \
template void itkImgFunc(itk::Image<unsigned char, dimension>*, param1Type, param2Type);           

//##Documentation
//## @brief Instantiate access function without additional parammeters
//## for a fixed datatype, but all dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunctionForFixedPixelType_1
//## \sa AltInstantiateAccessFunctionForFixedPixelType_2
//## @ingroup Adaptor
#define AltInstantiateAccessFunctionForFixedPixelType(itkImgFunc, pixelType)                          \
template void itkImgFunc(itk::Image<pixelType, 2>*);                                               \
template void itkImgFunc(itk::Image<pixelType, 3>*);                   

//##Documentation
//## @brief Instantiate access function with one additional parammeter
//## for a fixed datatype, but all dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunctionForFixedPixelType
//## \sa AltInstantiateAccessFunctionForFixedPixelType_2
//## @ingroup Adaptor
#define AltInstantiateAccessFunctionForFixedPixelType_1(itkImgFunc, pixelType, param1Type)            \
template void itkImgFunc(itk::Image<pixelType, 2>*, param1Type);                                   \
template void itkImgFunc(itk::Image<pixelType, 3>*, param1Type);                   

//##Documentation
//## @brief Instantiate access function with two additional parammeters
//## for a fixed datatype, but all dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa AltInstantiateAccessFunctionForFixedPixelType
//## \sa AltInstantiateAccessFunctionForFixedPixelType_1
//## @ingroup Adaptor
#define AltInstantiateAccessFunctionForFixedPixelType_2(itkImgFunc, pixelType, param1Type, param2Type)\
template void itkImgFunc(itk::Image<pixelType, 2>*, param1Type, param2Type);                       \
template void itkImgFunc(itk::Image<pixelType, 3>*, param1Type, param2Type);                   

#endif DOXYGEN_SKIP

#endif // of MITKALTINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
