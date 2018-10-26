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

#ifndef MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
#define MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED

#include <itkCastImageFilter.h>
#include <mitkConfig.h>
#include <mitkImageToItk.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_product.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>

#ifndef DOXYGEN_SKIP

#define InstantiateAccessFunctionImpl(r, itkImgFunc, type) BOOST_PP_CAT(InstantiateAccessFunction_, itkImgFunc) type

// product is of the form (itkImgFunc)(short)(2)
#ifdef _MSC_VER
#define InstantiateAccessFunctionProductImpl(r, product)                                                               \
  BOOST_PP_CAT(InstantiateAccessFunction_, BOOST_PP_SEQ_HEAD(product))                                                   \
  BOOST_PP_EXPAND(BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product)))
#else
#define InstantiateAccessFunctionProductImpl(r, product)                                                               \
  BOOST_PP_EXPAND(BOOST_PP_CAT(InstantiateAccessFunction_, BOOST_PP_SEQ_HEAD(product))                                    \
                   BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product)))
#endif

#endif // DOXYGEN_SKIP

//--------------------------------- instantiation functions  ------------------------------

/**
 * \brief Instantiate access function for the given pixel types and dimensions.
 *
 * Iteratively calls a macro named InstantiateAccessFunction_itkImgFunc
 * which you must define and which usually explicitly instantiates your access function.
 *
 * A call to InstantiateAccessFunctionForFixedPixelType(T, (a)(b), (d)(e)) results in calls
 *
 *   InstantiateAccessFunction_T(a, d) <br>
 *   InstantiateAccessFunction_T(a, e) <br>
 *   InstantiateAccessFunction_T(b, d) <br>
 *   InstantiateAccessFunction_T(b, e)
 *
 * That is, InstantiateAccessFunction_T is called for the cartesian product of the sequences pixelTypeSeq
 * and dimSeq.
 *
 * Example:
 * \code
 * template<typename TPixel, typename VDimension>
 * void MyImageAccessFunction(itk::Image<TPixel, VImageDimension>* itkImage)
 * { ... }
 *
 * #define InstantiateAccessFunction_MyImageAccessFunction(pixelType, dim) \
 *   template void MyImageAccessFunction(itk::Image<pixelType,dim>*);
 *
 * InstantiateAccessFunctionForFixedPixelType(MyImageAccessFunction, (int), (3))
 * \endcode
 *
 * Use this macro once after the definition of your access function.
 * Some compilers have memory problems without the explicit instantiation.
 * You may need to move the access function to a separate file. The CMake macro
 * MITK_MULTIPLEX_PICTYPE can help you with that. See \c mitk/CMake/mitkMacroMultiplexPicType.cmake
 * for documentation.
 *
 * \param itkImgFunc The custom part of the name of the macro to be called.
 * \param pixelTypeSeq a sequence of types, like (int)(short)(char).
 * \param dimSeq a sequence of dimensions, like (2)(3).
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForFixedType(itkImgFunc, pixelTypeSeq, dimSeq)                                        \
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(InstantiateAccessFunctionProductImpl, ((itkImgFunc))(pixelTypeSeq)(dimSeq))

/**
 * \brief Instantiate access function for all datatypes and dimensions.
 *
 * \sa InstantiateAccessFunctionForFixedType
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunction(itkImgFunc)                                                                          \
  InstantiateAccessFunctionForFixedType(itkImgFunc, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

/**
 * \brief Instantiate access function for all datatypes and a specific dimension.
 *
 * \sa InstantiateAccessFunctionForFixedType
 *
 * \param itkImgFunc The custom part of the name of the macro to be called.
 * \param dim a sequence of dimensions, like (2)(3).
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForFixedDimension(itkImgFunc, dim)                                                    \
  InstantiateAccessFunctionForFixedType(itkImgFunc, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ, (dim))

/**
 * \brief Instantiate access function for all given pixel types and all dimensions.
 *
 * \sa InstantiateAccessFunctionForFixedType
 *
 * \param itkImgFunc The custom part of the name of the macro to be called.
 * \param pixelTypeSeq a sequence of types, like (int)(short)(char).
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForFixedPixelType(itkImgFunc, pixelTypeSeq)                                           \
  InstantiateAccessFunctionForFixedType(itkImgFunc, pixelTypeSeq, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

/**
 * \brief Instantiate access function for integral datatypes and all dimensions.
 *
 * \sa InstantiateAccessFunctionForFixedType
 *
 * \param itkImgFunc The custom part of the name of the macro to be called.
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForIntegralPixelTypes(itkImgFunc)                                                     \
  InstantiateAccessFunctionForFixedType(                                                                               \
    itkImgFunc, MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

/**
 * \brief Instantiate access function for floating point datatypes and all dimensions.
 *
 * \sa InstantiateAccessFunctionForFixedType
 *
 * \param itkImgFunc The custom part of the name of the macro to be called.
 *
 * \ingroup Adaptor
 */
#define InstantiateAccessFunctionForFloatingPixelTypes(itkImgFunc)                                                     \
  InstantiateAccessFunctionForFixedType(                                                                               \
    itkImgFunc, MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

#endif // of MITKINSTANTIATEACCESSFUNCTIONS_H_HEADER_INCLUDED
