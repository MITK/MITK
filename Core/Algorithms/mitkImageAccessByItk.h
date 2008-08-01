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

#ifndef DOXYGEN_SKIP
#define _accessByItkWarning                                                            \
  {                                                                                    \
    itkGenericOutputMacro(<< "Pixel type " << pixelType.GetItkTypeAsString()           \
      << " not supported by AccessByItk")                                              \
  }

#define _accessByItkWarningParam(pixelType, accessor)                                  \
  {                                                                                    \
    itkGenericOutputMacro(<< "Pixel type " << pixelType.GetItkTypeAsString()           \
      << " not supported by " << accessor)                                             \
  }

#define _accessByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension)            \
  if ( pixelType == typeid(pixeltype) )                                                \
  {                                                                                    \
    typedef itk::Image<pixeltype, dimension> ImageType;                                \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();              \
    imagetoitk->SetInput(mitkImage);                                                   \
    imagetoitk->Update();                                                              \
    itkImageTypeFunction(imagetoitk->GetOutput());                                     \
  }                                                                                    \

#define _accessAllTypesByItk(mitkImage, itkImageTypeFunction, dimension)               \
      _accessByItk(mitkImage, itkImageTypeFunction, double, dimension) else            \
      _accessByItk(mitkImage, itkImageTypeFunction, float, dimension) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, int, dimension) else               \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned int, dimension) else      \
      _accessByItk(mitkImage, itkImageTypeFunction, short, dimension) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned short, dimension) else    \
      _accessByItk(mitkImage, itkImageTypeFunction, char, dimension) else              \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned char,  dimension) else    \
      _accessByItkWarning
#endif //DOXYGEN_SKIP

/**
 * @brief Access an mitk-image by an itk-image
 * 
 * Define a templated function or method (@a itkImageTypeFunction) 
 * within which the mitk-image (@a mitkImage) is accessed:
 * \code
 *   template < typename TPixel, unsigned int VImageDimension > 
 *   void ExampleFunction( itk::Image<TPixel, VImageDimension>* itkImage, TPixel* dummy = NULL );
 * \endcode
 * Instantiate the function using
 * \code
 *   InstantiateAccessFunction(ExampleFunction);
 * \endcode
 * Within the itk::Image passed to the function/method has the same
 * data-pointer as the mitk-image. So you have full read- and write-
 * access to the data vector of the mitk-image using the itk-image.
 * Call by:
 * \code
 *   inputMitkImage being of type mitk::Image*
 *   AccessByItk(inputMitkImage, ExampleFunction);
 * \endcode
 * @note If your inputMitkImage is an mitk::Image::Pointer, use 
 * inputMitkImage.GetPointer()
 * @note If you need to pass an additional parameter to your 
 * access-function (@a itkImageTypeFunction), use AccessByItk.
 * @note If you know the dimension of your input mitk-image,
 * it is better to use AccessFixedDimensionByItk (less code
 * is generated).
 * \sa AccessFixedDimensionByItk
 * \sa AccessFixedTypeByItk
 * \sa AccessFixedPixelTypeByItk
 * \sa AccessByItk_1
 * \sa AccessByItk_2
 * @ingroup Adaptor
 */
#define AccessByItk(mitkImage, itkImageTypeFunction)                                   \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  assert((mitkImage)->GetDimension()>=2 && (mitkImage)->GetDimension()<=3);            \
  if((mitkImage)->GetDimension()==2)                                                   \
  {                                                                                    \
    _accessAllTypesByItk(mitkImage, itkImageTypeFunction, 2);                          \
  }                                                                                    \
  else                                                                                 \
  {                                                                                    \
    _accessAllTypesByItk(mitkImage, itkImageTypeFunction, 3)                           \
  }                                                                                    \
}

