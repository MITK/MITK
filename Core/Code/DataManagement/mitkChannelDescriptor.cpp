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
#include "mitkChannelDescriptor.h"
#include "mitkMemoryUtilities.h"

mitk::ChannelDescriptor::ChannelDescriptor( mitk::PixelType type, size_t numOfElements, bool /*allocate*/)
  : m_PixelType(new PixelType(type)), m_Size(numOfElements), m_Data(NULL)
{
  //MITK_INFO << "Entering ChannelDescriptor constructor.";
}

mitk::ChannelDescriptor::~ChannelDescriptor()
{
  // TODO: The following line should be correct but leads to an error.
  // Solution might be: Hold PixelType on stack, return copy and implement
  // copy constructor as well as assignment operator.
  // delete m_PixelType;
}

/*
void mitk::ChannelDescriptor::Initialize(mitk::PixelType &type, size_t numOfElements, bool allocate)
{
  if( m_PixelType.GetPixelTypeId() != type.GetPixelTypeId() )
  {
    MITK_WARN << "Changing pixel type for channel: " <<
                 m_PixelType.GetItkTypeAsString() << " -> " <<
                 type.GetItkTypeAsString();
  }

  m_PixelType = type;

  m_Size = numOfElements * m_PixelType.GetSize();

  if( allocate )
  {
    this->AllocateData();
  }
}
*/

void mitk::ChannelDescriptor::AllocateData()
{
  if( m_Data == NULL)
  {
    m_Data = mitk::MemoryUtilities::AllocateElements<unsigned char>( m_Size );
  }
}
