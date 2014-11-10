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

#include "mitkIGTLMessageFactory.h"

// IGT message types
#include "igtlImageMessage.h"
#include "igtlTransformMessage.h"
#include "igtlPositionMessage.h"
#include "igtlStatusMessage.h"

//own types
#include "mitkIGTLDummyMessage.h"

#include "mitkIGTLMessageCommon.h"

#include "itksys/SystemTools.hxx"


//------------------------------------------------------------
// Define message clone classes
// igtlMessageHandlerClassMacro() defines a child class of
// igtl::MessageHandler to handle OpenIGTLink messages for
// the message type specified as the first argument. The
// second argument will be used for the name of this
// message handler class, while the third argument specifies
// a type of data that will be shared with the message functions
// of this handler class.

mitkIGTMessageCloneClassMacro(igtl::TransformMessage, TransformMsgCloneHandler);

/**
 * \brief Clones the original message interpreted as transform message
 * \param original_ The original message that will be interpreted as transform
 * message
 * \return The clone of the input message
 */
igtl::MessageBase::Pointer TransformMsgCloneHandler::Clone(igtl::MessageBase* original_)
{
  bool copySuccess = false;
  igtl::TransformMessage::Pointer clone_ = igtl::TransformMessage::New();

  //initialize the clone
  //  clone = igtl::MessageBase::New();

  igtl::TransformMessage* original = (igtl::TransformMessage*)original_;

  //copy all meta data
  copySuccess = clone_->Copy(original);

  if ( !copySuccess )
    return NULL;

  //copy all data that is important for this class
  //copy the matrix
  igtl::Matrix4x4 mat;
  original->GetMatrix(mat);
  clone_->SetMatrix(mat);

  //copy the normals
  float normals[3][3];
  original->GetNormals(normals);
  clone_->SetNormals(normals);

  //copy the position
  float position[3];
  original->GetPosition(position);
  clone_->SetPosition(position);

  return igtl::MessageBase::Pointer(clone_.GetPointer());
}


mitk::IGTLMessageFactory::IGTLMessageFactory()
{
  //create clone handlers
//  mitk::IGTLMessageCloneHandler::Pointer tmch = ;

  this->AddMessageType("NONE", NULL, NULL);

  //OpenIGTLink Types V1
//  this->AddMessageType("IMAGE", (PointerToCloneMessageFunction)&igtl::ImageMessage::New);
  this->AddMessageType("TRANSFORM", (PointerToMessageBaseNew)&igtl::TransformMessage::New, TransformMsgCloneHandler::New().GetPointer());
//  this->AddMessageType("POSITION", (PointerToCloneMessageFunction)&igtl::PositionMessage::New);
//  this->AddMessageType("STATUS", (PointerToCloneMessageFunction)&igtl::StatusMessage::New);
//  this->AddMessageType("CAPABILITY", (PointerToCloneMessageFunction)&igtl::StatusMessage::New);

//  //OpenIGTLink Types V2
//  this->AddMessageType("IMGMETA", (PointerToCloneMessageFunction)&igtl::IGTLDummyMessage::New);
//  this->AddMessageType("LBMETA", (PointerToCloneMessageFunction)&igtl::PlusTrackedFrameMessage::New);
//  this->AddMessageType("COLORT", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("POINT", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("TRAJ", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("TDATA", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("QTDATA", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("SENSOR", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("STRING", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("NDARRAY", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("BIND", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);
//  this->AddMessageType("POLYDATA", (PointerToCloneMessageFunction)&igtl::PlusUsMessage::New);

  //Own Types
  this->AddMessageType("DUMMY", (PointerToMessageBaseNew)&mitk::IGTLDummyMessage::New, DummyMsgCloneHandler::New().GetPointer());
}

mitk::IGTLMessageFactory::~IGTLMessageFactory()
{
}

void mitk::IGTLMessageFactory::AddMessageType(std::string messageTypeName,
           IGTLMessageFactory::PointerToMessageBaseNew messageTypeNewPointer,
           mitk::IGTLMessageCloneHandler::Pointer cloneHandler)
{
  this->AddMessageNewMethod(messageTypeName, messageTypeNewPointer);
  this->AddMessageCloneHandler(messageTypeName, cloneHandler);
}

void mitk::IGTLMessageFactory::AddMessageNewMethod(std::string messageTypeName,
           IGTLMessageFactory::PointerToMessageBaseNew messageTypeNewPointer )
{
  this->m_NewMethods[messageTypeName] = messageTypeNewPointer;
}

void mitk::IGTLMessageFactory::AddMessageCloneHandler(std::string msgTypeName,
          mitk::IGTLMessageCloneHandler::Pointer cloneHandler )
{
  this->m_CloneHandlers[msgTypeName] = cloneHandler;
}

mitk::IGTLMessageCloneHandler::Pointer
mitk::IGTLMessageFactory::GetCloneHandler(std::string messageTypeName)
{
  if ( this->m_CloneHandlers.find(messageTypeName) !=
       this->m_CloneHandlers.end() )
  {
    return m_CloneHandlers[messageTypeName];
  }

  MITK_ERROR("IGTLMessageFactory") << messageTypeName <<
                              " message type is not registered to factory!";

  mitkThrow() << messageTypeName << " message type is not registered to factory!";

  return NULL;
}

igtl::MessageBase::Pointer
mitk::IGTLMessageFactory::Clone(igtl::MessageBase::Pointer msg)
{
  return this->GetCloneHandler(msg->GetDeviceType())->Clone(msg);
}

mitk::IGTLMessageFactory::PointerToMessageBaseNew
mitk::IGTLMessageFactory::GetMessageTypeNewPointer(std::string messageTypeName)
{
  if ( this->m_NewMethods.find(messageTypeName) != this->m_NewMethods.end() )
  {
    return m_NewMethods[messageTypeName];
  }

  MITK_ERROR("IGTLMessageFactory") << messageTypeName <<
                                   " message type is not registered to factory!";
  return NULL;
}

igtl::MessageBase::Pointer
mitk::IGTLMessageFactory::CreateInstance(igtl::MessageHeader::Pointer msgHeader)
{
  std::string messageType;

  //check the header
  if ( msgHeader.IsNull() )
  {
    messageType = "NONE";
  }
  else
  {
    messageType = msgHeader->GetDeviceType();
  }

  //make message type uppercase
  messageType = itksys::SystemTools::UpperCase(messageType);

  //find the according new method
  if ( this->m_NewMethods.find( messageType )
       != this->m_NewMethods.end() )
  {
    if ( this->m_NewMethods[messageType] != NULL)
    {
      // Call tracker New() function if tracker not NULL
      return (*this->m_NewMethods[messageType])();
    }
    else
      return NULL;
  }
  else
  {
    MITK_ERROR("IGTLMessageFactory") << "Unknown IGT message type: "
                                     << messageType;
    return NULL;
  }
}
