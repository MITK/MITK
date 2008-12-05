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


#include "mitkPixelType.h"
#include <itkVector.h>
#include <itkRGBPixel.h>
#include <itkRGBAPixel.h>
#include <itkCovariantVector.h>
#include "itkDiffusionTensor3D.h"
#include "itkConfidenceDiffusionTensor3D.h"

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

#define _N_VEC(N_DIRS,PIXTYPE)                                                          \
  else if ( *m_TypeId == typeid( itk::Vector<PIXTYPE,N_DIRS> ))  \
  {                                                                                     \
  found = true;                                                                      \
  m_TypeId = & typeid( PIXTYPE );                                                    \
  m_NumberOfComponents *= N_DIRS;                                                \
  m_Type = ipPicFloat;                                                               \
  m_Bpe = sizeof(PIXTYPE) * 8 * m_NumberOfComponents;                                \
  m_ItkTypeId = &typeid( itk::Vector<PIXTYPE,N_DIRS> );     \
  }                                                                                     \


mitk::PixelType::PixelType() : m_TypeId( NULL ), m_Type( ipPicUnknown ), m_Bpe( 0 ), m_NumberOfComponents( 1 )
{

}

mitk::PixelType::PixelType( const mitk::PixelType& aPixelType )
{
  Initialize( *aPixelType.GetTypeId(), aPixelType.GetNumberOfComponents() );
}

mitk::PixelType::PixelType( const std::type_info& aTypeId, int numberOfComponents, ItkIOPixelType anItkIoPixelType ) : m_TypeId( &aTypeId ), m_NumberOfComponents( numberOfComponents )
{
  Initialize( aTypeId, numberOfComponents, anItkIoPixelType );
}

mitk::PixelType::PixelType( ipPicType_t type, int bpe, int numberOfComponents ) : m_Type( type ), m_Bpe( bpe ), m_NumberOfComponents( numberOfComponents )
{
  Initialize( type, bpe, numberOfComponents );
}

mitk::PixelType::PixelType( const ipPicDescriptor* pic ) : m_NumberOfComponents( 1 )
{
   if ( pic != NULL )
      Initialize( pic->type, pic->bpe );
   else
   {
      m_TypeId = NULL;
      //  itkExceptionMacro("pic.IsNull() has no pixel type.");
   }
}

bool mitk::PixelType::operator==(const mitk::PixelType& rhs) const
{
  std::cout << "operator==" << std::endl;

std::cout << "m_Type = " << m_Type << " " << rhs.m_Type << std::endl;
std::cout << "m_Bpe = " << m_Bpe << " " << rhs.m_Bpe << std::endl;
std::cout << "m_NumberOfComponents = " << m_NumberOfComponents << " " << rhs.m_NumberOfComponents << std::endl;
std::cout << "m_BitsPerComponent = " << m_BitsPerComponent << " " << rhs.m_BitsPerComponent << std::endl;

   return (
     *(this->m_TypeId) == *(rhs.m_TypeId)
    && this->m_Type == rhs.m_Type
    && this->m_Bpe == rhs.m_Bpe
    && this->m_NumberOfComponents == rhs.m_NumberOfComponents
    && this->m_BitsPerComponent == rhs.m_BitsPerComponent
    );
}

bool mitk::PixelType::operator!=(const mitk::PixelType& rhs) const
{
  return !(this->operator==(rhs));
}

bool mitk::PixelType::operator==(const std::type_info& typeId) const
{
  if (GetItkTypeId() == NULL)
    return false;
  return (*GetItkTypeId() == typeId);
}

bool mitk::PixelType::operator!=(const std::type_info& typeId) const
{
  return !(this->operator==(typeId));
}

