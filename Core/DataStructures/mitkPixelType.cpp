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


#include "mitkPixelType.h"
#include <itkRGBPixel.h>
#include <itkRGBAPixel.h>

//##ModelId=3E1412720060
mitk::PixelType::PixelType(const mitk::PixelType& aPixelType)
{
  Initialize(aPixelType.GetType(), aPixelType.GetBpe());
}

//##ModelId=3E15F73502BB
mitk::PixelType::PixelType() : m_TypeId(NULL), m_Type(ipPicUnknown), m_Bpe(0)
{

}

//##ModelId=3E1400C40198
mitk::PixelType::PixelType(const std::type_info& aTypeId) : m_TypeId(&aTypeId)
{
  Initialize(aTypeId);
}

//##ModelId=3E1400150088
mitk::PixelType::PixelType(ipPicType_t type, int bpe) : m_Type(type), m_Bpe(bpe)
{
  Initialize(type, bpe);
}

//##ModelId=3E1400060113
mitk::PixelType::PixelType(ipPicDescriptor* pic)
{
  if(pic!=NULL)
    Initialize(pic->type, pic->bpe);
  else
  {
    m_TypeId=NULL;
    //		itkExceptionMacro("pic.IsNull() has no pixel type.");
  }
}

void mitk::PixelType::Initialize(const std::type_info& aTypeId)
{
  m_TypeId = &aTypeId;
  if ( *m_TypeId == typeid(double) )
  {
    m_Type=ipPicFloat;
    m_Bpe=64;
  }
  else if ( *m_TypeId == typeid(float) )
  {
    m_Type=ipPicFloat;
    m_Bpe=32;
  }
  else if ( *m_TypeId == typeid(long) )
  {
    m_Type=ipPicInt;
    m_Bpe=64;
  }
  else if ( *m_TypeId == typeid(unsigned long) )
  {
    m_Type=ipPicUInt;
    m_Bpe=64;
  }
  else if ( *m_TypeId == typeid(int) )
  {
    m_Type=ipPicInt;
    m_Bpe=32;
  }
  else if ( *m_TypeId == typeid(unsigned int) )
  {
    m_Type=ipPicUInt;
    m_Bpe=32;
  }
  else if ( *m_TypeId == typeid(short) )
  {
    m_Type=ipPicInt;
    m_Bpe=16;
  }
  else if ( *m_TypeId == typeid(unsigned short) )
  {
    m_Type=ipPicUInt;
    m_Bpe=16;
  }
  else if ( *m_TypeId == typeid(char) )
  {
    m_Type=ipPicInt;
    m_Bpe=8;
  }
  else if ( *m_TypeId == typeid(unsigned char) )
  {
    m_Type=ipPicUInt;
    m_Bpe=8;
  }
  else if ( *m_TypeId == typeid(itk::RGBPixel<unsigned char>) )
  {
    itkExceptionMacro("Pixel type currently not supported.");
  }
  else if ( *m_TypeId == typeid(itk::RGBAPixel<unsigned char>) )
  {
    itkExceptionMacro("Pixel type currently not supported.");
  }
  else
  {
    itkExceptionMacro("Pixel type currently not supported.");
  }
}

//##ModelId=3E140E4F00E9
void mitk::PixelType::Initialize(ipPicType_t type, int bpe)
{
  m_Type = type;
  m_Bpe = bpe;
  switch(type)
  {
    case ipPicUnknown: m_TypeId=&typeid(void*); break;
    case ipPicBool:    m_TypeId=&typeid(bool); m_Bpe=8; break;
    case ipPicASCII:   m_TypeId=&typeid(char); m_Bpe=8; break;
    case ipPicInt:     switch(bpe)
                      {
    case 8: 		m_TypeId=&typeid(char); break;
    case 16: 		m_TypeId=&typeid(short); break;
    case 32: 		m_TypeId=&typeid(int); break;
    case 64: 		m_TypeId=&typeid(long); break;
    default:		m_TypeId=NULL;
      					itkExceptionMacro("Pixel type currently not supported.");
                      }
                      break;
    case ipPicUInt:    switch(bpe)
                      {
    case 8: 		m_TypeId=&typeid(unsigned char); break;
    case 16: 		m_TypeId=&typeid(unsigned short); break;
    case 32: 		m_TypeId=&typeid(unsigned int); break;
    case 64: 		m_TypeId=&typeid(unsigned long); break;
    default:		m_TypeId=NULL;
        				itkExceptionMacro("Pixel type currently not supported.");
                      }
                      break;
    case ipPicFloat:   switch(bpe)
                      {
    case 32: 		m_TypeId=&typeid(float); break;
    case 64: 		m_TypeId=&typeid(double); break;
    default:		m_TypeId=NULL;
      					itkExceptionMacro("Pixel type currently not supported.");
                      }
                      break;
    case ipPicNonUniform: m_TypeId=&typeid(void*); break;
    case ipPicTSV:     m_TypeId=&typeid(void*); break;
    default:		m_TypeId=NULL;
      				itkExceptionMacro("Pixel type currently not supported.");
      break;
  }
}
