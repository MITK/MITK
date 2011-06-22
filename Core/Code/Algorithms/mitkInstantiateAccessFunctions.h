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
#include <mitkConfig.h>

#include <mitkPPSeqForEach.h>
#include <mitkPPSeqForEachProduct.h>
#include <mitkPPSeqToTuple.h>
#include <mitkPPCat.h>
#include <mitkPPExpand.h>
#include <mitkPPTupleRem.h>

#ifndef DOXYGEN_SKIP

#define InstantiateAccessFunctionImpl(r, itkImgFunc, type) \
  MITK_PP_CAT(InstantiateAccessFunction_, itkImgFunc) type

#ifdef _MSC_VER
#define InstantiateAccessFunctionProductImpl(r, product) \
  MITK_PP_CAT(InstantiateAccessFunction_, MITK_PP_SEQ_HEAD(product)) \
  MITK_PP_EXPAND(MITK_PP_SEQ_TO_TUPLE(MITK_PP_SEQ_TAIL(product)))
#else
#define InstantiateAccessFunctionProductImpl(r, product) \
  MITK_PP_EXPAND(MITK_PP_CAT(InstantiateAccessFunction_, MITK_PP_SEQ_HEAD(product)) \
                 MITK_PP_SEQ_TO_TUPLE(MITK_PP_SEQ_TAIL(product)))
#endif

#endif // DOXYGEN_SKIP


//--------------------------------- instantiation functions  ------------------------------

/**
 * \brief Instantiate access function for all datatypes and a specific dimension.
 *
 * This macro iteratively calls a macro named InstantiateAccessFunction_itkImgFunc
 * which you must define to instantiate your access function.
 *
 * Example:
 * \code
 * template<typename TPixelType, typename VDimension>
 * void MyImageAccessFunction(itk::Image<TPixel, VImageDimension>* itkImage)
 * { ... }
 *
 * #define InstantiateAccessFunction_MyImageAccessFunction(pixelType, dim) \\
 *   template void MyImageAccessFunction(itk::Image<pixelType,dim>*);
 *
 * InstantiateAccessFunctionForFixedDimension(MyImageAccessFunction,2)
 * \endcode
 *
 * Use this macro once after the definition of your access function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the access function to a separate file. The CMake macro
 * MITK_MULTIPLEX_PICTYPE can help you with that. See \c mitk/CMake/mitkMacroMultiplexPicType.cmake
 * for documentation.
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForFixedDimension(itkImgFunc, dim)                                        \
  MITK_PP_SEQ_FOR_EACH(InstantiateAccessFunctionImpl, itkImgFunc, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dim))

/**
 * \brief Instantiate access function for all datatypes and dimensions.
 *
 * This macro iteratively calls a macro named InstantiateAccessFunction_itkImgFunc
 * which you must define to instantiate your access function.
 *
 * Example:
 * \code
 * template<typename TPixelType, typename VDimension>
 * void MyImageAccessFunction(itk::Image<TPixel, VImageDimension>* itkImage)
 * { ... }
 *
 * #define InstantiateAccessFunction_MyImageAccessFunction(pixelType, dim) \\
 *   template void MyImageAccessFunction(itk::Image<pixelType,dim>*);
 *
 * InstantiateAccessFunction(MyImageAccessFunction)
 * \endcode
 *
 * Use this macro once after the definition of your access function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the access function to a separate file. The CMake macro
 * MITK_MULTIPLEX_PICTYPE can help you with that. See \c mitk/CMake/mitkMacroMultiplexPicType.cmake
 * for documentation.
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunction(itkImgFunc)                                                             \
  InstantiateAccessFunctionForFixedDimension(itkImgFunc, 2)                                               \
  InstantiateAccessFunctionForFixedDimension(itkImgFunc, 3)

/**
 * InstantiateAccessFunctionProduct(functionname, (seq1)(seq2)...(seqN))
 *
 * Iteratively calls a macro named InstantiateAccessFunction_functionname(type)
 * which usually explicitly instantiates a template class or function.
 *
 * A call to InstantiateAccessFunctionProduct(T, ((a)(b)) ((d)(e)) ) results in calls
 *
 *   InstantiateAccessFunction_T(a, d)
 *   InstantiateAccessFunction_T(a, e)
 *   InstantiateAccessFunction_T(b, d)
 *   InstantiateAccessFunction_T(b, e)
 *
 * That is, InstantiateAccessFunction_T is called for the cartesian product of the sequences seq1 ... seqN
 *
 * BE CAREFUL WITH YOUR PARENTHESIS!  The argument PRODUCT is a
 * sequence of sequences.
 */
