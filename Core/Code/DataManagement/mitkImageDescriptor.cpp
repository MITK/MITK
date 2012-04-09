#include "mitkImageDescriptor.h"

mitk::ImageDescriptor::ImageDescriptor()
{
  //initialize the dimensions array
  for(unsigned int i=0; i<MAX_IMAGE_DIMENSIONS; i++)
  {
    m_Dimensions[i] = 1;
  }

  this->m_NumberOfChannels = 0;
}

// FIXME manage memory flag
void mitk::ImageDescriptor::AddNewChannel(mitk::PixelType ptype, const char *name)
{
  size_t elems = 1;

  for( unsigned int i=0; i<this->m_NumberOfDimensions; i++)
    elems *= this->m_Dimensions[i];

  mitk::ChannelDescriptor desc(ptype, elems);

  this->m_ChannelDesc.push_back( desc );

  if( name == 0)
    m_ChannelNames.push_back( "Unnamed ["+ptype.GetItkTypeAsString()+"]");
  else
    m_ChannelNames.push_back(name);

  this->m_NumberOfChannels++;

  return;
}

void mitk::ImageDescriptor::Initialize(const ImageDescriptor::Pointer refDescriptor, unsigned int channel)
{
  // initialize the members holding dimension information
  this->m_NumberOfChannels = refDescriptor->GetNumberOfChannels();
  this->m_NumberOfDimensions = refDescriptor->GetNumberOfDimensions();
  const unsigned int *refDims = refDescriptor->GetDimensions();

  // copy the dimension information
  for( unsigned int i=0; i< this->m_NumberOfDimensions; i++)
  {
    this->m_Dimensions[i] = refDims[i];
  }

  // get the channel descriptor and store them and so the name of the channel
  mitk::ChannelDescriptor desc = refDescriptor->GetChannelDescriptor(channel);
  this->m_ChannelDesc.push_back( desc );
  this->m_ChannelNames.push_back( refDescriptor->GetChannelName(channel) );
}

void mitk::ImageDescriptor::Initialize(const unsigned int *dims, const unsigned int dim)
{
  this->m_NumberOfDimensions = dim;

  // copy the dimension information
  for( unsigned int i=0; i< this->m_NumberOfDimensions; i++)
  {
    this->m_Dimensions[i] = dims[i];
  }
}

mitk::ChannelDescriptor mitk::ImageDescriptor::GetChannelDescriptor(unsigned int id) const
{
  return this->m_ChannelDesc[id];
}

mitk::PixelType mitk::ImageDescriptor::GetChannelTypeByName(const char *name) const
{
  unsigned int idFound = 0;
  const std::string search_str(name);

  for( ConstChannelNamesIter iter = this->m_ChannelNames.begin();
       iter < this->m_ChannelNames.end(); iter++)
  {
    if( search_str.compare( *iter ) ) idFound = iter - this->m_ChannelNames.begin();
  }

  return (m_ChannelDesc[idFound]).GetPixelType();
}

mitk::PixelType mitk::ImageDescriptor::GetChannelTypeById(const unsigned int id) const
{
  if( id > this->m_NumberOfChannels )
  {
    throw std::invalid_argument("The given id exceeds the number of active channel.");
  }
  else
  {
    mitk::ChannelDescriptor refDesc = this->m_ChannelDesc[id];
    return refDesc.GetPixelType();//
  }
}

const std::string mitk::ImageDescriptor::GetChannelName(unsigned int id) const
{
  if( id > this->m_ChannelNames.size() )
    return "Out-of-range-access";
  else
    return this->m_ChannelNames.at( id );
}

