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


#ifndef MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
#define MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
 
#include <itkCastImageFilter.h>
#include <mitkImageToItk.h>

#include <mitkPPArgCount.h>
#include <mitkPPSeqForEach.h>
#include <mitkPPSeqForEachProduct.h>
#include <mitkPPSeqToTuple.h>
#include <mitkPPCat.h>
#include <mitkPPExpand.h>
#include <mitkPPTupleRem.h>
#include <mitkPPStringize.h>

#include <sstream>

namespace mitk {

/**
 * \brief Exception class thrown in AccessByItk macros.
 *
 * This exception can be thrown by the invocation of the AccessByItk macros,
 * if the MITK image is of non-expected dimension or pixel type.
 */
class AccessByItkException : public virtual std::runtime_error
{
public:

  AccessByItkException(const std::string& msg) : std::runtime_error(msg) {}
  ~AccessByItkException() throw() {}
};

}

#ifndef DOXYGEN_SKIP

#define _accessByItkPixelTypeException(pixelType, pixelTypeSeq)                        \
  {                                                                                    \
    std::string msg("Pixel type ");                                                    \
    msg.append(pixelType.GetItkTypeAsString());                                        \
    msg.append(" is not in " MITK_PP_STRINGIZE(pixelTypeSeq));                         \
    throw mitk::AccessByItkException(msg);                                             \
  }

#define _accessByItkDimensionException(dim, validDims)                                 \
  {                                                                                    \
    std::stringstream msg;                                                             \
    msg << "Dimension " << (dim) << " is not in " << validDims ;                       \
    throw mitk::AccessByItkException(msg.str());                                       \
  }

#define _checkValidDimension(mitkImage)                                                \
  {                                                                                    \
    if (mitkImage->GetDimension() < 2 || mitkImage->GetDimension() > 3)                \
    _accessByItkDimensionException(mitkImage->GetDimension(), "(2)(3)");               \
  }

#define _checkSpecificDimensionIter(r, mitkImage, dim)                                 \
  if (mitkImage->GetDimension() == dim); else

#define _checkSpecificDimension(mitkImage, dimSeq)                                     \
  MITK_PP_SEQ_FOR_EACH(_checkSpecificDimensionIter, mitkImage, dimSeq)                 \
  _accessByItkDimensionException(mitkImage->GetDimension(), MITK_PP_STRINGIZE(dimSeq))

#define _msvc_expand_bug(macro, arg) MITK_PP_EXPAND(macro arg)

//-------------------------------- 0-Arg Versions --------------------------------------

#define _accessByItk(itkImageTypeFunction, pixeltype, dimension)                       \
  if ( pixelType == typeid(pixeltype) && constImage->GetDimension() == dimension)      \
  {                                                                                    \
    typedef itk::Image<pixeltype, dimension> ImageType;                                \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();              \
    imagetoitk->SetInput(constImage);                                                  \
    imagetoitk->Update();                                                              \
    itkImageTypeFunction(imagetoitk->GetOutput());                                     \
  } else

#define _accessByItkArgs(itkImageTypeFunction, type)                                   \
  (itkImageTypeFunction, MITK_PP_TUPLE_REM(2)type)

#ifdef _MSC_VER
#define _accessByItkIter(r, itkImageTypeFunction, type)                                \
  _msvc_expand_bug(_accessByItk, (itkImageTypeFunction, MITK_PP_TUPLE_REM(2)type))
#else
#define _accessByItkIter(r, itkImageTypeFunction, type)                                \
  MITK_PP_EXPAND(_accessByItk _accessByItkArgs(itkImageTypeFunction, type))
#endif

#define _accessAllTypesByItk(itkImageTypeFunction, dimension)                          \
  MITK_PP_SEQ_FOR_EACH(_accessByItkIter, itkImageTypeFunction, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension))

// product will be of the form (itkImageTypeFunction)(short)(2) for pixel type short and dimension 2
#ifdef _MSC_VER
#define _accessByItkProductIter(r, product)                                          \
  _msvc_expand_bug(_accessByItk, _msvc_expand_bug(_accessByItkArgs, (MITK_PP_SEQ_HEAD(product), MITK_PP_SEQ_TO_TUPLE(MITK_PP_SEQ_TAIL(product)))))
#else
#define _accessByItkProductIter(r, product)                                            \
  MITK_PP_EXPAND(_accessByItk _accessByItkArgs(MITK_PP_SEQ_HEAD(product), MITK_PP_SEQ_TO_TUPLE(MITK_PP_SEQ_TAIL(product))))