#define SET_ITK_TYPE_ID(anItkIoPixelType_test, numberOfComponents_test, ITK_TYPE)     \
  if ( (itk::ImageIOBase::anItkIoPixelType_test == anItkIoPixelType ) &&              \
       (numberOfComponents_test == m_NumberOfComponents)                              \
     )                                                                                \
  {                                                                                   \
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
    SET_ITK_TYPE_ID(RGB, 3, itk::RGBPixel<TYPE> ) else                                \
    SET_ITK_TYPE_ID(DIFFUSIONTENSOR3D, 6, itk::DiffusionTensor3D<TYPE> ) else         \
    SET_ITK_TYPE_ID(VECTOR, 3, Vector3Type ) else                                     \
    SET_ITK_TYPE_ID(COVARIANTVECTOR, 3, CovariantVector3Type ) else                   \
    SET_ITK_TYPE_ID(POINT, 3, Point3Type ) else                                       \
                                                                                      \
    SET_ITK_TYPE_ID(VECTOR, 2, Vector2Type ) else                                     \
    SET_ITK_TYPE_ID(COVARIANTVECTOR, 2, CovariantVector2Type ) else                   \
    SET_ITK_TYPE_ID(POINT, 2, Point2Type ) else                                       \
    {                                                                                 \
    }                                                                                 \
  }                                                                                   \
  else 

void mitk::PixelType::Initialize( const std::type_info& aTypeId, int numberOfComponents, ItkIOPixelType anItkIoPixelType )
{
   m_TypeId = &aTypeId;
   m_NumberOfComponents = numberOfComponents;
   if(m_NumberOfComponents == 1)
     m_ItkTypeId = &aTypeId;
   else
     m_ItkTypeId = NULL;
   SET_TYPE(double, ipPicFloat)
   SET_TYPE(float, ipPicFloat)
   SET_TYPE(long, ipPicInt)
   SET_TYPE(unsigned long, ipPicUInt)
   SET_TYPE(int, ipPicInt)
   SET_TYPE(unsigned int, ipPicUInt)
   SET_TYPE(short, ipPicInt)
   SET_TYPE(unsigned short, ipPicUInt)
   SET_TYPE(char, ipPicInt)
   SET_TYPE(unsigned char, ipPicUInt)
   if ( *m_TypeId == typeid( itk::DiffusionTensor3D<float> ) )
   {
     m_TypeId = & typeid( float );
     m_NumberOfComponents *= 6;
     m_Type = ipPicFloat;
     m_Bpe = sizeof(float) * 8 * m_NumberOfComponents;
     m_ItkTypeId = &typeid( itk::DiffusionTensor3D<float> );
   }
   else if ( *m_TypeId == typeid( itk::DiffusionTensor3D<double> ) )
   {
     m_TypeId = & typeid( double );
     m_NumberOfComponents *= 6;
     m_Type = ipPicFloat;
     m_Bpe = sizeof(double) * 8 * m_NumberOfComponents;
     m_ItkTypeId = &typeid( itk::DiffusionTensor3D<double> );
   }
   else if ( *m_TypeId == typeid( itk::ConfidenceDiffusionTensor3D<float> ) )
   {
     m_TypeId = & typeid( float );
     m_NumberOfComponents *= 7;
     m_Type = ipPicFloat;
     m_Bpe = sizeof(float) * 8 * m_NumberOfComponents;
     m_ItkTypeId = &typeid( itk::ConfidenceDiffusionTensor3D<float> );
   }
   else if ( *m_TypeId == typeid( itk::ConfidenceDiffusionTensor3D<double> ) )
   {
     m_TypeId = & typeid( double );
     m_NumberOfComponents *= 7;
     m_Type = ipPicFloat;
     m_Bpe = sizeof(double) * 8 * m_NumberOfComponents;
     m_ItkTypeId = &typeid( itk::ConfidenceDiffusionTensor3D<double> );
   }
   else if ( *m_TypeId == typeid( itk::RGBPixel<unsigned char> ) )
   {
     m_Type = ipPicUInt;
     m_NumberOfComponents = 3;
     m_Bpe = sizeof(unsigned char) * 8 * m_NumberOfComponents;
     m_ItkTypeId = &typeid( itk::RGBPixel<unsigned char> );
   }
   else if ( *m_TypeId == typeid( itk::RGBAPixel<unsigned char> ) )
   {
     itkExceptionMacro( "Pixel type currently not supported." );
   }
   else
   {
     bool found = false;
     
     // the following lines allow for fixed-size vector images up to a certain size limit
     // (commented out for shorter compile times)
     //HUNDRED_VECS(000)
     //HUNDRED_VECS(100)
     //HUNDRED_VECS(200)
     //HUNDRED_VECS(300)
     //HUNDRED_VECS(400)
     //HUNDRED_VECS(500)
     //HUNDRED_VECS(600)
     //HUNDRED_VECS(700)

     // allow for fixed-size vectors of specific length
     // (inspired by itkPointshell.cpp, precompiled q-ball configs)
     //TEN_VECS(0)
     //N_VEC(11)
     //N_VEC(12)
     N_VEC(3)
     N_VEC(42)
     N_VEC(92)
     N_VEC(162)
     N_VEC(252)
     N_VEC(362)
     //N_VEC(492)
     //N_VEC(642)
     //N_VEC(812)
     //N_VEC(1002)

     if(!found)
       itkExceptionMacro( "Pixel type currently not supported." );
   }
   m_BitsPerComponent = m_Bpe / m_NumberOfComponents;
}

