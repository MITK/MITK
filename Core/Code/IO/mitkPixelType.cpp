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


mitk::PixelType::PixelType( const mitk::PixelType& other )
  : m_ComponentType( other.m_ComponentType ),
    m_PixelType( other.m_PixelType),
    m_ComponentTypeName( other.m_ComponentTypeName ),
    m_PixelTypeName( other.m_PixelTypeName ),
    m_NumberOfComponents( other.m_NumberOfComponents ),
    m_BytesPerComponent( other.m_BytesPerComponent )
{
}

mitk::PixelType& mitk::PixelType::operator=(const PixelType& other)
{

  m_ComponentType = other.m_ComponentType;
  m_PixelType = other.m_PixelType;
  m_ComponentTypeName = other.m_ComponentTypeName;
  m_PixelTypeName = other.m_PixelTypeName;
  m_NumberOfComponents = other.m_NumberOfComponents;
  m_BytesPerComponent = other.m_BytesPerComponent;

  return *this;
}

itk::ImageIOBase::IOPixelType mitk::PixelType::GetPixelType() const
{
  return m_PixelType;
}

int mitk::PixelType::GetComponentType() const
{
  return m_ComponentType;
}

std::string mitk::PixelType::GetPixelTypeAsString() const
{
  return m_PixelTypeName;
}

std::string mitk::PixelType::GetComponentTypeAsString() const
{
  return m_ComponentTypeName;
}

std::string mitk::PixelType::GetTypeAsString() const
{
  return m_PixelTypeName + " (" + m_ComponentTypeName + ")";
}

size_t mitk::PixelType::GetSize() const
{
  return (m_NumberOfComponents * m_BytesPerComponent);
}

size_t mitk::PixelType::GetBpe() const
{
  return this->GetSize() * 8;
}

size_t mitk::PixelType::GetNumberOfComponents() const
{
  return m_NumberOfComponents;
}

size_t mitk::PixelType::GetBitsPerComponent() const
{
  return m_BytesPerComponent * 8;
}

mitk::PixelType::~PixelType()
{}

mitk::PixelType::PixelType( const int componentType,
                            const ItkIOPixelType pixelType,
                            std::size_t bytesPerComponent,
                            std::size_t numberOfComponents,
                            const std::string& componentTypeName,
                            const std::string& pixelTypeName)
  : m_ComponentType( componentType ),
    m_PixelType( pixelType ),
    m_ComponentTypeName(componentTypeName),
    m_PixelTypeName(pixelTypeName),
    m_NumberOfComponents( numberOfComponents ),
    m_BytesPerComponent( bytesPerComponent )
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
