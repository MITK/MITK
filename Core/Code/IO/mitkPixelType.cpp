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


#include "mitkPixelType.h"
#include <mitkLogMacros.h>
#include <itkVector.h>
#include <itkRGBPixel.h>
#include <itkRGBAPixel.h>
#include <itkCovariantVector.h>
#include "itkDiffusionTensor3D.h"

#define HUNDRED_VECS(HUN)   \
  TEN_VECS(HUN)                \
  TEN_VECS(HUN+10)             \
  TEN_VECS(HUN+20)             \
  TEN_VECS(HUN+30)             \
  TEN_VECS(HUN+40)             \
  TEN_VECS(HUN+50)             \
  TEN_VECS(HUN+60)             \
  TEN_VECS(HUN+70)             \
  TEN_VECS(HUN+80)             \
  TEN_VECS(HUN+90)             \

#define TEN_VECS(TEN)   \
  if(false){}               \
  N_VEC(TEN+ 1)                \
  N_VEC(TEN+ 2)             \
  N_VEC(TEN+ 3)             \
  N_VEC(TEN+ 4)             \
  N_VEC(TEN+ 5)             \
  N_VEC(TEN+ 6)             \
  N_VEC(TEN+ 7)             \
  N_VEC(TEN+ 8)             \
  N_VEC(TEN+ 9)             \
  N_VEC(TEN+10)             \

#define N_VEC(N_DIRS)       \
  _N_VEC(N_DIRS,double)     \
  _N_VEC(N_DIRS,float)      \
  _N_VEC(N_DIRS,short)      \

#define _N_VEC(N_DIRS,PIXTYPE)                                                          \
  else if ( *m_TypeId == typeid( itk::Vector<PIXTYPE,N_DIRS> ))  \
  {                                                                                     \
  found = true;                                                                      \
  m_TypeId = & typeid( PIXTYPE );                                                    \
  m_NumberOfComponents *= N_DIRS;                                                \
  m_Type = mitkIpPicFloat;                                                               \
  m_Bpe = sizeof(PIXTYPE) * 8 * m_NumberOfComponents;                                \
  m_ItkTypeId = &typeid( itk::Vector<PIXTYPE,N_DIRS> );     \
  }                                                                                     \



const std::type_info &mitk::GetPixelTypeFromITKImageIO(const itk::ImageIOBase::Pointer imageIO)
{
  // return the component type for scalar types
  if( imageIO->GetNumberOfComponents() == 1)
  {
    return imageIO->GetComponentTypeInfo();
  }
  else
  {
    itk::ImageIOBase::IOPixelType ptype = imageIO->GetPixelType();

    switch(ptype)
    {
    case itk::ImageIOBase::RGBA:
      return typeid( itk::RGBAPixel< unsigned char> );
      break;
    case itk::ImageIOBase::RGB:
      return typeid( itk::RGBPixel< unsigned char>);
      break;
    default:
      return imageIO->GetComponentTypeInfo();
    }
  }
}

mitk::PixelType::PixelType( const mitk::PixelType& other )
  : m_ComponentType( other.m_ComponentType ),
    m_PixelType( other.m_PixelType),
    m_ComponentTypeName( other.m_ComponentTypeName ),
    m_PixelTypeName( other.m_PixelTypeName ),
    m_NumberOfComponents( other.m_NumberOfComponents ),
    m_BytesPerComponent( other.m_BytesPerComponent )
{

}

bool mitk::PixelType::operator==(const mitk::PixelType& rhs) const
{
  MITK_DEBUG << "operator==" << std::endl;

  MITK_DEBUG << "m_NumberOfComponents = " << m_NumberOfComponents << " " << rhs.m_NumberOfComponents << std::endl;
  MITK_DEBUG << "m_BytesPerComponent = " << m_BytesPerComponent << " " << rhs.m_BytesPerComponent << std::endl;
  MITK_DEBUG << "m_PixelTypeName = " << m_PixelTypeName << " " << rhs.m_PixelTypeName << std::endl;
  MITK_DEBUG << "m_PixelType = " << m_PixelType << " " << rhs.m_PixelType << std::endl;

  bool returnValue = ( this->m_PixelType == rhs.m_PixelType
                       && this->m_ComponentType == rhs.m_ComponentType
                       && this->m_NumberOfComponents == rhs.m_NumberOfComponents
                       && this->m_BytesPerComponent == rhs.m_BytesPerComponent
                       );

  if(returnValue)
    MITK_DEBUG << " [TRUE] ";
  else
    MITK_DEBUG << " [FALSE] ";

  return returnValue;
}

