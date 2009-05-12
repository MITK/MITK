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


#ifndef MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
#define MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
 
#include <itkCastImageFilter.h>
#include <mitkImageToItk.h>

// File created using Visual Studio by replacing regular expression
// itkImgFunc\(itk\:\:Image\<(.*),(.*)\> 
// with 
// itkImgFunc<\1, \2>(itk::Image<\1,\2>
// from mitkAltInstantiateAccessFunction.h

//--------------------------------- instantiation functions  ------------------------------
//##Documentation
//## @brief Instantiate access function without additional parammeters
//## for all datatypes and dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file. The CMake macro
//## MITK_MULTIPLEX_PICTYPE can help you with that. See \c mitk/CMake/CoreHelpers.cmake
//## for documentation.
//## 
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunction_1
//## \sa InstantiateAccessFunction_2
//## @ingroup Adaptor
#define InstantiateAccessFunction(itkImgFunc)                       \
  InstantiateAccessFunctionForFixedDimension(itkImgFunc, 2)         \
  InstantiateAccessFunctionForFixedDimension(itkImgFunc, 3)         

//##Documentation
//## @brief Instantiate access function with one additional parammeter
//## for all datatypes and dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunction
//## \sa InstantiateAccessFunction_2
//## @ingroup Adaptor
#define InstantiateAccessFunction_1(itkImgFunc, param1Type)                       \
  InstantiateAccessFunctionForFixedDimension_1(itkImgFunc, 2, param1Type)         \
  InstantiateAccessFunctionForFixedDimension_1(itkImgFunc, 3, param1Type)         

//##Documentation
//## @brief Instantiate access function with two additional parammeters
//## for all datatypes and dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \sa InstantiateAccessFunction
//## \sa InstantiateAccessFunction_1
//## @ingroup Adaptor
#define InstantiateAccessFunction_2(itkImgFunc, param1Type, param2Type)                \
  InstantiateAccessFunctionForFixedDimension_2(itkImgFunc, 2, param1Type, param2Type)  \
  InstantiateAccessFunctionForFixedDimension_2(itkImgFunc, 3, param1Type, param2Type)

//##Documentation
//## @brief Instantiate access function without additional parammeters
//## for all datatypes, but fixed dimension
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunctionForFixedDimension_1
//## \sa InstantiateAccessFunctionForFixedDimension_2
//## @ingroup Adaptor
#define InstantiateAccessFunctionForFixedDimension(itkImgFunc, dimension)                       \
template void itkImgFunc<double,  dimension>(itk::Image<double, dimension>*);                   \
template void itkImgFunc<float,  dimension>(itk::Image<float, dimension>*);                     \
template void itkImgFunc<int,  dimension>(itk::Image<int, dimension>*);                         \
template void itkImgFunc<unsigned int,  dimension>(itk::Image<unsigned int, dimension>*);       \
template void itkImgFunc<short,  dimension>(itk::Image<short, dimension>*);                     \
template void itkImgFunc<unsigned short,  dimension>(itk::Image<unsigned short, dimension>*);   \
template void itkImgFunc<char,  dimension>(itk::Image<char, dimension>*);                       \
template void itkImgFunc<unsigned char,  dimension>(itk::Image<unsigned char, dimension>*);

//##Documentation
//## @brief Instantiate access function with one additional parammeter
//## for all datatypes, but fixed dimension
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunctionForFixedDimension
//## \sa InstantiateAccessFunctionForFixedDimension_2
//## @ingroup Adaptor
#define InstantiateAccessFunctionForFixedDimension_1(itkImgFunc, dimension, param1Type)                   \
template void itkImgFunc<double,  dimension>(itk::Image<double, dimension>*, param1Type);                 \
template void itkImgFunc<float,  dimension>(itk::Image<float, dimension>*, param1Type);                   \
template void itkImgFunc<int,  dimension>(itk::Image<int, dimension>*, param1Type);                       \
template void itkImgFunc<unsigned int,  dimension>(itk::Image<unsigned int, dimension>*, param1Type);     \
template void itkImgFunc<short,  dimension>(itk::Image<short, dimension>*, param1Type);                   \
template void itkImgFunc<unsigned short,  dimension>(itk::Image<unsigned short, dimension>*, param1Type); \
template void itkImgFunc<char,  dimension>(itk::Image<char, dimension>*, param1Type);                     \
template void itkImgFunc<unsigned char,  dimension>(itk::Image<unsigned char, dimension>*, param1Type);