void mitk::PixelType::Initialize( ipPicType_t type, int bpe, int numberOfComponents )
{
   m_Type = type;
   m_Bpe = bpe;
   m_NumberOfComponents = numberOfComponents;
   switch ( type )
   {
   case ipPicUnknown:
      m_TypeId = &typeid( void* );
      break;
   case ipPicBool:
      m_TypeId = &typeid( bool );
      m_Bpe = sizeof(bool) * 8 * numberOfComponents;
      break;
   case ipPicASCII:
      m_TypeId = &typeid( char );
      m_Bpe = sizeof(char) * 8 * numberOfComponents;
      break;
   case ipPicInt:
      switch ( bpe / numberOfComponents )
      {
      case 8:
         m_TypeId = &typeid( char );
         break;
      case 16:
         m_TypeId = &typeid( short );
         break;
      case 32:
         m_TypeId = &typeid( int );
         break;
      case 64:
         m_TypeId = &typeid( long );
         break;
      default:
         m_TypeId = NULL;
         itkExceptionMacro( "Pixel type currently not supported." );
      }
      break;
   case ipPicUInt:
      switch ( bpe / numberOfComponents )
      {
      case 8:
         m_TypeId = &typeid( unsigned char );
         break;
      case 16:
         m_TypeId = &typeid( unsigned short );
         break;
      case 24:
         m_TypeId = &typeid( unsigned char );
         m_NumberOfComponents = numberOfComponents = 3;
         break;
      case 32:
         m_TypeId = &typeid( unsigned int );
         break;
      case 64:
         m_TypeId = &typeid( unsigned long );
         break;
      default:
         m_TypeId = NULL;
         itkExceptionMacro( "Pixel type currently not supported." );
      }
      break;
   case ipPicFloat:
      switch ( bpe / numberOfComponents )
      {
      case 32:
         m_TypeId = &typeid( float );
         break;
      case 64:
         m_TypeId = &typeid( double );
         break;
      default:
         m_TypeId = NULL;
         itkExceptionMacro( "Pixel type currently not supported." );
      }
      break;
   case ipPicNonUniform:
      m_TypeId = &typeid( void* );
      break;
   case ipPicTSV:
      m_TypeId = &typeid( void* );
      break;
   default:
      m_TypeId = NULL;
      itkExceptionMacro( "Pixel type currently not supported." );
      break;
   }
   m_BitsPerComponent = m_Bpe / numberOfComponents;

   if(m_NumberOfComponents == 1)
     m_ItkTypeId = m_TypeId;
   else
     m_ItkTypeId = NULL;
}

std::string mitk::PixelType::GetItkTypeAsString() const
{
  if (GetItkTypeId()==NULL)
  {
    return "unknown";
  }
  else
  {
    return GetItkTypeId()->name();
  }
}