#endif

#define _accessFixedTypeByItk(itkImageTypeFunction, pixelTypeSeq, dimSeq)              \
  MITK_PP_SEQ_FOR_EACH_PRODUCT(_accessByItkProductIter, ((itkImageTypeFunction))(pixelTypeSeq)(dimSeq))

//-------------------------------- n-Arg Versions --------------------------------------

#define _accessByItk_n(itkImageTypeFunction, pixeltype, dimension, args)               \
  if ( pixelType == typeid(pixeltype) && constImage->GetDimension() == dimension)      \
  {                                                                                    \
    typedef itk::Image<pixeltype, dimension> ImageType;                                \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();              \
    imagetoitk->SetInput(constImage);                                                  \
    imagetoitk->Update();                                                              \
    itkImageTypeFunction(imagetoitk->GetOutput(), MITK_PP_TUPLE_REM(MITK_PP_SEQ_HEAD(args))MITK_PP_SEQ_TAIL(args)); \
  } else                                                                               

#define _accessByItkArgs_n(itkImageTypeFunction, type, args)                           \
  (itkImageTypeFunction, MITK_PP_TUPLE_REM(2) type, args)

// data will be of the form (itkImageTypeFunction)(3)(a,b,c) for the variable argument list a,b,c
#ifdef _MSC_VER
#define _accessByItkIter_n(r, data, type)                                              \
  _msvc_expand_bug(_accessByItk_n, (MITK_PP_SEQ_HEAD(data), MITK_PP_TUPLE_REM(2)type, MITK_PP_SEQ_TAIL(data)))
#else
#define _accessByItkIter_n(r, data, type)                                              \
  MITK_PP_EXPAND(_accessByItk_n _accessByItkArgs_n(MITK_PP_SEQ_HEAD(data), type, MITK_PP_SEQ_TAIL(data)))
#endif

#define _accessAllTypesByItk_n(itkImageTypeFunction, dimension, va_tuple)              \
  MITK_PP_SEQ_FOR_EACH(_accessByItkIter_n, (itkImageTypeFunction)(MITK_PP_ARG_COUNT va_tuple) va_tuple, MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension))


// product will be of the form ((itkImageTypeFunction)(3)(a,b,c))(short)(2)
// for the variable argument list a,b,c and for pixel type short and dimension 2
#ifdef _MSC_VER
#define _accessByItkProductIter_n(r, product)                                          \
  _msvc_expand_bug(_accessByItk_n, _msvc_expand_bug(_accessByItkArgs_n, (MITK_PP_SEQ_HEAD(MITK_PP_SEQ_HEAD(product)), MITK_PP_SEQ_TO_TUPLE(MITK_PP_SEQ_TAIL(product)), MITK_PP_SEQ_TAIL(MITK_PP_SEQ_HEAD(product)))))
#else
#define _accessByItkProductIter_n(r, product)                                          \
  MITK_PP_EXPAND(_accessByItk_n _accessByItkArgs_n(MITK_PP_SEQ_HEAD(MITK_PP_SEQ_HEAD(product)), MITK_PP_SEQ_TO_TUPLE(MITK_PP_SEQ_TAIL(product)), MITK_PP_SEQ_TAIL(MITK_PP_SEQ_HEAD(product))))
#endif

#define _accessFixedTypeByItk_n(itkImageTypeFunction, pixelTypeSeq, dimSeq, va_tuple)  \
  MITK_PP_SEQ_FOR_EACH_PRODUCT(_accessByItkProductIter_n, (((itkImageTypeFunction)(MITK_PP_ARG_COUNT va_tuple) va_tuple))(pixelTypeSeq)(dimSeq))


#endif //DOXYGEN_SKIP

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
 * data-pointer as the mitk-image. So you have full read- and write-
 * access to the data vector of the mitk-image using the itk-image.
 * Call by:
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
 * \note If your inputMitkImage is an mitk::Image::Pointer, use
 * inputMitkImage.GetPointer()
 * \note If you need to pass additional parameters to your
 * access-function (\a itkImageTypeFunction), use AccessByItk_n.
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
#define AccessByItk(mitkImage, itkImageTypeFunction)                                   \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessAllTypesByItk(itkImageTypeFunction, 2)                                        \
  _accessAllTypesByItk(itkImageTypeFunction, 3)                                        \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), MITK_ACCESSBYITK_PIXEL_TYPES_SEQ) \
}

