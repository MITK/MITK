/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkIGTLDummyMessage.h"

#include "igtl_header.h"
#include "igtl_util.h"


mitk::IGTLDummyMessage::IGTLDummyMessage() : StringMessage()
{
}

mitk::IGTLDummyMessage::~IGTLDummyMessage()
{
}

void mitk::IGTLDummyMessage::SetDummyString( const std::string& dummyString )
{
  this->m_dummyString = dummyString;
  this->m_String = "This is a dummy string";
}

std::string mitk::IGTLDummyMessage::GetDummyString()
{
  return this->m_dummyString;
}

igtl::MessageBase::Pointer mitk::IGTLDummyMessage::Clone()
{
  //initialize the clone
  mitk::IGTLDummyMessage::Pointer clone = mitk::IGTLDummyMessage::New();

  //copy the data
  clone->SetString(this->GetString());
  clone->SetDummyString(this->GetDummyString());

  return igtl::MessageBase::Pointer(clone.GetPointer());
}

/**
 * \brief Clones the original message interpreted as transform message
 * \param original_ The original message that will be interpreted as transform
 * message
 * \return The clone of the input message
 */
igtl::MessageBase::Pointer mitk::DummyMsgCloneHandler::Clone(igtl::MessageBase* original_)
{
  mitk::IGTLDummyMessage* original = (mitk::IGTLDummyMessage*)original_;
  return original->Clone();
}