//##Documentation
//## @brief Access an mitk-image with known dimension by an itk-image
//## 
//## For usage, see AccessByItk.
//## @param dimension dimension of the mitk-image. 
//##
//## If the image has a different dimension, an exception is thrown 
//## (by assert).
//## If you do not know the dimension for sure, use AccessByItk.
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk
//## \sa AccessFixedPixelTypeByItk
//## @ingroup Adaptor
#define AccessFixedDimensionByItk(mitkImage, itkImageTypeFunction, dimension)          \
{                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                        \
  const mitk::Image* constImage = mitkImage;                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                      \
  assert((mitkImage)->GetDimension()==dimension);                                      \
  _accessAllTypesByItk(mitkImage, itkImageTypeFunction, dimension)                     \
}

//##Documentation
//## @brief Access an mitk-image with known type (pixel type and dimension) 
//## by an itk-image
//## 
//## For usage, see AccessByItk.
//## @param dimension dimension of the mitk-image. If the image 
//## has a different dimension, an exception is thrown (by assert).
//## If you do not know the dimension for sure, use AccessByItk.
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk
//## \sa AccessFixedPixelTypeByItk
//## @ingroup Adaptor
#define AccessFixedTypeByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension)              \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
   const mitk::Image* constImage = mitkImage;                                                    \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert((mitkImage)->GetDimension()==dimension);                                                \
  _accessByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension) else                       \
  _accessByItkWarning                                                                            \
}

//##Documentation
//## @brief Access an mitk-image with known pixeltype (but unknown dimension) 
//## by an itk-image and pass two additional parameters to the access-function
//## 
//## For usage, see AccessByItk.
//## @param pixeltype pixel type of the mitk-image. 
//##
//## If the image has a different pixel type, an exception is 
//## thrown (by assert).
//## If you do not know the pixel type for sure, use AccessByItk.
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk
//## \sa AccessFixedPixelTypeByItk
//## @ingroup Adaptor
#define AccessFixedPixelTypeByItk(mitkImage, itkImageTypeFunction, pixeltype)                    \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert((mitkImage)->GetDimension()>=2 && (mitkImage)->GetDimension()<=3);                      \
  if((mitkImage)->GetDimension()==2)                                                             \
  {                                                                                              \
    _accessByItk(mitkImage, itkImageTypeFunction, pixeltype, 2) else                             \
    _accessByItkWarning                                                                          \
  }                                                                                              \
  else                                                                                           \
  {                                                                                              \
    _accessByItk(mitkImage, itkImageTypeFunction, pixeltype, 3) else                             \
    _accessByItkWarning                                                                          \
  }                                                                                              \
}

//----------------------- version with 1 additional paramater ------------------
#ifndef DOXYGEN_SKIP
#define _accessByItk_1(mitkImage, itkImageTypeFunction, pixeltype, dimension, param1)            \
  if ( pixelType == typeid(pixeltype) )                                                          \
  {                                                                                              \
    typedef itk::Image<pixeltype, dimension> ImageType;                                          \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                          \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();                        \
    imagetoitk->SetInput(mitkImage);                                                             \
    imagetoitk->Update();                                                                        \
    itkImageTypeFunction(imagetoitk->GetOutput(), param1);                                       \
  }

#define _accessAllTypesByItk_1(mitkImage, itkImageTypeFunction, dimension, param1)               \
      _accessByItk_1(mitkImage, itkImageTypeFunction, double, dimension, param1) else            \
      _accessByItk_1(mitkImage, itkImageTypeFunction, float, dimension, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, int, dimension, param1) else               \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned int, dimension, param1) else      \
      _accessByItk_1(mitkImage, itkImageTypeFunction, short, dimension, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned short, dimension, param1) else    \
      _accessByItk_1(mitkImage, itkImageTypeFunction, char, dimension, param1) else              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned char,  dimension, param1) else    \
      _accessByItkWarning                                                                        \

#endif //DOXYGEN_SKIP