/**
 * \brief Access a mitk-image with known pixeltype (but unknown dimension) by an itk-image.
 *
 * For usage, see AccessByItk.
 *
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int)
 *
 * If the image has a different pixel type, a mitk::AccessByItkException exception is
 * thrown. If you do not know the pixel type for sure, use AccessByItk.
 *
 * \sa AccessByItk
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessFixedPixelTypeByItk(mitkImage, itkImageTypeFunction, pixelTypeSeq)       \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessFixedTypeByItk(itkImageTypeFunction, pixelTypeSeq, (2)(3))                    \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), pixelTypeSeq)              \
}

/**
 * \brief Access a mitk-image with an integral pixel type by an itk-image
 *
 * See AccessByItk for details.
 *
 * \sa AccessFixedPixelTypeByItk
 * \sa AccessByItk
 * \sa AccessIntegralPixelTypeByItk_n
 */
#define AccessIntegralPixelTypeByItk(mitkImage, itkImageTypeFunction)                  \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessFixedTypeByItk(itkImageTypeFunction, (int)(unsigned int)(short)(unsigned short)(char)(unsigned char), (2)(3)) \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), (int)(unsigned int)(short)(unsigned short)(char)(unsigned char)) \
}

/**
 * \brief Access a mitk-image with a floating point pixel type by an ITK image
 *
 * See AccessByItk for details.
 *
 * \sa AccessFixedPixelTypeByItk
 * \sa AccessByItk
 * \sa AccessFloatingPixelTypeByItk_n
 */
#define AccessFloatingPixelTypeByItk(mitkImage, itkImageTypeFunction)                  \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessFixedTypeByItk(itkImageTypeFunction, (float)(double), (2)(3))                 \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), (float)(double))           \
}


/**
 * \brief Access a mitk-image with known dimension by an itk-image
 *
 * For usage, see AccessByItk.
 *
 * \param dimension Dimension of the mitk-image. If the image has a different dimension,
 *        a mitk::AccessByItkException exception is thrown.
 *
 * \note If you do not know the dimension for sure, use AccessByItk.
 *
 * \sa AccessByItk
 * \sa AccessFixedDimensionByItk_n
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk
 *
 * \ingroup Adaptor
 */
#define AccessFixedDimensionByItk(mitkImage, itkImageTypeFunction, dimension)          \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkSpecificDimension(mitkImage, (dimension));                                     \
  _accessAllTypesByItk(itkImageTypeFunction, dimension)                                \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), MITK_ACCESSBYITK_PIXEL_TYPES_SEQ) \
}

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
 * For more information, see AccessByItk.
 *
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int).
 * \param dimension A sequence of dimensions, like (2)(3).
 *
 * If the image has a different dimension or pixel type,
 * a mitk::AccessByItkException exception is thrown.
 *
 * \note If you do not know the dimension for sure, use AccessByItk.
 *
 * \sa AccessByItk
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk_n
 * \sa AccessFixedPixelTypeByItk
 *
 * \ingroup Adaptor
 */
#define AccessFixedTypeByItk(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq)    \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkSpecificDimension(mitkImage, dimSeq);                                          \
  _accessFixedTypeByItk(itkImageTypeFunction, pixelTypeSeq, dimSeq)                    \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), pixelTypeSeq)              \
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
 *     AccessByItk_n(inputMitkImage, ExampleFunction, param);
 *   }
 *   catch (const mitk::AccessByItkException& e)
 *   {
 *     // mitk::Image is of wrong pixel type or dimension,
 *     // insert error handling here
 *   }
 * \endcode
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
#define AccessByItk_n(mitkImage, itkImageTypeFunction, va_tuple)                       \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessAllTypesByItk_n(itkImageTypeFunction, 2, va_tuple)                            \
  _accessAllTypesByItk_n(itkImageTypeFunction, 3, va_tuple)                            \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), MITK_ACCESSBYITK_PIXEL_TYPES_SEQ) \
}

/**
 * \brief Access a mitk-image with known pixeltype (but unknown dimension) by an itk-image
 *        with one or more parameters.
 *
 * For usage, see AccessByItk_n.
 *
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int)
 *
 * If the image has a different pixel type, a mitk::AccessByItkException exception is
 * thrown. If you do not know the pixel type for sure, use AccessByItk_n.
 *
 * \sa AccessByItk_n
 * \sa AccessFixedDimensionByItk_n
 * \sa AccessFixedTypeByItk_n
 * \sa AccessFixedPixelTypeByItk
 *
 * \ingroup Adaptor
 */