//##Documentation
//## @brief Instantiate access function with two additional parammeters
//## for all datatypes, but fixed dimension
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunctionForFixedDimension
//## \sa InstantiateAccessFunctionForFixedDimension_1
//## @ingroup Adaptor
#define InstantiateAccessFunctionForFixedDimension_2(itkImgFunc, dimension, param1Type, param2Type)                   \
template void itkImgFunc<double,  dimension>(itk::Image<double, dimension>*, param1Type, param2Type);                 \
template void itkImgFunc<float,  dimension>(itk::Image<float, dimension>*, param1Type, param2Type);                   \
template void itkImgFunc<int,  dimension>(itk::Image<int, dimension>*, param1Type, param2Type);                       \
template void itkImgFunc<unsigned int,  dimension>(itk::Image<unsigned int, dimension>*, param1Type, param2Type);     \
template void itkImgFunc<short,  dimension>(itk::Image<short, dimension>*, param1Type, param2Type);                   \
template void itkImgFunc<unsigned short,  dimension>(itk::Image<unsigned short, dimension>*, param1Type, param2Type); \
template void itkImgFunc<char,  dimension>(itk::Image<char, dimension>*, param1Type, param2Type);                     \
template void itkImgFunc<unsigned char,  dimension>(itk::Image<unsigned char, dimension>*, param1Type, param2Type);

//##Documentation
//## @brief Instantiate access function without additional parammeters
//## for a fixed datatype, but all dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunctionForFixedPixelType_1
//## \sa InstantiateAccessFunctionForFixedPixelType_2
//## @ingroup Adaptor
#define InstantiateAccessFunctionForFixedPixelType(itkImgFunc, pixelType)                          \
template void itkImgFunc<pixelType,  2>(itk::Image<pixelType, 2>*);                                \
template void itkImgFunc<pixelType,  3>(itk::Image<pixelType, 3>*);

//##Documentation
//## @brief Instantiate access function with one additional parammeter
//## for a fixed datatype, but all dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunctionForFixedPixelType
//## \sa InstantiateAccessFunctionForFixedPixelType_2
//## @ingroup Adaptor
#define InstantiateAccessFunctionForFixedPixelType_1(itkImgFunc, pixelType, param1Type)            \
template void itkImgFunc<pixelType,  2>(itk::Image<pixelType, 2>*, param1Type);                    \
template void itkImgFunc<pixelType,  3>(itk::Image<pixelType, 3>*, param1Type);

//##Documentation
//## @brief Instantiate access function with two additional parammeters
//## for a fixed datatype, but all dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file.
//## \note If you experience compiler errors, try to
//## include mitkAltInstantiateAccessFunctions.h and
//## add "Alt" (for alternate) before the call
//## (e.g. AltInstantiateAccessFunction(...)).
//## \sa InstantiateAccessFunctionForFixedPixelType
//## \sa InstantiateAccessFunctionForFixedPixelType_1
//## @ingroup Adaptor
#define InstantiateAccessFunctionForFixedPixelType_2(itkImgFunc, pixelType, param1Type, param2Type)\
template void itkImgFunc<pixelType,  2>(itk::Image<pixelType, 2>*, param1Type, param2Type);        \
template void itkImgFunc<pixelType,  3>(itk::Image<pixelType, 3>*, param1Type, param2Type);

#endif // of MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
