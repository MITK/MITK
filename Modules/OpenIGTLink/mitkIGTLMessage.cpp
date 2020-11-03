/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLMessage.h"
#include "mitkException.h"
#include "mitkIGTLMessageCommon.h"

mitk::IGTLMessage::IGTLMessage() : itk::DataObject(),
  m_DataValid(false), m_IGTTimeStamp(0), m_Name()
{
  m_Message = igtl::MessageBase::New();
}


mitk::IGTLMessage::IGTLMessage(const mitk::IGTLMessage& toCopy) :
  itk::DataObject()
{
  // TODO SW: Graft does the same, remove code duplications, set Graft to
  // deprecated, remove duplication in tescode
  this->Graft(&toCopy);
}

mitk::IGTLMessage::~IGTLMessage()
{
}

mitk::IGTLMessage::IGTLMessage(igtl::MessageBase::Pointer message)
{
  this->SetMessage(message);
  this->SetName(message->GetDeviceName());
}

void mitk::IGTLMessage::Graft( const DataObject *data )
{
  // Attempt to cast data to an IGTLMessage
  const Self* msg;
  try
  {
    msg = dynamic_cast<const Self *>(data);
  }
  catch( ... )
  {
    itkExceptionMacro( << "mitk::IGTLMessage::Graft cannot cast "
                       << typeid(data).name() << " to "
                       << typeid(const Self *).name() );
    return;
  }
  if (!msg)
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::IGTLMessage::Graft cannot cast "
                       << typeid(data).name() << " to "
                       << typeid(const Self *).name() );
    return;
  }
  // Now copy anything that is needed
  this->SetMessage(msg->GetMessage());
  this->SetDataValid(msg->IsDataValid());
  this->SetIGTTimeStamp(msg->GetIGTTimeStamp());
  this->SetName(msg->GetName());
}

void mitk::IGTLMessage::SetMessage(igtl::MessageBase::Pointer msg)
{
  m_Message = msg;
  unsigned int ts = 0;
  unsigned int frac = 0;
  m_Message->GetTimeStamp(&ts, &frac); //ts = seconds / frac = nanoseconds
  this->SetName(m_Message->GetDeviceName());
  double timestamp = ts * 1000.0 + frac;
  this->SetIGTTimeStamp(timestamp);
  this->SetDataValid(true);
}

bool mitk::IGTLMessage::IsDataValid() const
{
  return m_DataValid;
}


void mitk::IGTLMessage::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << "name: "           << this->GetName() << std::endl;
  os << indent << "type: "           << this->GetIGTLMessageType() << std::endl;
  os << indent << "valid: "     << this->IsDataValid() << std::endl;
  os << indent << "timestamp: "      << this->GetIGTTimeStamp() << std::endl;
  os << indent << "OpenIGTLinkMessage: " << std::endl;
  m_Message->Print(os);
  this->Superclass::PrintSelf(os, indent);
}

std::string mitk::IGTLMessage::ToString() const
{
  std::stringstream output;
  this->Print(output);
  return output.str();
}


void mitk::IGTLMessage::CopyInformation( const DataObject* data )
{
  this->Superclass::CopyInformation( data );

  const Self * nd = nullptr;
  try
  {
    nd = dynamic_cast<const Self*>(data);
  }
  catch( ... )
  {
    // data could not be cast back down
    itkExceptionMacro(<< "mitk::IGTLMessage::CopyInformation() cannot cast "
                      << typeid(data).name() << " to "
                      << typeid(Self*).name() );
  }
  if ( !nd )
  {
    // pointer could not be cast back down
    itkExceptionMacro(<< "mitk::IGTLMessage::CopyInformation() cannot cast "
                      << typeid(data).name() << " to "
                      << typeid(Self*).name() );
  }
  /* copy all meta data */
}

bool mitk::Equal(const mitk::IGTLMessage& leftHandSide,
                 const mitk::IGTLMessage& rightHandSide,
                 ScalarType /*eps*/, bool verbose)
{
  bool returnValue = true;

  if( std::string(rightHandSide.GetName()) != std::string(leftHandSide.GetName()) )
  {
    if(verbose)
    {
      MITK_INFO << "[( IGTLMessage )] Name differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetName()
                << "rightHandSide is " << rightHandSide.GetName();
    }
    returnValue = false;
  }

  if( rightHandSide.GetIGTTimeStamp() != leftHandSide.GetIGTTimeStamp() )
  {
    if(verbose)
    {
      MITK_INFO << "[( IGTLMessage )] IGTTimeStamp differs.";
      MITK_INFO << "leftHandSide is " << leftHandSide.GetIGTTimeStamp()
                << "rightHandSide is " << rightHandSide.GetIGTTimeStamp();
    }
    returnValue = false;
  }

  return returnValue;
}

const char* mitk::IGTLMessage::GetIGTLMessageType() const
{
  return this->m_Message->GetDeviceType();
}

template < typename IGTLMessageType >
IGTLMessageType* mitk::IGTLMessage::GetMessage() const
{
  return dynamic_cast<IGTLMessageType*>(this->m_Message);
}