#define AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, va_tuple) \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessFixedTypeByItk_n(itkImageTypeFunction, pixelTypeSeq, (2)(3), va_tuple)        \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), pixelTypeSeq)              \
}

/**
 * \brief Access an mitk::Image with an integral pixel type by an ITK image with
 *        one or more parameters.
 *
 * See AccessByItk_n for details.
 *
 * \sa AccessFixedPixelTypeByItk_n
 * \sa AccessByItk_n
 * \sa AccessIntegralPixelTypeByItk
 */
#define AccessIntegralPixelTypeByItk_n(mitkImage, itkImageTypeFunction, va_tuple)      \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessFixedTypeByItk_n(itkImageTypeFunction, (int)(unsigned int)(short)(unsigned short)(char)(unsigned char), (2)(3), va_tuple) \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), (int)(unsigned int)(short)(unsigned short)(char)(unsigned char)) \
}

/**
 * \brief Access an mitk::Image with a floating point pixel type by an ITK image
 *        with one or more parameters.
 *
 * See AccessByItk_n for details.
 *
 * \sa AccessFixedPixelTypeByItk_n
 * \sa AccessByItk_n
 * \sa AccessFloatingPixelTypeByItk
 */
#define AccessFloatingPixelTypeByItk_n(mitkImage, itkImageTypeFunction, va_tuple)      \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkValidDimension(mitkImage);                                                     \
  _accessFixedTypeByItk_n(itkImageTypeFunction, (float)(double), (2)(3), va_tuple)     \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), (float)(double))           \
}

/**
 * \brief Access a mitk-image with known dimension by an itk-image with
 *        one or more parameters.
 *
 * For usage, see AccessByItk_n.
 *
 * \param dimension Dimension of the mitk-image. If the image has a different dimension,
 *        a mitk::AccessByItkException exception is thrown.
 *
 * \note If you do not know the dimension for sure, use AccessByItk_n.
 *
 * \sa AccessByItk_n
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk_n
 * \sa AccessFixedPixelTypeByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, va_tuple) \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  _checkSpecificDimension(mitkImage, (dimension));                                     \
  _accessAllTypesByItk_n(itkImageTypeFunction, dimension, va_tuple)                    \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), MITK_ACCESSBYITK_PIXEL_TYPES_SEQ) \
}

/**
 * \brief Access a mitk-image with known type (pixel type and dimension) by an itk-image
 *        with one or more parameters.
 *
 * For usage, see AccessFixedTypeByItk.
 *
 * \param pixelTypeSeq A sequence of pixel types, like (short)(char)(int).
 * \param dimension A sequence of dimensions, like (2)(3).
 *
 * If the image has a different dimension or pixel type,
 * a mitk::AccessByItkException exception is thrown.
 *
 * \note If you do not know the dimension for sure, use AccessByItk_n.
 *
 * \sa AccessByItk_n
 * \sa AccessFixedDimensionByItk_n
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk_n
 *
 * \ingroup Adaptor
 */
#define AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, va_tuple) \
{                                                                                          \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                            \
  const mitk::Image* constImage = mitkImage;                                               \
  const_cast<mitk::Image*>(constImage)->Update();                                          \
  _checkSpecificDimension(mitkImage, dimSeq);                                              \
  _accessFixedTypeByItk_n(itkImageTypeFunction, pixelTypeSeq, dimSeq, va_tuple)            \
  _accessByItkPixelTypeException(mitkImage->GetPixelType(), pixelTypeSeq)                  \
}

//------------------------- For back-wards compatibility -------------------------------

#define AccessByItk_1(mitkImage, itkImageTypeFunction, arg1) AccessByItk_n(mitkImage, itkImageTypeFunction, (arg1))
#define AccessFixedPixelTypeByItk_1(mitkImage, itkImageTypeFunction, pixelTypeSeq, arg1) AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, (arg1))
#define AccessFixedDimensionByItk_1(mitkImage, itkImageTypeFunction, dimension, arg1) AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, (arg1))
#define AccessFixedTypeByItk_1(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, arg1) AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, (arg1))

#define AccessByItk_2(mitkImage, itkImageTypeFunction, arg1, arg2) AccessByItk_n(mitkImage, itkImageTypeFunction, (arg1,arg2))
#define AccessFixedPixelTypeByItk_2(mitkImage, itkImageTypeFunction, pixelTypeSeq, arg1, arg2) AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, (arg1,arg2))
#define AccessFixedDimensionByItk_2(mitkImage, itkImageTypeFunction, dimension, arg1, arg2) AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, (arg1,arg2))
#define AccessFixedTypeByItk_2(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, arg1, arg2) AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, (arg1,arg2))