//##Documentation
//## @brief Access an mitk-image by an itk-image and pass one 
//## additional parameter to the access-function
//## 
//## For usage, see AccessByItk. The only difference to AccessByItk
//## is that an additional parameter (@a param1) is passed.
//## @note If you know the dimension of your input mitk-image,
//## it is better to use AccessFixedDimensionByItk_1 (less code
//## is generated).
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk_1
//## \sa AccessFixedTypeByItk_1
//## \sa AccessFixedPixelTypeByItk_1
//## @ingroup Adaptor
#define AccessByItk_1(mitkImage, itkImageTypeFunction, param1)                                   \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert(mitkImage->GetDimension()>=2 && mitkImage->GetDimension()<=3);                          \
  if(mitkImage->GetDimension()==2)                                                               \
  {                                                                                              \
    _accessAllTypesByItk_1(mitkImage, itkImageTypeFunction, 2, param1);                          \
  }                                                                                              \
  else                                                                                           \
  {                                                                                              \
    _accessAllTypesByItk_1(mitkImage, itkImageTypeFunction, 3, param1)                           \
  }                                                                                              \
}

//##Documentation
//## @brief Access an mitk-image with known dimension by an itk-image 
//## and pass one additional parameter to the access-function
//## 
//## For usage, see AccessByItk_1 and AccessByItk.
//## @param dimension dimension of the mitk-image. 
//##
//## If the image has a different dimension, an exception is thrown 
//## (by assert).
//## If you do not know the dimension for sure, use AccessByItk_1.
//## \sa AccessByItk_2
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk_1
//## \sa AccessFixedTypeByItk_1
//## \sa AccessFixedPixelTypeByItk_1
//## @ingroup Adaptor
#define AccessFixedDimensionByItk_1(mitkImage, itkImageTypeFunction, dimension, param1)          \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert(mitkImage->GetDimension()==dimension);                                                  \
  _accessAllTypesByItk_1(mitkImage, itkImageTypeFunction, dimension, param1)                     \
}

//##Documentation
//## @brief Access an mitk-image with known type (pixel type and dimension) 
//## by an itk-image and pass one additional parameters to the access-function
//## 
//## For usage, see AccessByItk_1 and AccessByItk.
//## @param pixeltype pixel type of the mitk-image. 
//## @param dimension dimension of the mitk-image. 
//##
//## If the image has a different pixel type or dimension, an exception is 
//## thrown (by assert).
//## If you do not know the pixel type and dimension for sure, use AccessByItk_1.
//## \sa AccessByItk_1
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk_1
//## \sa AccessFixedTypeByItk_1
//## \sa AccessFixedPixelTypeByItk_1
//## @ingroup Adaptor
#define AccessFixedTypeByItk_1(mitkImage, itkImageTypeFunction, pixeltype, dimension, param1)    \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert(mitkImage->GetDimension()==dimension);                                                  \
  _accessByItk_1(mitkImage, itkImageTypeFunction, pixeltype, dimension, param1) else             \
  _accessByItkWarning                                                                            \
}

//##Documentation
//## @brief Access an mitk-image with known pixeltype (but unknown dimension) 
//## by an itk-image and pass one additional parameters to the access-function
//## 
//## For usage, see AccessByItk_1 and AccessByItk.
//## @param pixeltype pixel type of the mitk-image. 
//##
//## If the image has a different pixel type, an exception is 
//## thrown (by assert).
//## If you do not know the pixel type for sure, use AccessByItk_2.
//## \sa AccessByItk_1
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk_1
//## \sa AccessFixedPixelTypeByItk_1
//## @ingroup Adaptor
#define AccessFixedPixelTypeByItk_1(mitkImage, itkImageTypeFunction, pixeltype, param1)          \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert(mitkImage->GetDimension()>=2 && mitkImage->GetDimension()<=3);                          \
  if(mitkImage->GetDimension()==2)                                                               \
  {                                                                                              \
    _accessByItk_1(mitkImage, itkImageTypeFunction, pixeltype, 2, param1) else                   \
    _accessByItkWarning                                                                          \
  }                                                                                              \
  else                                                                                           \
  {                                                                                              \
    _accessByItk_1(mitkImage, itkImageTypeFunction, pixeltype, 3, param1) else                   \
    _accessByItkWarning                                                                          \
  }                                                                                              \
}

