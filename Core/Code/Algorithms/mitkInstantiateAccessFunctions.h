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
//## @deprecated Use InstantiateAccessItkImageFunction instead
//## @brief Instantiate access function without additional parameters
//## for all datatypes and dimensions
//##
//## Use this macro once after the definition of your access function.
//## Some compilers have memory problems without the explicit instantiation.
//## You may need to move the access function to a separate file. The CMake macro
//## MITK_MULTIPLEX_PICTYPE can help you with that. See \c mitk/CMake/mitkMacroMultiplexPicType.cmake
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

/**
 * \brief Instantiate access function without additional parameters
 *        for all datatypes listed MITK_ACCESSBYITK_PIXEL_TYPES and dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunction(className)                \
  InstantiateAccessItkImageFunctionForFixedDimension(className, 2)  \
  InstantiateAccessItkImageFunctionForFixedDimension(itkImgFunc, 3)

//##Documentation
//## @deprecated Use InstantiateAccessItkImageFunction_1 instead
//## @brief Instantiate access function with one additional parameter
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

/**
 * \brief Instantiate access function with one additional parameter
 *        for all datatypes listed MITK_ACCESSBYITK_PIXEL_TYPES and dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunction_1(className, param1Type, param1)                \
  InstantiateAccessItkImageFunctionForFixedDimension_1(className, 2, param1Type, param1)  \
  InstantiateAccessItkImageFunctionForFixedDimension_1(className, 3, param1Type, param1)

//##Documentation
//## @deprecated Use InstantiateAccessItkImageFunction_2 instead
//## @brief Instantiate access function with two additional parameters
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

/**
 * \brief Instantiate access function with two additional parameters
 *        for all datatypes listed MITK_ACCESSBYITK_PIXEL_TYPES and dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunction_2(className, param1Type, param1, param2Type, param2) \
  InstantiateAccessItkImageFunctionForFixedDimension_1(className, 2, param1Type, param1, param2Type, param2) \
  InstantiateAccessItkImageFunctionForFixedDimension_1(className, 3, param1Type, param1, param2Type, param2)

//##Documentation
//## @deprecated Use InstantiateAccessItkImageFunctionForFixedDimension
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

/**
 * \brief Instantiate access function without additional parameters
 *        for all datatypes listed MITK_ACCESSBYITK_PIXEL_TYPES and a fixed dimension.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForFixedDimension(className, dimension)                \
namespace {                                                                                     \
void instantiate_ ## className ## dimension ()                                                  \
{                                                                                               \
  typedef mitk::PixelTypeList<MITK_ACCESSBYITK_PIXEL_TYPES> MyTypes;                            \
  mitk::PixelTypeSwitch<MyTypes> typeSwitch;                                                    \
  mitk::AccessItkImageFunctor<className,dimension> memberFunctor(0, 0);                         \
  int typeCount = mitk::PixelTypeLength<MyTypes>::value;                                        \
  for(int i = 0; i < typeCount; ++i)                                                            \
  {                                                                                             \
    typeSwitch(i, memberFunctor);                                                               \
  }                                                                                             \
}

//##Documentation
//## @deprecated UseInstantiateAccessFunctionForFixedDimension_1 instead
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

/**
 * \brief Instantiate access function with one additional parameters
 *        for all datatypes listed MITK_ACCESSBYITK_PIXEL_TYPES and a fixed dimension.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForFixedDimension_1(className, dimension, param1Type, param1)    \
namespace {                                                                                               \
void instantiate1_ ## className ## dimension ()                                                           \
{                                                                                                         \
  typedef mitk::PixelTypeList<MITK_ACCESSBYITK_PIXEL_TYPES> MyTypes;                                      \
  mitk::PixelTypeSwitch<MyTypes> typeSwitch;                                                              \
  mitk::AccessItkImageFunctor<className,dimension,param1Type> memberFunctor(0, 0, param1);                \
  int typeCount = mitk::PixelTypeLength<MyTypes>::value;                                                  \
  for(int i = 0; i < typeCount; ++i)                                                                      \
  {                                                                                                       \
    typeSwitch(i, memberFunctor);                                                                         \
  }                                                                                                       \
}                                                                                                         \
}

//##Documentation
//## @deprecated Use InstantiateAccessItkImageFunctionForFixedDimension_2 instead
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

/**
 * \brief Instantiate access function with two additional parameters
 *        for all datatypes listed MITK_ACCESSBYITK_PIXEL_TYPES and a fixed dimension.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForFixedDimension_2(className, dimension, param1Type, param1, param2Type, param2) \
namespace {                                                                                                   \
void instantiate2_ ## className ## dimension ()                                                               \
{                                                                                                             \
  typedef mitk::PixelTypeList<MITK_ACCESSBYITK_PIXEL_TYPES> MyTypes;                                          \
  mitk::PixelTypeSwitch<MyTypes> typeSwitch;                                                                  \
  mitk::AccessItkImageFunctor<className,dimension,param1Type,param2Type> memberFunctor(0, 0, param1, param2); \
  int typeCount = mitk::PixelTypeLength<MyTypes>::value;                                                      \
  for(int i = 0; i < typeCount; ++i)                                                                          \
  {                                                                                                           \
    typeSwitch(i, memberFunctor);                                                                             \
  }                                                                                                           \
}                                                                                                             \
}

//##Documentation
//## @deprecated Use InstantiateAccessItkImageFunctionForFixedPixelType instead
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

/**
 * \brief Instantiate access function without additional parameters
 *        for all listed datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForSpecificPixelTypes(className, ...)                                \
namespace {                                                                                                   \
void instantiate_ ## className ## Types ()                                                                    \
{                                                                                                             \
  typedef mitk::PixelTypeList<__VA_ARGS__> MyTypes;                                                           \
  mitk::PixelTypeSwitch<MyTypes> typeSwitch;                                                                  \
  mitk::AccessItkImageFunctor<className,2> memberFunctor2(0, 0);                                              \
  mitk::AccessItkImageFunctor<className,3> memberFunctor3(0, 0);                                              \
  int typeCount = mitk::PixelTypeLength<MyTypes>::value;                                                      \
  for(int i = 0; i < typeCount; ++i)                                                                          \
  {                                                                                                           \
    typeSwitch(i, memberFunctor2);                                                                            \
    typeSwitch(i, memberFunctor3);                                                                            \
  }                                                                                                           \
}                                                                                                             \
}

/**
 * \brief Instantiate access function without additional parameters
 *        for integral datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForIntegralPixelTypes(className)                                \
InstantiateAccessItkImageFunctionForSpecificPixelTypes(className, int, unsigned int, short, unsigned short, char, unsigned char)

/**
 * \brief Instantiate access function without additional parameters
 *        for floating point datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForFloatingPixelTypes(className)                                \
InstantiateAccessItkImageFunctionForSpecificPixelTypes(className, float, double)

//##Documentation
//## @deprecated Use InstantiateAccessItkImageFunctionForFixedPixelType_1
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

/**
 * \brief Instantiate access function with one additional parameter
 *        for all listed datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForSpecificPixelTypes_1(className, param1Type, param1, ...)          \
namespace {                                                                                                   \
void instantiate1_ ## className ## Types ()                                                                   \
{                                                                                                             \
  typedef mitk::PixelTypeList<__VA_ARGS__> MyTypes;                                                           \
  mitk::PixelTypeSwitch<MyTypes> typeSwitch;                                                                  \
  mitk::AccessItkImageFunctor<className,2,param1Type> memberFunctor2(0, 0, param1);                           \
  mitk::AccessItkImageFunctor<className,3,param1Type> memberFunctor3(0, 0, param1);                           \
  int typeCount = mitk::PixelTypeLength<MyTypes>::value;                                                      \
  for(int i = 0; i < typeCount; ++i)                                                                          \
  {                                                                                                           \
    typeSwitch(i, memberFunctor2);                                                                            \
    typeSwitch(i, memberFunctor3);                                                                            \
  }                                                                                                           \
}                                                                                                             \
}

/**
 * \brief Instantiate access function with one additional parameter
 *        for integral datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForIntegralPixelTypes_1(className, param1Type, param1) \
InstantiateAccessItkImageFunctionForSpecificPixelTypes_1(className, param1Type, param1, int, unsigned int, short, unsigned short, char, unsigned char)

/**
 * \brief Instantiate access function with one additional parameter
 *        for floating point datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForFloatingPixelTypes_1(className, param1Type, param1) \
InstantiateAccessItkImageFunctionForSpecificPixelTypes_1(className, param1Type, param1, float, double)

//##Documentation
//## @deprecated Use InstantiateAccessItkImageFunctionForSpecificPixelTypes_2 instead
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

/**
 * \brief Instantiate access function with two additional parameters
 *        for all listed datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForSpecificPixelTypes_2(className, param1Type, param1, param2Type, param2, ...) \
namespace {                                                                                                   \
void instantiate2_ ## className ## Types ()                                                                   \
{                                                                                                             \
  typedef mitk::PixelTypeList<__VA_ARGS__> MyTypes;                                                           \
  mitk::PixelTypeSwitch<MyTypes> typeSwitch;                                                                  \
  mitk::AccessItkImageFunctor<className,2,param1Type,param2Type> memberFunctor2(0, 0, param1, param2);        \
  mitk::AccessItkImageFunctor<className,3,param1Type,param2Type> memberFunctor3(0, 0, param1, param2);        \
  int typeCount = mitk::PixelTypeLength<MyTypes>::value;                                                      \
  for(int i = 0; i < typeCount; ++i)                                                                          \
  {                                                                                                           \
    typeSwitch(i, memberFunctor2);                                                                            \
    typeSwitch(i, memberFunctor3);                                                                            \
  }                                                                                                           \
}                                                                                                             \
}

/**
 * \brief Instantiate access function with two additional parameters
 *        for integral datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForIntegralPixelTypes_2(className, param1Type, param1, param2Type, param2) \
InstantiateAccessItkImageFunctionForSpecificPixelTypes_2(className, param1Type, param1, param2Type, param2, int, unsigned int, short, unsigned short, char, unsigned char)

/**
 * \brief Instantiate access function with two additional parameters
 *        for floating point datatypes and all dimensions.
 *
 * Use this macro once after the definition of your class containing the
 * templated AccessItkImage function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the class containing the access function to a separate file.
 * The CMake macro MITK_MULTIPLEX_PICTYPE can help you with that.
 * See CMake::mitkMacroMultiplexPicType for documentation.
 */
#define InstantiateAccessItkImageFunctionForFloatingPixelTypes_2(className, param1Type, param1, param2Type, param2) \
InstantiateAccessItkImageFunctionForSpecificPixelTypes_2(className, param1Type, param1, param2Type, param2, float, double)

#endif // of MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
