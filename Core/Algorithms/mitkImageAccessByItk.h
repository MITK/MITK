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


#ifndef MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
#define MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
 
#include <mitkImageToItk.h>

#define _accessByItk(mitkImage, itkImageTypeFunction, pixeltype, dimension)           \
  if ( typeId == typeid(pixeltype) )                                                  \
{                                                                                     \
    typedef mitk::ImageToItk<pixeltype, dimension> ImageToItkType;                    \
    ImageToItkType::Pointer imagetoitk = ImageToItkType::New();                       \
    imagetoitk->SetInput(mitkImage);                                                  \
    imagetoitk->Update();                                                             \
    itkImageTypeFunction<pixeltype, dimension>(imagetoitk->GetOutput());              \
}                                                              

#define AccessByItk(mitkImage, itkImageTypeFunction)                           \
{                                                                              \
  const std::type_info& typeId=*mitkImage->GetPixelType().GetTypeId();         \
  switch (mitkImage->GetDimension())                                           \
  {                                                                            \
    case 2:                                                                    \
      _accessByItk(mitkImage, itkImageTypeFunction, double, 2) else            \
      _accessByItk(mitkImage, itkImageTypeFunction, float, 2) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, long, 2) else              \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned long, 2) else     \
      _accessByItk(mitkImage, itkImageTypeFunction, int, 2) else               \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned int, 2) else      \
      _accessByItk(mitkImage, itkImageTypeFunction, short, 2) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned short, 2) else    \
      _accessByItk(mitkImage, itkImageTypeFunction, char, 2) else              \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned char,  2)         \
      break;                                                                   \
    case 3:                                                                    \
      _accessByItk(mitkImage, itkImageTypeFunction, double, 3) else            \
      _accessByItk(mitkImage, itkImageTypeFunction, float, 3) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, long, 3) else              \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned long, 3) else     \
      _accessByItk(mitkImage, itkImageTypeFunction, int, 3) else               \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned int, 3) else      \
      _accessByItk(mitkImage, itkImageTypeFunction, short, 3) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned short, 3) else    \
      _accessByItk(mitkImage, itkImageTypeFunction, char, 3) else              \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned char,  3)         \
      break;                                                                   \
    case 4:                                                                    \
      _accessByItk(mitkImage, itkImageTypeFunction, double, 4) else            \
      _accessByItk(mitkImage, itkImageTypeFunction, float, 4) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, long, 4) else              \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned long, 4) else     \
      _accessByItk(mitkImage, itkImageTypeFunction, int, 4) else               \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned int, 4) else      \
      _accessByItk(mitkImage, itkImageTypeFunction, short, 4) else             \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned short, 4) else    \
      _accessByItk(mitkImage, itkImageTypeFunction, char, 4) else              \
      _accessByItk(mitkImage, itkImageTypeFunction, unsigned char,  4)         \
      break;                                                                   \
    default:                                                                   \
      assert(mitkImage->GetDimension()>=2 && mitkImage->GetDimension()<=4);    \
      break;                                                                   \
  }                                                                            \
}

#define Access3DByItk(mitkImage, itkImageTypeFunction)                         \
{                                                                              \
  const std::type_info& typeId=*mitkImage->GetPixelType().GetTypeId();         \
  assert(mitkImage->GetDimension()==3);                                        \
  _accessByItk(mitkImage, itkImageTypeFunction, double, 3) else                \
  _accessByItk(mitkImage, itkImageTypeFunction, float, 3) else                 \
  _accessByItk(mitkImage, itkImageTypeFunction, long, 3) else                  \
  _accessByItk(mitkImage, itkImageTypeFunction, unsigned long, 3) else         \
  _accessByItk(mitkImage, itkImageTypeFunction, int, 3) else                   \
  _accessByItk(mitkImage, itkImageTypeFunction, unsigned int, 3) else          \
  _accessByItk(mitkImage, itkImageTypeFunction, short, 3) else                 \
  _accessByItk(mitkImage, itkImageTypeFunction, unsigned short, 3) else        \
  _accessByItk(mitkImage, itkImageTypeFunction, char, 3) else                  \
  _accessByItk(mitkImage, itkImageTypeFunction, unsigned char,  3)             \
}
//----------------------- version with 1 additional paramater ------------------
#define _accessByItk_1(mitkImage, itkImageTypeFunction, pixeltype, dimension, param1)     \
  if ( typeId == typeid(pixeltype) )                                                      \
{                                                                                         \
    typedef mitk::ImageToItk<pixeltype, dimension> ImageToItkType;                        \
    ImageToItkType::Pointer imagetoitk = ImageToItkType::New();                           \
    imagetoitk->SetInput(mitkImage);                                                      \
    imagetoitk->Update();                                                                 \
    itkImageTypeFunction<pixeltype, dimension>(imagetoitk->GetOutput(), param1);          \
}