//----------------------- version with 2 additional paramaters -----------------
#ifndef DOXYGEN_SKIP
#define _accessByItk_2(mitkImage, itkImageTypeFunction, pixeltype, dimension, param1, param2)    \
  if ( pixelType == typeid(pixeltype) )                                                          \
  {                                                                                              \
    typedef itk::Image<pixeltype, dimension> ImageType;                                          \
    typedef mitk::ImageToItk<ImageType> ImageToItkType;                                          \
    itk::SmartPointer<ImageToItkType> imagetoitk = ImageToItkType::New();                        \
    imagetoitk->SetInput(mitkImage);                                                             \
    imagetoitk->Update();                                                                        \
    itkImageTypeFunction(imagetoitk->GetOutput(), param1, param2);                               \
  }

#define _accessAllTypesByItk_2(mitkImage, itkImageTypeFunction, dimension, param1, param2)               \
      _accessByItk_2(mitkImage, itkImageTypeFunction, double, dimension, param1, param2) else            \
      _accessByItk_2(mitkImage, itkImageTypeFunction, float, dimension, param1, param2) else             \
      _accessByItk_2(mitkImage, itkImageTypeFunction, int, dimension, param1, param2) else               \
      _accessByItk_2(mitkImage, itkImageTypeFunction, unsigned int, dimension, param1, param2) else      \
      _accessByItk_2(mitkImage, itkImageTypeFunction, short, dimension, param1, param2) else             \
      _accessByItk_2(mitkImage, itkImageTypeFunction, unsigned short, dimension, param1, param2) else    \
      _accessByItk_2(mitkImage, itkImageTypeFunction, char, dimension, param1, param2) else              \
      _accessByItk_2(mitkImage, itkImageTypeFunction, unsigned char,  dimension, param1, param2) else    \
      _accessByItkWarning                                                                                \

#endif //DOXYGEN_SKIP

//##Documentation
//## @brief Access an mitk-image by an itk-image and pass two 
//## additional parameters to the access-function
//## 
//## For usage, see AccessByItk. The only difference to AccessByItk
//## is that two additional parameters (@a param1, @a param2) are passed.
//## @note If you know the dimension of your input mitk-image,
//## it is better to use AccessFixedDimensionByItk_2 (less code
//## is generated).
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk_2
//## \sa AccessFixedTypeByItk_2
//## \sa AccessFixedPixelTypeByItk_2
//## @ingroup Adaptor
#define AccessByItk_2(mitkImage, itkImageTypeFunction, param1, param2)                           \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert(mitkImage->GetDimension()>=2 && mitkImage->GetDimension()<=3);                          \
  if(mitkImage->GetDimension()==2)                                                               \
  {                                                                                              \
    _accessAllTypesByItk_2(mitkImage, itkImageTypeFunction, 2, param1, param2);                  \
  }                                                                                              \
  else                                                                                           \
  {                                                                                              \
    _accessAllTypesByItk_2(mitkImage, itkImageTypeFunction, 3, param1, param2)                   \
  }                                                                                              \
}

//##Documentation
//## @brief Access an mitk-image with known dimension by an itk-image 
//## and pass two additional parameters to the access-function
//## 
//## For usage, see AccessByItk_2 and AccessByItk.
//## @param dimension dimension of the mitk-image. 
//##
//## If the image has a different dimension, an exception is thrown 
//## (by assert).
//## If you do not know the dimension for sure, use AccessByItk_2.
//## \sa AccessByItk_2
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk_2
//## \sa AccessFixedTypeByItk_2
//## \sa AccessFixedPixelTypeByItk_2
//## @ingroup Adaptor
#define AccessFixedDimensionByItk_2(mitkImage, itkImageTypeFunction, dimension, param1, param2)  \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert(mitkImage->GetDimension()==dimension);                                                  \
  _accessAllTypesByItk_2(mitkImage, itkImageTypeFunction, dimension, param1, param2)             \
}

