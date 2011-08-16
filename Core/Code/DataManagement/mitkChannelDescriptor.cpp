#include "mitkChannelDescriptor.h"
#include "mitkMemoryUtilities.h"

mitk::ChannelDescriptor::ChannelDescriptor()
  : m_Data(NULL), m_Name(NULL)
{
  this->m_Size = 0;
}

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

void mitk::ChannelDescriptor::AllocateData()
{
  if( m_Data == NULL)
  {
    m_Data = mitk::MemoryUtilities::AllocateElements<unsigned char>( m_Size );
  }
}