#define AccessByItk_1(mitkImage, itkImageTypeFunction, param1)                           \
{                                                                                        \
  const std::type_info& typeId=*mitkImage->GetPixelType().GetTypeId();                   \
  switch (mitkImage->GetDimension())                                                     \
  {                                                                                      \
    case 2:                                                                              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, double, 2, param1) else            \
      _accessByItk_1(mitkImage, itkImageTypeFunction, float, 2, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, long, 2, param1) else              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned long, 2, param1) else     \
      _accessByItk_1(mitkImage, itkImageTypeFunction, int, 2, param1) else               \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned int, 2, param1) else      \
      _accessByItk_1(mitkImage, itkImageTypeFunction, short, 2, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned short, 2, param1) else    \
      _accessByItk_1(mitkImage, itkImageTypeFunction, char, 2, param1) else              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned char,  2, param1)         \
      break;                                                                             \
    case 3:                                                                              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, double, 3, param1) else            \
      _accessByItk_1(mitkImage, itkImageTypeFunction, float, 3, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, long, 3, param1) else              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned long, 3, param1) else     \
      _accessByItk_1(mitkImage, itkImageTypeFunction, int, 3, param1) else               \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned int, 3, param1) else      \
      _accessByItk_1(mitkImage, itkImageTypeFunction, short, 3, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned short, 3, param1) else    \
      _accessByItk_1(mitkImage, itkImageTypeFunction, char, 3, param1) else              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned char,  3, param1)         \
      break;                                                                             \
    case 4:                                                                              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, double, 4, param1) else            \
      _accessByItk_1(mitkImage, itkImageTypeFunction, float, 4, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, long, 4, param1) else              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned long, 4, param1) else     \
      _accessByItk_1(mitkImage, itkImageTypeFunction, int, 4, param1) else               \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned int, 4, param1) else      \
      _accessByItk_1(mitkImage, itkImageTypeFunction, short, 4, param1) else             \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned short, 4, param1) else    \
      _accessByItk_1(mitkImage, itkImageTypeFunction, char, 4, param1) else              \
      _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned char,  4, param1)         \
      break;                                                                             \
    default:                                                                             \
      assert(mitkImage->GetDimension()>=2 && mitkImage->GetDimension()<=4);              \
      break;                                                                             \
  }                                                                                      \
}

#define Access3DByItk_1(mitkImage, itkImageTypeFunction, param1)                         \
{                                                                                        \
  const std::type_info& typeId=*mitkImage->GetPixelType().GetTypeId();                   \
  assert(mitkImage->GetDimension()==3);                                                  \
  _accessByItk_1(mitkImage, itkImageTypeFunction, double, 3, param1) else                \
  _accessByItk_1(mitkImage, itkImageTypeFunction, float, 3, param1) else                 \
  _accessByItk_1(mitkImage, itkImageTypeFunction, long, 3, param1) else                  \
  _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned long, 3, param1) else         \
  _accessByItk_1(mitkImage, itkImageTypeFunction, int, 3, param1) else                   \
  _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned int, 3, param1) else          \
  _accessByItk_1(mitkImage, itkImageTypeFunction, short, 3, param1) else                 \
  _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned short, 3, param1) else        \
  _accessByItk_1(mitkImage, itkImageTypeFunction, char, 3, param1) else                  \
  _accessByItk_1(mitkImage, itkImageTypeFunction, unsigned char,  3, param1)             \
}


#endif // of MITKIMAGEACCESSBYITK_H_HEADER_INCLUDED
