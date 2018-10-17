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

#ifndef MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
#define MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED

#include <itkCastImageFilter.h>
#include <mitkImageToItk.h>
#include <mitkPPArgCount.h>
#include <boost/preprocessor/expand.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_product.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/rem.hpp>

#include <sstream>

namespace mitk
{
  /**
   * \brief Exception class thrown in #AccessByItk macros.
   *
   * This exception can be thrown by the invocation of the #AccessByItk macros,
   * if the MITK image is of non-expected dimension or pixel type.
   *
   * \ingroup Adaptor
   */
  class AccessByItkException : public virtual std::runtime_error
  {
  public:
    AccessByItkException(const std::string &msg) : std::runtime_error(msg) {}
    ~AccessByItkException() throw() override {}
  };
}

#ifndef DOXYGEN_SKIP

#define _accessByItkPixelTypeException(pixelType, pixelTypeSeq)                                                        \
  {                                                                                                                    \
    std::string msg("Pixel type ");                                                                                    \
    msg.append(pixelType.GetPixelTypeAsString());                                                                      \
    msg.append(" is not in " BOOST_PP_STRINGIZE(pixelTypeSeq));                                                         \
    throw mitk::AccessByItkException(msg);                                                                             \
  }

#define _accessByItkDimensionException(dim, validDims)                                                                 \
  {                                                                                                                    \
    std::stringstream msg;                                                                                             \
    msg << "Dimension " << (dim) << " is not in " << validDims;                                                        \
    throw mitk::AccessByItkException(msg.str());                                                                       \
  }

#define _checkSpecificDimensionIter(r, mitkImage, dim)                                                                 \
  if (mitkImage->GetDimension() == dim)                                                                                \
    ;                                                                                                                  \
  else

#define _checkSpecificDimension(mitkImage, dimSeq)                                                                     \
  BOOST_PP_SEQ_FOR_EACH(_checkSpecificDimensionIter, mitkImage, dimSeq)                                                 \
  _accessByItkDimensionException(mitkImage->GetDimension(), BOOST_PP_STRINGIZE(dimSeq))

#define _msvc_expand_bug(macro, arg) BOOST_PP_EXPAND(macro arg)

//-------------------------------- 0-Arg Versions --------------------------------------

#define _accessByItk(itkImageTypeFunctionAndImageSeq, pixeltype, dimension)                                            \
  if (pixelType == mitk::MakePixelType<pixeltype, dimension>(pixelType.GetNumberOfComponents()) &&                     \
      BOOST_PP_SEQ_TAIL(itkImageTypeFunctionAndImageSeq)->GetDimension() == dimension)                                  \
  {                                                                                                                    \
    BOOST_PP_SEQ_HEAD(itkImageTypeFunctionAndImageSeq)                                                                  \
    (mitk::ImageToItkImage<pixeltype, dimension>(BOOST_PP_SEQ_TAIL(itkImageTypeFunctionAndImageSeq)).GetPointer());     \
  }                                                                                                                    \
  else

#define _accessByItkArgs(itkImageTypeFunction, type) (itkImageTypeFunction, BOOST_PP_TUPLE_REM(2) type)

// product will be of the form ((itkImageTypeFunction)(mitkImage))(short)(2) for pixel type short and dimension 2
#ifdef _MSC_VER
#define _accessByItkProductIter(r, product)                                                                            \
  _msvc_expand_bug(                                                                                                    \
    _accessByItk,                                                                                                      \
    _msvc_expand_bug(_accessByItkArgs, (BOOST_PP_SEQ_HEAD(product), BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product)))))
#else
#define _accessByItkProductIter(r, product)                                                                            \
  BOOST_PP_EXPAND(                                                                                                      \
    _accessByItk _accessByItkArgs(BOOST_PP_SEQ_HEAD(product), BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product))))
#endif

#define _accessFixedTypeByItk(itkImageTypeFunction, mitkImage, pixelTypeSeq, dimSeq)                                   \
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(_accessByItkProductIter, (((itkImageTypeFunction)(mitkImage)))(pixelTypeSeq)(dimSeq))