//##Documentation
//## @brief Access an mitk-image with known type (pixel type and dimension) 
//## by an itk-image and pass two additional parameters to the access-function
//## 
//## For usage, see AccessByItk_2 and AccessByItk.
//## @param pixeltype pixel type of the mitk-image. 
//## @param dimension dimension of the mitk-image. 
//##
//## If the image has a different pixel type or dimension, an exception is 
//## thrown (by assert).
//## If you do not know the pixel type and dimension for sure, use AccessByItk_2.
//## \sa AccessByItk_2
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk_2
//## \sa AccessFixedPixelTypeByItk_2
//## @ingroup Adaptor
#define AccessFixedTypeByItk_2(mitkImage, itkImageTypeFunction, pixeltype, dimension, param1, param2)  \
{                                                                                                      \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                        \
  const mitk::Image* constImage = mitkImage;                                                           \
  const_cast<mitk::Image*>(constImage)->Update();                                                      \
  assert(mitkImage->GetDimension()==dimension);                                                        \
  _accessByItk_2(mitkImage, itkImageTypeFunction, pixeltype, dimension, param1, param2) else           \
  _accessByItkWarning                                                                                  \
}

//##Documentation
//## @brief Access an mitk-image with known pixel type (but unknown dimension) 
//## by an itk-image and pass two additional parameters to the access-function
//## 
//## For usage, see AccessByItk_2 and AccessByItk.
//## @param pixeltype pixel type of the mitk-image. 
//##
//## If the image has a different pixel type, an exception is 
//## thrown (by assert).
//## If you do not know the pixel type for sure, use AccessByItk_2.
//## \sa AccessByItk_2
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk_2
//## \sa AccessFixedPixelTypeByItk_2
//## @ingroup Adaptor
#define AccessFixedPixelTypeByItk_2(mitkImage, itkImageTypeFunction, pixeltype, param1, param2)  \
{                                                                                                \
  const mitk::PixelType& pixelType = mitkImage->GetPixelType();                                  \
  const mitk::Image* constImage = mitkImage;                                                     \
  const_cast<mitk::Image*>(constImage)->Update();                                                \
  assert(mitkImage->GetDimension()>=2 && mitkImage->GetDimension()<=3);                          \
  if(mitkImage->GetDimension()==2)                                                               \
  {                                                                                              \
    _accessByItk_2(mitkImage, itkImageTypeFunction, pixeltype, 2, param1, param2) else           \
    _accessByItkWarning                                                                          \
  }                                                                                              \
  else                                                                                           \
  {                                                                                              \
    _accessByItk_2(mitkImage, itkImageTypeFunction, pixeltype, 3, param1, param2) else           \
    _accessByItkWarning                                                                          \
  }                                                                                              \
}

#define _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, pixeltype2, dimension) \
  if ( pixelType2 == typeid(pixeltype2) )                                                                      \
  {                                                                                                            \
    typedef itk::Image<pixeltype, dimension> ImageType1;                                                       \
    typedef itk::Image<pixeltype2, dimension> ImageType2;                                                      \
    typedef mitk::ImageToItk<ImageType1> ImageToItkType1;                                                      \
    typedef mitk::ImageToItk<ImageType2> ImageToItkType2;                                                      \
    itk::SmartPointer<ImageToItkType1> imagetoitk1 = ImageToItkType1::New();                                   \
    imagetoitk1->SetInput(mitkImage1);                                                                         \
    imagetoitk1->Update();                                                                                     \
    itk::SmartPointer<ImageToItkType2> imagetoitk2 = ImageToItkType2::New();                                   \
    imagetoitk2->SetInput(mitkImage2);                                                                         \
    imagetoitk2->Update();                                                                                     \
    itkImageTypeFunction(imagetoitk1->GetOutput(), imagetoitk2->GetOutput());                                  \
  }     
       