#define InstantiateAccessFunctionProduct(itkImgFunc, product)                                             \
  MITK_PP_SEQ_FOR_EACH_PRODUCT(InstantiateAccessFunctionProductImpl, ((itkImgFunc))product)


/**
 * \brief Instantiate access function for a fixed pixel type and all dimensions.
 *
 * This macro iteratively calls a macro named InstantiateAccessFunction_itkImgFunc
 * which you must define to instantiate your access function.
 *
 * Example:
 * \code
 * template<typename TPixelType, typename VDimension>
 * void MyImageAccessFunction(itk::Image<TPixel, VImageDimension>* itkImage)
 * { ... }
 *
 * #define InstantiateAccessFunction_MyImageAccessFunction(pixelType, dim) \\
 *   template void MyImageAccessFunction(itk::Image<pixelType,dim>*);
 *
 * InstantiateAccessFunctionForFixedPixelType(MyImageAccessFunction, float)
 * \endcode
 *
 * Use this macro once after the definition of your access function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the access function to a separate file. The CMake macro
 * MITK_MULTIPLEX_PICTYPE can help you with that. See \c mitk/CMake/mitkMacroMultiplexPicType.cmake
 * for documentation.
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForFixedPixelType(itkImgFunc, pixelType)                              \
MITK_PP_SEQ_FOR_EACH(InstantiateAccessFunctionImpl, itkImgFunc, ((pixelType,2))((pixelType,3)))        \

/**
 * \brief Instantiate access function for a given sequence of image types.
 *
 * This macro iteratively calls a macro named InstantiateAccessFunction_itkImgFunc
 * which you must define to instantiate your access function.
 *
 * Example:
 * \code
 * template<typename TPixelType, typename VDimension>
 * void MyImageAccessFunction(itk::Image<TPixel, VImageDimension>* itkImage)
 * { ... }
 *
 * #define InstantiateAccessFunction_MyImageAccessFunction(pixelType, dim) \\
 *   template void MyImageAccessFunction(itk::Image<pixelType,dim>*);
 *
 * InstantiateAccessFunctionForSpecificTypes(MyImageAccessFunction, (short,2)(float,2)(float,3))
 * \endcode
 *
 * Use this macro once after the definition of your access function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the access function to a separate file. The CMake macro
 * MITK_MULTIPLEX_PICTYPE can help you with that. See \c mitk/CMake/mitkMacroMultiplexPicType.cmake
 * for documentation.
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForSpecificTypes(itkImgFunc, typeSeq)                            \
MITK_PP_SEQ_FOR_EACH(InstantiateAccessFunctionImpl, itkImgFunc, typeSeq)                          \

/**
 * \brief Instantiate access function for integral datatypes and all dimensions.
 *
 * \sa InstantiateAccessFunction
 */
#define InstantiateAccessFunctionForIntegralPixelTypes(itkImgFunc)                        \
InstantiateAccessFunctionForSpecificTypes(itkImgFunc, (int,2)(int,3)(unsigned int,2)(unsigned int,3)(short,2)(short,3)(unsigned short,2)(unsigned short,3)(char,2)(char,3)(unsigned char,2)(unsigned char,3))

/**
 * \brief Instantiate access function for floating point datatypes and all dimensions.
 *
 * \sa InstantiateAccessFunction
 */
#define InstantiateAccessFunctionForFloatingPixelTypes(itkImgFunc)                                \
InstantiateAccessFunctionForSpecificPixelTypes(itkImgFunc, (float,2)(float,3)(double,2)(double,3))


#endif // of MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