//-------------------------------- n-Arg Versions --------------------------------------

#define _accessByItk_n(itkImageTypeFunctionAndImageSeq, pixeltype, dimension, args)                                    \
  if (pixelType == mitk::MakePixelType<pixeltype, dimension>(pixelType.GetNumberOfComponents()) &&                     \
      BOOST_PP_SEQ_TAIL(itkImageTypeFunctionAndImageSeq)->GetDimension() == dimension)                                  \
  {                                                                                                                    \
    BOOST_PP_SEQ_HEAD(itkImageTypeFunctionAndImageSeq)                                                                  \
    (mitk::ImageToItkImage<pixeltype, dimension>(BOOST_PP_SEQ_TAIL(itkImageTypeFunctionAndImageSeq)).GetPointer(),      \
     BOOST_PP_TUPLE_REM(BOOST_PP_SEQ_HEAD(args)) BOOST_PP_SEQ_TAIL(args));                                                \
  }                                                                                                                    \
  else

#define _accessByItkArgs_n(itkImageTypeFunction, type, args) (itkImageTypeFunction, BOOST_PP_TUPLE_REM(2) type, args)

// product will be of the form (((itkImageTypeFunction)(mitkImage))(3)(a,b,c))(short)(2)
// for the variable argument list a,b,c and for pixel type short and dimension 2
#ifdef _MSC_VER
#define _accessByItkProductIter_n(r, product)                                                                          \
  _msvc_expand_bug(_accessByItk_n,                                                                                     \
                   _msvc_expand_bug(_accessByItkArgs_n,                                                                \
                                    (BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_HEAD(product)),                                      \
                                     BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product)),                                  \
                                     BOOST_PP_SEQ_TAIL(BOOST_PP_SEQ_HEAD(product)))))
#else
#define _accessByItkProductIter_n(r, product)                                                                          \
  BOOST_PP_EXPAND(_accessByItk_n _accessByItkArgs_n(BOOST_PP_SEQ_HEAD(BOOST_PP_SEQ_HEAD(product)),                        \
                                                   BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product)),                    \
                                                   BOOST_PP_SEQ_TAIL(BOOST_PP_SEQ_HEAD(product))))
#endif

#define _accessFixedTypeByItk_n(itkImageTypeFunction, mitkImage, pixelTypeSeq, dimSeq, va_tuple)                       \
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(                                                                                        \
    _accessByItkProductIter_n,                                                                                         \
    ((((itkImageTypeFunction)(mitkImage))(MITK_PP_ARG_COUNT va_tuple)va_tuple))(pixelTypeSeq)(dimSeq))

#endif // DOXYGEN_SKIP

/**
 * \brief Access a MITK image by an ITK image
 *
 * Define a templated function or method (\a itkImageTypeFunction)
 * within which the mitk-image (\a mitkImage) is accessed:
 * \code
 *   template < typename TPixel, unsigned int VImageDimension >
 *   void ExampleFunction( itk::Image<TPixel, VImageDimension>* itkImage );
 * \endcode
 *
 * The itk::Image passed to the function/method has the same
 * data-pointer as the mitk-image. Depending on the const-ness of the
 * \c mitkImage, your templated access function needs to take a const or
 * non-const itk::Image pointer and you will get read-only or full read/write
 * access to the data vector of the mitk-image using the itk-image.
 *
 * Example code using the access function above:
 * \code
 *   mitk::Image* inputMitkImage = ...
 *   try
 *   {
 *     AccessByItk(inputMitkImage, ExampleFunction);
 *   }
 *   catch (const mitk::AccessByItkException& e)
 *   {
 *     // mitk::Image is of wrong pixel type or dimension,
 *     // insert error handling here
 *   }
 * \endcode
 *
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \note If your inputMitkImage is an mitk::Image::Pointer, use
 * inputMitkImage.GetPointer()
 * \note If you need to pass additional parameters to your
 * access-function (\a itkImageTypeFunction), use #AccessByItk_n.
 * \note If you know the dimension of your input mitk-image,
 * it is better to use AccessFixedDimensionByItk (less code
 * is generated).
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk
 * \sa AccessByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessByItk(mitkImage, itkImageTypeFunction)                                                                   \
  AccessFixedTypeByItk(                                                                                                \
    mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

/**
 * \brief Access a mitk-image with known pixeltype (but unknown dimension) by an itk-image.
 *
 * For usage, see #AccessByItk.
 *
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int)
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * If the image has a different pixel type, a mitk::AccessByItkException exception is
 * thrown. If you do not know the pixel type for sure, use #AccessByItk.
 *
 * \sa AccessByItk
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessFixedPixelTypeByItk(mitkImage, itkImageTypeFunction, pixelTypeSeq)                                       \
  AccessFixedTypeByItk(mitkImage, itkImageTypeFunction, pixelTypeSeq, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

/**
 * \brief Access a mitk-image with an integral pixel type by an itk-image
 *
 * See #AccessByItk for details.
 *
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \sa AccessFixedPixelTypeByItk
 * \sa AccessByItk
 * \sa AccessIntegralPixelTypeByItk_n
 */