#define _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, dimension)                    \
  if ( pixelType1 == typeid(pixeltype) )                                                                             \
  {                                                                                                                  \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, double, dimension) else          \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, float, dimension) else           \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, int, dimension) else             \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, unsigned int, dimension) else    \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, short, dimension) else           \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, unsigned short, dimension) else  \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, char, dimension) else            \
    _accessTwoImagesByItk2(mitkImage1, mitkImage2, itkImageTypeFunction, pixeltype, unsigned char, dimension) else   \
    _accessByItkWarningParam(pixelType2, _accessTwoImagesByItk)                                                      \
  }     

#define _accessTwoImagesAllTypesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, dimension)               \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, double, dimension) else            \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, float, dimension) else             \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, int, dimension) else               \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, unsigned int, dimension) else      \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, short, dimension) else             \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, unsigned short, dimension) else    \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, char, dimension) else              \
      _accessTwoImagesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, unsigned char,  dimension) else    \
      _accessByItkWarningParam(pixelType1, _accessTwoImagesByItk)

//##Documentation
//## @brief Access two mitk-images with known dimension by itk-images
//## 
//## For usage, see AccessByItk.
//## @param dimension dimension of the mitk-image. 
//##
//## If one of the images has a different dimension, an exception is thrown 
//## (by assert).
//## If you do not know the dimension for sure, use AccessByItk.
//## \sa AccessByItk
//## \sa AccessFixedDimensionByItk
//## \sa AccessFixedTypeByItk
//## \sa AccessFixedPixelTypeByItk
//## @ingroup Adaptor
#define AccessTwoImagesFixedDimensionByItk(mitkImage1, mitkImage2, itkImageTypeFunction, dimension)  \
{                                                                                                    \
  const mitk::PixelType& pixelType1 = mitkImage1->GetPixelType();                                    \
  const mitk::PixelType& pixelType2 = mitkImage2->GetPixelType();                                    \
  const mitk::Image* constImage1 = mitkImage1;                                                       \
  const mitk::Image* constImage2 = mitkImage2;                                                       \
  const_cast<mitk::Image*>(constImage1)->Update();                                                   \
  const_cast<mitk::Image*>(constImage2)->Update();                                                   \
  assert((mitkImage1)->GetDimension()==dimension);                                                   \
  assert((mitkImage2)->GetDimension()==dimension);                                                   \
  _accessTwoImagesAllTypesByItk(mitkImage1, mitkImage2, itkImageTypeFunction, dimension)             \
}

#ifdef mitkCore_EXPORTS
#define MITK_CORE_EXPORT_EXTERN 
#else
#define MITK_CORE_EXPORT_EXTERN extern 
#endif

//----------------------- cast functions. Will be moved to mitkImageCast.h -----------------
namespace mitk 
{
#ifndef DOXYGEN_SKIP
  template < typename TPixel, unsigned int VImageDimension, class ItkOutputImageType > 
  void _CastToItkImage2Access( itk::Image<TPixel, VImageDimension>* itkInputImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage);
#endif //DOXYGEN_SKIP
  //##Documentation
  //## @brief Cast an mitk::Image to an itk::Image with a specific type. You don't have to initialize the itk::Image<..>::Pointer.
  //## @ingroup Adaptor
 template <typename ItkOutputImageType> extern void MITK_CORE_EXPORT CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<ItkOutputImageType>& itkOutputImage);
}

//----------------------- include to be removed  -----------------------
#include <mitkInstantiateAccessFunctions.h>

#endif // of MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