bool mitk::PixelType::operator!=(const mitk::PixelType& rhs) const
{
  return !(this->operator==(rhs));
}

std::string mitk::PixelType::PixelNameFromItkIOType(ItkIOPixelType ptype)
{
  std::string s;
  switch(ptype)
  {
  case itk::ImageIOBase::SCALAR:
    return (s = "scalar");
  case itk::ImageIOBase::VECTOR:
    return (s = "vector");
  case itk::ImageIOBase::COVARIANTVECTOR:
    return (s = "covariant_vector");
  case itk::ImageIOBase::POINT:
    return (s = "point");
  case itk::ImageIOBase::OFFSET:
    return (s = "offset");
  case itk::ImageIOBase::RGB:
    return (s = "rgb");
  case itk::ImageIOBase::RGBA:
    return (s = "rgba");
  case itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR:
    return (s = "symmetric_second_rank_tensor");
  case itk::ImageIOBase::DIFFUSIONTENSOR3D:
    return (s = "diffusion_tensor_3D");
  case itk::ImageIOBase::COMPLEX:
    return (s = "complex");
  case itk::ImageIOBase::UNKNOWNPIXELTYPE:
    break;
  default:
    mitkThrow() << "Unknown pixel type "<< ptype;
  }
  return (s="unknown");
}

#define SET_ITK_TYPE_ID(anItkIoPixelType_test, numberOfComponents_test, ITK_TYPE)     \
  if ( (itk::ImageIOBase::anItkIoPixelType_test == anItkIoPixelType ) &&              \
       (numberOfComponents_test == m_NumberOfComponents)                              \
     )                                                                                \
  {          *                                                                         \
    m_ItkTypeId = &typeid(ITK_TYPE);                                                  \
  }

#define SET_TYPE(TYPE, IPPIC_TYPE)                                                    \
  if ( *m_TypeId == typeid( TYPE ) )                                                  \
  {                                                                                   \
    m_Type = IPPIC_TYPE;                                                              \
    m_Bpe = sizeof(TYPE) * 8 * m_NumberOfComponents;                                  \
                                                                                      \
    typedef itk::Vector<TYPE, 3> Vector3Type;                                         \
    typedef itk::CovariantVector<TYPE, 3> CovariantVector3Type;                       \
    typedef itk::Point<TYPE, 3> Point3Type;                                           \
    typedef itk::Vector<TYPE, 2> Vector2Type;                                         \
    typedef itk::CovariantVector<TYPE, 2> CovariantVector2Type;                       \
    typedef itk::Point<TYPE, 2> Point2Type;                                           \
                                                                                      \
    SET_ITK_TYPE_ID(UNKNOWNPIXELTYPE, 1, TYPE ) else                                  \
    SET_ITK_TYPE_ID(SCALAR, 1, TYPE ) else                                            \
                                                                                      \
    SET_ITK_TYPE_ID(VECTOR, 2, Vector2Type ) else                                     \
    SET_ITK_TYPE_ID(COVARIANTVECTOR, 2, CovariantVector2Type ) else                   \
    SET_ITK_TYPE_ID(POINT, 2, Point2Type ) else                                       \
                                                                                      \
    SET_ITK_TYPE_ID(RGB, 3, itk::RGBPixel<TYPE> ) else                                \
    /*SET_ITK_TYPE_ID(DIFFUSIONTENSOR3D, 6, itk::DiffusionTensor3D<TYPE> ) else */    \
    SET_ITK_TYPE_ID(VECTOR, 3, Vector3Type ) else                                     \
    SET_ITK_TYPE_ID(COVARIANTVECTOR, 3, CovariantVector3Type ) else                   \
    SET_ITK_TYPE_ID(POINT, 3, Point3Type ) else                                       \
                                                                                      \
    SET_ITK_TYPE_ID(RGBA, 4, itk::RGBAPixel<TYPE> ) else                              \
    {                                                                                 \
    }                                                                                 \
  }                                                                                   \
  else
