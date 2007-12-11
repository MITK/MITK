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

//##ModelId=3E1412720060
mitk::PixelType::PixelType( const mitk::PixelType& aPixelType )
{
  Initialize( *aPixelType.GetTypeId(), aPixelType.GetNumberOfComponents() );
}

//##ModelId=3E15F73502BB
mitk::PixelType::PixelType() : m_TypeId( NULL ), m_Type( ipPicUnknown ), m_Bpe( 0 ), m_NumberOfComponents( 1 )
{}

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


mitk::PixelType::PixelType( const std::type_info& aTypeId, int numberOfComponents ) : m_TypeId( &aTypeId ), m_NumberOfComponents( numberOfComponents )
{
   Initialize( aTypeId, numberOfComponents );
}

//##ModelId=3E1400150088
mitk::PixelType::PixelType( ipPicType_t type, int bpe, int numberOfComponents ) : m_Type( type ), m_Bpe( bpe ), m_NumberOfComponents( numberOfComponents )
{
   Initialize( type, bpe, numberOfComponents );
}

//##ModelId=3E1400060113
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

void mitk::PixelType::Initialize( const std::type_info& aTypeId, int numberOfComponents )
{
   m_TypeId = &aTypeId;
   m_NumberOfComponents = numberOfComponents;
   if ( *m_TypeId == typeid( double ) )
   {
      m_Type = ipPicFloat;
      m_Bpe = sizeof(double) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( float ) )
   {
      m_Type = ipPicFloat;
      m_Bpe = sizeof(float) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( long ) )
   {
      m_Type = ipPicInt;
      m_Bpe = sizeof(long) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( unsigned long ) )
   {
      m_Type = ipPicUInt;
      m_Bpe = sizeof(unsigned long) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( int ) )
   {
      m_Type = ipPicInt;
      m_Bpe = sizeof(int) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( unsigned int ) )
   {
      m_Type = ipPicUInt;
      m_Bpe = sizeof(unsigned int) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( short ) )
   {
      m_Type = ipPicInt;
      m_Bpe = sizeof(short) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( unsigned short ) )
   {
      m_Type = ipPicUInt;
      m_Bpe = sizeof(unsigned short) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( char ) )
   {
      m_Type = ipPicInt;
      m_Bpe = sizeof(char) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( unsigned char ) )
   {
      m_Type = ipPicUInt;
      m_Bpe = sizeof(unsigned char) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( itk::Vector<double,3> ) )
   {
      m_TypeId = & typeid( double );
      m_NumberOfComponents *= 3;
      m_Type = ipPicFloat;
      m_Bpe = sizeof(double) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( itk::Vector<float,3> ) )
   {
      m_TypeId = & typeid( float );
      m_NumberOfComponents *= 3;
      m_Type = ipPicFloat;
      m_Bpe = sizeof(float) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( itk::Vector<double,2> ) )
   {
      m_TypeId = & typeid( double );
      m_NumberOfComponents *= 2;
      m_Type = ipPicFloat;
      m_Bpe = sizeof(double) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( itk::Vector<float,2> ) )
   {
      m_TypeId = & typeid( float );
      m_NumberOfComponents *= 2;
      m_Type = ipPicFloat;
      m_Bpe = sizeof(float) * 8 * m_NumberOfComponents;
   }
   else if ( *m_TypeId == typeid( itk::RGBPixel<unsigned char> ) )
   {
      m_Type = ipPicUInt;
      m_NumberOfComponents = 3;
      m_Bpe = sizeof(unsigned char) * 8 * m_NumberOfComponents;
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

//##ModelId=3E140E4F00E9
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
}


