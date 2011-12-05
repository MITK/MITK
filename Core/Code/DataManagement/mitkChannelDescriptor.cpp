#include "mitkChannelDescriptor.h"
#include "mitkMemoryUtilities.h"

mitk::ChannelDescriptor::ChannelDescriptor( mitk::PixelType type, size_t numOfElements, bool allocate)
  : m_PixelType(NULL), m_Size(numOfElements), m_Data(NULL)
{
  m_PixelType = new PixelType(type);

  //MITK_INFO << "Entering ChannelDescriptor constructor.";
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