#define AccessIntegralPixelTypeByItk(mitkImage, itkImageTypeFunction)                                                  \
  AccessFixedTypeByItk(                                                                                                \
    mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

/**
 * \brief Access a mitk-image with a floating point pixel type by an ITK image
 *
 * See #AccessByItk for details.
 *
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \sa AccessFixedPixelTypeByItk
 * \sa AccessByItk
 * \sa AccessFloatingPixelTypeByItk_n
 */
#define AccessFloatingPixelTypeByItk(mitkImage, itkImageTypeFunction)                                                  \
  AccessFixedTypeByItk(                                                                                                \
    mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

#define AccessVectorPixelTypeByItk(mitkImage, itkImageTypeFunction)                                                    \
  AccessFixedTypeByItk(                                                                                                \
    mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ)

/**
 * \brief Access a mitk-image with known dimension by an itk-image
 *
 * For usage, see #AccessByItk.
 *
 * \param dimension Dimension of the mitk-image. If the image has a different dimension,
 *        a mitk::AccessByItkException exception is thrown.
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \note If you do not know the dimension for sure, use #AccessByItk.
 *
 * \sa AccessByItk
 * \sa AccessFixedDimensionByItk_n
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk
 *
 * \ingroup Adaptor
 */
#define AccessFixedDimensionByItk(mitkImage, itkImageTypeFunction, dimension)                                          \
  AccessFixedTypeByItk(mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ, (dimension))

#define AccessVectorFixedDimensionByItk(mitkImage, itkImageTypeFunction, dimension)                                    \
  AccessFixedTypeByItk(mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ, (dimension))

/**
 * \brief Access a mitk-image with known type (pixel type and dimension) by an itk-image.
 *
 * The provided mitk-image must be in the set of types created by taking the
 * cartesian product of the pixel type sequence and the dimension sequence.
 * For example, a call to
 * \code
 * AccessFixedTypeByItk(myMitkImage, MyAccessFunction, (short)(int), (2)(3))
 * \endcode
 * asserts that the type of myMitkImage (pixeltype,dim) is in the set {(short,2),(short,3),(int,2),(int,3)}.
 * For more information, see #AccessByItk.
 *
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int).
 * \param dimSeq A sequence of dimensions, like (2)(3).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * If the image has a different dimension or pixel type,
 * a mitk::AccessByItkException exception is thrown.
 *
 * \note If you do not know the dimension for sure, use #AccessByItk.
 *
 * \sa AccessByItk
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk_n
 * \sa AccessFixedPixelTypeByItk
 *
 * \ingroup Adaptor
 */
#define AccessFixedTypeByItk(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq)                                    \
                                                                                                                       \
  {                                                                                                                    \
    const mitk::PixelType &pixelType = mitkImage->GetPixelType();                                                      \
    _checkSpecificDimension(mitkImage, dimSeq);                                                                        \
    _accessFixedTypeByItk(itkImageTypeFunction, mitkImage, pixelTypeSeq, dimSeq)                                       \
      _accessByItkPixelTypeException(mitkImage->GetPixelType(), pixelTypeSeq)                                          \
  }

//------------------------------ n-Arg Access Macros -----------------------------------

/**
 * \brief Access a MITK image by an ITK image with one or more parameters.
 *
 * Define a templated function or method (\a itkImageTypeFunction) with one ore more
 * additional parameters, within which the mitk-image (\a mitkImage) is accessed:
 * \code
 *   template < typename TPixel, unsigned int VImageDimension >
 *   void ExampleFunction( itk::Image<TPixel, VImageDimension>* itkImage, SomeType param);
 * \endcode
 *
 * The itk::Image passed to the function/method has the same
 * data-pointer as the mitk-image. So you have full read- and write-
 * access to the data vector of the mitk-image using the itk-image.
 * Call by:
 * \code
 *   SomeType param = ...
 *   mitk::Image* inputMitkImage = ...
 *   try
 *   {
 *     AccessByItk_n(inputMitkImage, ExampleFunction, (param));
 *   }
 *   catch (const mitk::AccessByItkException& e)
 *   {
 *     // mitk::Image is of wrong pixel type or dimension,
 *     // insert error handling here
 *   }
 * \endcode
 *
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \note If your inputMitkImage is an mitk::Image::Pointer, use
 * inputMitkImage.GetPointer()
 * \note If you know the dimension of your input mitk-image,
 * it is better to use AccessFixedDimensionByItk_n (less code
 * is generated).
 * \sa AccessFixedDimensionByItk_n
 * \sa AccessFixedTypeByItk_n
 * \sa AccessFixedPixelTypeByItk_n
 * \sa AccessByItk
 *
 * \ingroup Adaptor
 */
#define AccessByItk_n(mitkImage, itkImageTypeFunction, va_tuple)                                                       \
  AccessFixedTypeByItk_n(                                                                                              \
    mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ, MITK_ACCESSBYITK_DIMENSIONS_SEQ, va_tuple)

/**
 * \brief Access a mitk-image with known pixeltype (but unknown dimension) by an itk-image
 *        with one or more parameters.
 *
 * For usage, see #AccessByItk_n.
 *
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * If the image has a different pixel type, a mitk::AccessByItkException exception is
 * thrown. If you do not know the pixel type for sure, use #AccessByItk_n.
 *
 * \sa AccessByItk_n
 * \sa AccessFixedDimensionByItk_n
 * \sa AccessFixedTypeByItk_n
 * \sa AccessFixedPixelTypeByItk
 *
 * \ingroup Adaptor
 */
#define AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, va_tuple)                           \
  AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, MITK_ACCESSBYITK_DIMENSIONS_SEQ, va_tuple)

/**
 * \brief Access an mitk::Image with an integral pixel type by an ITK image with
 *        one or more parameters.
 *
 * See #AccessByItk_n for details.
 *
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \sa AccessFixedPixelTypeByItk_n
 * \sa AccessByItk_n
 * \sa AccessIntegralPixelTypeByItk
 */
#define AccessIntegralPixelTypeByItk_n(mitkImage, itkImageTypeFunction, va_tuple)                                      \
  AccessFixedTypeByItk_n(mitkImage,                                                                                    \
                         itkImageTypeFunction,                                                                         \
                         MITK_ACCESSBYITK_INTEGRAL_PIXEL_TYPES_SEQ,                                                    \
                         MITK_ACCESSBYITK_DIMENSIONS_SEQ,                                                              \
                         va_tuple)

/**
 * \brief Access an mitk::Image with a floating point pixel type by an ITK image
 *        with one or more parameters.
 *
 * See #AccessByItk_n for details.
 *
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \sa AccessFixedPixelTypeByItk_n
 * \sa AccessByItk_n
 * \sa AccessFloatingPixelTypeByItk
 */
#define AccessFloatingPixelTypeByItk_n(mitkImage, itkImageTypeFunction, va_tuple)                                      \
  AccessFixedTypeByItk_n(mitkImage,                                                                                    \
                         itkImageTypeFunction,                                                                         \
                         MITK_ACCESSBYITK_FLOATING_PIXEL_TYPES_SEQ,                                                    \
                         MITK_ACCESSBYITK_DIMENSIONS_SEQ,                                                              \
                         va_tuple)

/**
 * \brief Access a vector mitk::Image by an ITK vector image with one or more parameters.
 *
 * See #AccessByItk_n for details.
 *
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \sa AccessFixedPixelTypeByItk_n
 * \sa AccessByItk_n
 */
#define AccessVectorPixelTypeByItk_n(mitkImage, itkImageTypeFunction, va_tuple)                                        \
  AccessFixedTypeByItk_n(mitkImage,                                                                                    \
                         itkImageTypeFunction,                                                                         \
                         MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ,                                                      \
                         MITK_ACCESSBYITK_DIMENSIONS_SEQ,                                                              \
                         va_tuple)

/**
 * \brief Access a mitk-image with known dimension by an itk-image with
 *        one or more parameters.
 *
 * For usage, see #AccessByItk_n.
 *
 * \param dimension Dimension of the mitk-image. If the image has a different dimension,
 *        a mitk::AccessByItkException exception is thrown.
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \note If you do not know the dimension for sure, use #AccessByItk_n.
 *
 * \sa AccessByItk_n
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk_n
 * \sa AccessFixedPixelTypeByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, va_tuple)                              \
  AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_PIXEL_TYPES_SEQ, (dimension), va_tuple)

/**
 * \brief Access a vector mitk-image with known dimension by a ITK vector image with
 *        one or more parameters.
 *
 * For usage, see #AccessByItk_n.
 *
 * \param dimension Dimension of the mitk-image. If the image has a different dimension,
 *        a mitk::AccessByItkException exception is thrown.
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * \note If you do not know the dimension for sure, use #AccessVectorPixelTypeByItk_n.
 *
 * \sa AccessByItk_n
 * \sa AccessVectorPixelTypeByItk_n
 * \sa AccessFixedTypeByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessVectorFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, va_tuple)                        \
  AccessFixedTypeByItk_n(                                                                                              \
    mitkImage, itkImageTypeFunction, MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ, (dimension), va_tuple)

/**
 * \brief Access a mitk-image with known type (pixel type and dimension) by an itk-image
 *        with one or more parameters.
 *
 * For usage, see AccessFixedTypeByItk.
 *
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int).
 * \param dimSeq A sequence of dimensions, like (2)(3).
 * \param va_tuple A variable length tuple containing the arguments to be passed
 *        to the access function itkImageTypeFunction, e.g. ("first", 2, THIRD).
 * \param mitkImage The MITK input image.
 * \param itkImageTypeFunction The templated access-function to be called.
 *
 * \throws mitk::AccessByItkException If mitkImage is of unsupported pixel type or dimension.
 *
 * If the image has a different dimension or pixel type,
 * a mitk::AccessByItkException exception is thrown.
 *
 * \note If you do not know the dimension for sure, use #AccessByItk_n.
 *
 * \sa AccessByItk_n
 * \sa AccessFixedDimensionByItk_n
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, va_tuple)                        \
                                                                                                                       \
  {                                                                                                                    \
    const mitk::PixelType &pixelType = mitkImage->GetPixelType();                                                      \
    _checkSpecificDimension(mitkImage, dimSeq);                                                                        \
    _accessFixedTypeByItk_n(itkImageTypeFunction, mitkImage, pixelTypeSeq, dimSeq, va_tuple)                           \
      _accessByItkPixelTypeException(mitkImage->GetPixelType(), pixelTypeSeq)                                          \
  }

//------------------------- For back-wards compatibility -------------------------------

#define AccessByItk_1(mitkImage, itkImageTypeFunction, arg1) AccessByItk_n(mitkImage, itkImageTypeFunction, (arg1))
#define AccessFixedPixelTypeByItk_1(mitkImage, itkImageTypeFunction, pixelTypeSeq, arg1)                               \
  AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, (arg1))
#define AccessFixedDimensionByItk_1(mitkImage, itkImageTypeFunction, dimension, arg1)                                  \
  AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, (arg1))
#define AccessFixedTypeByItk_1(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, arg1)                            \
  AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, (arg1))

#define AccessByItk_2(mitkImage, itkImageTypeFunction, arg1, arg2)                                                     \
  AccessByItk_n(mitkImage, itkImageTypeFunction, (arg1, arg2))
#define AccessFixedPixelTypeByItk_2(mitkImage, itkImageTypeFunction, pixelTypeSeq, arg1, arg2)                         \
  AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, (arg1, arg2))
#define AccessFixedDimensionByItk_2(mitkImage, itkImageTypeFunction, dimension, arg1, arg2)                            \
  AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, (arg1, arg2))
#define AccessFixedTypeByItk_2(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, arg1, arg2)                      \
  AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, (arg1, arg2))

#define AccessByItk_3(mitkImage, itkImageTypeFunction, arg1, arg2, arg3)                                               \
  AccessByItk_n(mitkImage, itkImageTypeFunction, (arg1, arg2, arg3))
#define AccessFixedPixelTypeByItk_3(mitkImage, itkImageTypeFunction, pixelTypeSeq, arg1, arg2, arg3)                   \
  AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, (arg1, arg2, arg3))
#define AccessFixedDimensionByItk_3(mitkImage, itkImageTypeFunction, dimension, arg1, arg2, arg3)                      \
  AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, (arg1, arg2, arg3))
#define AccessFixedTypeByItk_3(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, arg1, arg2, arg3)                \
  AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, (arg1, arg2, arg3))

//----------------------------- Access two MITK Images ---------------------------------

#ifndef DOXYGEN_SKIP

#define _accessTwoImagesByItk(itkImageTypeFunction, pixeltype1, dim1, pixeltype2, dim2)                                \
  if (pixelType1 == mitk::MakePixelType<itk::Image<pixeltype1, dim1>>() &&                                             \
      pixelType2 == mitk::MakePixelType<itk::Image<pixeltype2, dim2>>() && constImage1->GetDimension() == dim1 &&      \
      constImage2->GetDimension() == dim2)                                                                             \
  {                                                                                                                    \
    typedef itk::Image<pixeltype1, dim1> ImageType1;                                                                   \
    typedef itk::Image<pixeltype2, dim2> ImageType2;                                                                   \
    typedef mitk::ImageToItk<ImageType1> ImageToItkType1;                                                              \
    typedef mitk::ImageToItk<ImageType2> ImageToItkType2;                                                              \
    itk::SmartPointer<ImageToItkType1> imagetoitk1 = ImageToItkType1::New();                                           \
    imagetoitk1->SetInput(nonConstImage1);                                                                             \
    imagetoitk1->Update();                                                                                             \
    itk::SmartPointer<ImageToItkType2> imagetoitk2 = ImageToItkType2::New();                                           \
    imagetoitk2->SetInput(nonConstImage2);                                                                             \
    imagetoitk2->Update();                                                                                             \
    itkImageTypeFunction(imagetoitk1->GetOutput(), imagetoitk2->GetOutput());                                          \
  }                                                                                                                    \
  else

#define _accessTwoImagesByItkArgs2(itkImageTypeFunction, type1, type2)                                                 \
  (itkImageTypeFunction, BOOST_PP_TUPLE_REM(2) type1, BOOST_PP_TUPLE_REM(2) type2)

#define _accessTwoImagesByItkArgs(product)                                                                             \
  BOOST_PP_EXPAND(_accessTwoImagesByItkArgs2 BOOST_PP_EXPAND(                                                            \
    (BOOST_PP_SEQ_HEAD(product), BOOST_PP_TUPLE_REM(2) BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product)))))

// product is of the form (itkImageTypeFunction)((short,2))((char,2))
#ifdef _MSC_VER
#define _accessTwoImagesByItkIter(r, product)                                                                          \
  BOOST_PP_EXPAND(_accessTwoImagesByItk _msvc_expand_bug(                                                               \
    _accessTwoImagesByItkArgs2,                                                                                        \
    (BOOST_PP_SEQ_HEAD(product),                                                                                        \
     _msvc_expand_bug(BOOST_PP_TUPLE_REM(2), BOOST_PP_EXPAND(BOOST_PP_SEQ_TO_TUPLE(BOOST_PP_SEQ_TAIL(product)))))))
#else
#define _accessTwoImagesByItkIter(r, product) BOOST_PP_EXPAND(_accessTwoImagesByItk _accessTwoImagesByItkArgs(product))
#endif

#define _accessTwoImagesByItkForEach(itkImageTypeFunction, tseq1, tseq2)                                               \
  BOOST_PP_SEQ_FOR_EACH_PRODUCT(_accessTwoImagesByItkIter, ((itkImageTypeFunction))(tseq1)(tseq2))

#endif // DOXYGEN_SKIP

/**
 * \brief Access two mitk-images with known dimension by itk-images
 *
 * Define a templated function or method (\a itkImageTypeFunction)
 * within which the mitk-images (\a mitkImage1 and \a mitkImage2) are accessed:
 * \code
 *   template <typename TPixel1, unsigned int VImageDimension1, typename TPixel2, unsigned int VImageDimension2>
 *   void ExampleFunctionTwoImages(itk::Image<TPixel1, VImageDimension1>* itkImage1, itk::Image<TPixel2,
 * VImageDimension2>* itkImage2);
 * \endcode
 *
 * The itk::Image passed to the function/method has the same
 * data-pointer as the mitk-image. So you have full read- and write-
 * access to the data vector of the mitk-image using the itk-image.
 * Call by:
 * \code
 *   mitk::Image* inputMitkImage1 = ...
 *   mitk::Image* inputMitkImage2 = ...
 *   try
 *   {
 *     AccessTwoImagesFixedDimensionByItk(inputMitkImage1, inputMitkImage2, ExampleFunctionTwoImages, 3);
 *   }
 *   catch (const mitk::AccessByItkException& e)
 *   {
 *     // mitk::Image arguments are of wrong pixel type or dimension,
 *     // insert error handling here
 *   }
 * \endcode
 *
 * \note If your inputMitkImage1 or inputMitkImage2 is a mitk::Image::Pointer, use
 * inputMitkImage1.GetPointer().
 *
 * \param mitkImage1 The first MITK input image.
 * \param mitkImage1 The second MITK input image.
 * \param itkImageTypeFunction The name of the template access-function to be called.
 * \param dimension Dimension of the two mitk-images.
 *
 * \throws mitk::AccessByItkException If mitkImage1 and mitkImage2 have different dimensions or
 *         one of the images is of unsupported pixel type or dimension.
 *
 * \sa #AccessByItk
 *
 * \ingroup Adaptor
 */
#define AccessTwoImagesFixedDimensionByItk(mitkImage1, mitkImage2, itkImageTypeFunction, dimension)                    \
                                                                                                                       \
  {                                                                                                                    \
    const mitk::PixelType &pixelType1 = mitkImage1->GetPixelType();                                                    \
    const mitk::PixelType &pixelType2 = mitkImage2->GetPixelType();                                                    \
    const mitk::Image *constImage1 = mitkImage1;                                                                       \
    const mitk::Image *constImage2 = mitkImage2;                                                                       \
    mitk::Image *nonConstImage1 = const_cast<mitk::Image *>(constImage1);                                              \
    mitk::Image *nonConstImage2 = const_cast<mitk::Image *>(constImage2);                                              \
    nonConstImage1->Update();                                                                                          \
    nonConstImage2->Update();                                                                                          \
    _checkSpecificDimension(mitkImage1, (dimension));                                                                  \
    _checkSpecificDimension(mitkImage2, (dimension));                                                                  \
    _accessTwoImagesByItkForEach(                                                                                      \
      itkImageTypeFunction, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension), MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension))    \
    {                                                                                                                  \
      std::string msg("Pixel type ");                                                                                  \
      msg.append(pixelType1.GetComponentTypeAsString());                                                               \
      msg.append(" or pixel type ");                                                                                   \
      msg.append(pixelType2.GetComponentTypeAsString());                                                               \
      msg.append(" is not in " BOOST_PP_STRINGIZE(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension)));                         \
      throw mitk::AccessByItkException(msg);                                                                           \
    }                                                                                                                  \
  }

#endif // of MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