#define AccessByItk_3(mitkImage, itkImageTypeFunction, arg1, arg2, arg3) AccessByItk_n(mitkImage, itkImageTypeFunction, (arg1,arg2,arg3))
#define AccessFixedPixelTypeByItk_3(mitkImage, itkImageTypeFunction, pixelTypeSeq, arg1, arg2, arg3) AccessFixedPixelTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, (arg1,arg2,arg3))
#define AccessFixedDimensionByItk_3(mitkImage, itkImageTypeFunction, dimension, arg1, arg2, arg3) AccessFixedDimensionByItk_n(mitkImage, itkImageTypeFunction, dimension, (arg1,arg2,arg3))
#define AccessFixedTypeByItk_3(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, arg1, arg2, arg3) AccessFixedTypeByItk_n(mitkImage, itkImageTypeFunction, pixelTypeSeq, dimSeq, (arg1,arg2,arg3))


//----------------------------- Access two MITK Images ---------------------------------

#ifndef DOXYGEN_SKIP

#define _accessTwoImagesByItkCheckType(pixeltype1, dim1, pixeltype2, dim2)             \
  if (pixelType1 == typeid(pixeltype1) && pixelType2 == typeid(pixeltype2) &&          \
      mitkImage1->GetDimension() == dim1 && mitkImage2->GetDimension() == dim2)        \

#define _accessTwoImagesByItk2(type1, type2) \
  _accessTwoImagesByItkCheckType(MITK_PP_TUPLE_REM(2)type1, MITK_PP_TUPLE_REM(2)type2) \
  {                                                                                    \
    typedef itk::Image<MITK_PP_TUPLE_REM(2)type1> ImageType1;                          \
    typedef itk::Image<MITK_PP_TUPLE_REM(2)type2> ImageType2;                          \
    typedef mitk::ImageToItk<ImageType1> ImageToItkType1;                              \
    typedef mitk::ImageToItk<ImageType2> ImageToItkType2;                              \
    itk::SmartPointer<ImageToItkType1> imagetoitk1 = ImageToItkType1::New();           \
    imagetoitk1->SetInput(mitkImage1);                                                 \
    imagetoitk1->Update();                                                             \
    itk::SmartPointer<ImageToItkType2> imagetoitk2 = ImageToItkType2::New();           \
    imagetoitk2->SetInput(mitkImage2);                                                 \
    imagetoitk2->Update();                                                             \
    itkImageTypeFunction(imagetoitk1->GetOutput(), imagetoitk2->GetOutput());          \
  } else

// product is of the form ((short,2))((char,3))
#define _accessTwoImagesByItk(r, product)                                              \
  _accessTwoImagesByItk2 MITK_PP_SEQ_TO_TUPLE(product)

#endif // DOXYGEN_SKIP

//##Documentation
//## @brief Access two mitk-images with known dimension by itk-images
//## 
//## For usage, see AccessByItk.
//## @param dimension dimension of the mitk-image. 
//##
//## If one of the images has a different dimension, a mitk::AccessByItkException exception is thrown.
//##
//## If you do not know the dimension for sure, use AccessByItk.
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk
//## \sa AccessFixedPixelTypeByItk
//## @ingroup Adaptor
#define AccessTwoImagesFixedDimensionByItk(mitkImage1, mitkImage2, itkImageTypeFunction, dimension) \
{                                                                                                   \
  const mitk::PixelType& pixelType1 = mitkImage1->GetPixelType();                                   \
  const mitk::PixelType& pixelType2 = mitkImage2->GetPixelType();                                   \
  const mitk::Image* constImage1 = mitkImage1;                                                      \
  const mitk::Image* constImage2 = mitkImage2;                                                      \
  const_cast<mitk::Image*>(constImage1)->Update();                                                  \
  const_cast<mitk::Image*>(constImage2)->Update();                                                  \
  _checkSpecificDimension(mitkImage1, (dimension));                                                 \
  _checkSpecificDimension(mitkImage2, (dimension));                                                 \
  MITK_PP_FOR_EACH_PRODUCT(_accessTwoImagesByItk, (MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension))(MITK_ACCESSBYITK_TYPES_DIMN_SEQ(dimension)) \
}


#endif // of MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
