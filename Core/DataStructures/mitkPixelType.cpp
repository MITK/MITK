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
    SET_ITK_TYPE_ID(DIFFUSIONTENSOR3D, 6, itk::DiffusionTensor3D<TYPE> ) else                                \
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
   if ( *m_TypeId == typeid( itk::Vector<double,3> ) )
   {
      m_TypeId = & typeid( double );
      m_NumberOfComponents *= 3;
      m_Type = ipPicFloat;
      m_Bpe = sizeof(double) * 8 * m_NumberOfComponents;
      m_ItkTypeId = &typeid( itk::Vector<double,3> );
   }
   else if ( *m_TypeId == typeid( itk::Vector<float,3> ) )
   {
      m_TypeId = & typeid( float );
      m_NumberOfComponents *= 3;
      m_Type = ipPicFloat;
      m_Bpe = sizeof(float) * 8 * m_NumberOfComponents;
      m_ItkTypeId = &typeid( itk::Vector<float,3> );
   }
   else if ( *m_TypeId == typeid( itk::Vector<double,2> ) )
   {
      m_TypeId = & typeid( double );
      m_NumberOfComponents *= 2;
      m_Type = ipPicFloat;
      m_Bpe = sizeof(double) * 8 * m_NumberOfComponents;
      m_ItkTypeId = &typeid( itk::Vector<double,2> );
   }
   else if ( *m_TypeId == typeid( itk::Vector<float,2> ) )
   {
     m_TypeId = & typeid( float );
     m_NumberOfComponents *= 2;
     m_Type = ipPicFloat;
     m_Bpe = sizeof(float) * 8 * m_NumberOfComponents;
     m_ItkTypeId = &typeid( itk::Vector<float,2> );
   }
   else if ( *m_TypeId == typeid( itk::DiffusionTensor3D<float> ) )
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
