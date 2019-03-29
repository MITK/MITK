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
#include "igtlImageMetaMessage.h"
#include "igtlPointMessage.h"
#include "igtlTrajectoryMessage.h"
#include "igtlStringMessage.h"
#include "igtlSensorMessage.h"
#include "igtlBindMessage.h"
#include "igtlPolyDataMessage.h"
#include "igtlQuaternionTrackingDataMessage.h"
#include "igtlCapabilityMessage.h"
#include "igtlNDArrayMessage.h"
#include "igtlTrackingDataMessage.h"
#include "igtlColorTableMessage.h"
#include "igtlLabelMetaMessage.h"

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

mitkIGTMessageCloneClassMacro(igtl::TransformMessage, TransformMsgCloneHandler)

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

  if (!copySuccess)
    return nullptr;

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

  this->AddMessageNewMethod("NONE", nullptr);

  // OpenIGTLink Types V1
  this->AddMessageNewMethod("IMAGE", (PointerToMessageBaseNew)&igtl::ImageMessage::New);
  this->AddMessageNewMethod("TRANSFORM", (PointerToMessageBaseNew)&igtl::TransformMessage::New);
  this->AddMessageNewMethod("POSITION", (PointerToMessageBaseNew)&igtl::PositionMessage::New);
  this->AddMessageNewMethod("STATUS", (PointerToMessageBaseNew)&igtl::StatusMessage::New);
  this->AddMessageNewMethod("CAPABILITY", (PointerToMessageBaseNew)&igtl::CapabilityMessage::New);

  this->AddMessageNewMethod("GET_IMAGE", (PointerToMessageBaseNew)&igtl::GetImageMessage::New);
  this->AddMessageNewMethod("GET_TRANS", (PointerToMessageBaseNew)&igtl::GetTransformMessage::New);
  //this->AddMessageNewMethod("GET_POS", (PointerToMessageBaseNew)&igtl::GetPositionMessage::New); //not available???
  this->AddMessageNewMethod("GET_STATUS", (PointerToMessageBaseNew)&igtl::GetStatusMessage::New);

  // OpenIGTLink Types V2
  this->AddMessageNewMethod("IMGMETA", (PointerToMessageBaseNew)&igtl::ImageMetaMessage::New);
  this->AddMessageNewMethod("LBMETA", (PointerToMessageBaseNew)&igtl::LabelMetaMessage::New);
  this->AddMessageNewMethod("COLORT", (PointerToMessageBaseNew)&igtl::ColorTableMessage::New);
  this->AddMessageNewMethod("POINT", (PointerToMessageBaseNew)&igtl::PointMessage::New);
  this->AddMessageNewMethod("TRAJ", (PointerToMessageBaseNew)&igtl::TrajectoryMessage::New);
  this->AddMessageNewMethod("TDATA", (PointerToMessageBaseNew)&igtl::TrackingDataMessage::New);
  this->AddMessageNewMethod("QTDATA", (PointerToMessageBaseNew)&igtl::QuaternionTrackingDataMessage::New);
  this->AddMessageNewMethod("SENSOR", (PointerToMessageBaseNew)&igtl::SensorMessage::New);
  this->AddMessageNewMethod("STRING", (PointerToMessageBaseNew)&igtl::StringMessage::New);
  this->AddMessageNewMethod("NDARRAY", (PointerToMessageBaseNew)&igtl::NDArrayMessage::New);
  this->AddMessageNewMethod("BIND", (PointerToMessageBaseNew)&igtl::BindMessage::New);
  this->AddMessageNewMethod("POLYDATA", (PointerToMessageBaseNew)&igtl::PolyDataMessage::New);

  this->AddMessageNewMethod("GET_IMGMETA", (PointerToMessageBaseNew)&igtl::GetImageMetaMessage::New);
  this->AddMessageNewMethod("GET_LBMETA", (PointerToMessageBaseNew)&igtl::GetLabelMetaMessage::New);
  this->AddMessageNewMethod("GET_COLORT", (PointerToMessageBaseNew)&igtl::GetColorTableMessage::New);
  this->AddMessageNewMethod("GET_POINT", (PointerToMessageBaseNew)&igtl::GetPointMessage::New);
  this->AddMessageNewMethod("GET_TRAJ", (PointerToMessageBaseNew)&igtl::GetTrajectoryMessage::New);
  //  this->AddMessageNewMethod("GET_TDATA",      (PointerToMessageBaseNew)&igtl::GetTrackingDataMessage::New); //not available???
  //  this->AddMessageNewMethod("GET_QTDATA",     (PointerToMessageBaseNew)&igtl::GetQuaternionTrackingDataMessage::New); //not available???
  //  this->AddMessageNewMethod("GET_SENSOR",     (PointerToMessageBaseNew)&igtl::GetSensorMessage::New); //not available???
  //  this->AddMessageNewMethod("GET_STRING",     (PointerToMessageBaseNew)&igtl::GetStringMessage::New); //not available???
  //  this->AddMessageNewMethod("GET_NDARRAY",    (PointerToMessageBaseNew)&igtl::GetNDArrayMessage::New); //not available???
  this->AddMessageNewMethod("GET_BIND", (PointerToMessageBaseNew)&igtl::GetBindMessage::New);
  this->AddMessageNewMethod("GET_POLYDATA", (PointerToMessageBaseNew)&igtl::GetPolyDataMessage::New);

  this->AddMessageNewMethod("RTS_BIND", (PointerToMessageBaseNew)&igtl::RTSBindMessage::New);
  this->AddMessageNewMethod("RTS_QTDATA", (PointerToMessageBaseNew)&igtl::RTSQuaternionTrackingDataMessage::New);
  this->AddMessageNewMethod("RTS_TDATA", (PointerToMessageBaseNew)&igtl::RTSTrackingDataMessage::New);
  //todo: check if there are more RTS messages

  this->AddMessageNewMethod("STT_BIND", (PointerToMessageBaseNew)&igtl::StartBindMessage::New);
  this->AddMessageNewMethod("STT_TDATA", (PointerToMessageBaseNew)&igtl::StartTrackingDataMessage::New);
  this->AddMessageNewMethod("STT_QTDATA", (PointerToMessageBaseNew)&igtl::StartQuaternionTrackingDataMessage::New);
  //todo: check if there are more STT messages

  this->AddMessageNewMethod("STP_BIND", (PointerToMessageBaseNew)&igtl::StopBindMessage::New);
  this->AddMessageNewMethod("STP_TDATA", (PointerToMessageBaseNew)&igtl::StopTrackingDataMessage::New);
  this->AddMessageNewMethod("STP_QTDATA", (PointerToMessageBaseNew)&igtl::StopQuaternionTrackingDataMessage::New);
  //todo: check if there are more STP messages

  //Own Types
  this->AddMessageNewMethod("DUMMY", (PointerToMessageBaseNew)&mitk::IGTLDummyMessage::New);
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
  IGTLMessageFactory::PointerToMessageBaseNew messageTypeNewPointer)
{
  this->m_NewMethods[messageTypeName] = messageTypeNewPointer;
}

void mitk::IGTLMessageFactory::AddMessageCloneHandler(std::string msgTypeName,
  mitk::IGTLMessageCloneHandler::Pointer cloneHandler)
{
  this->m_CloneHandlers[msgTypeName] = cloneHandler;
}

mitk::IGTLMessageCloneHandler::Pointer
mitk::IGTLMessageFactory::GetCloneHandler(std::string messageTypeName)
{
  if (this->m_CloneHandlers.find(messageTypeName) !=
    this->m_CloneHandlers.end())
  {
    return m_CloneHandlers[messageTypeName];
  }

  MITK_ERROR("IGTLMessageFactory") << messageTypeName <<
    " message type is not registered to factory!";

  mitkThrow() << messageTypeName << " message type is not registered to factory!";

  return nullptr;
}

igtl::MessageBase::Pointer
mitk::IGTLMessageFactory::Clone(igtl::MessageBase::Pointer msg)
{
  return this->GetCloneHandler(msg->GetDeviceType())->Clone(msg);
}

mitk::IGTLMessageFactory::PointerToMessageBaseNew
mitk::IGTLMessageFactory::GetMessageTypeNewPointer(std::string messageTypeName)
{
  if (this->m_NewMethods.find(messageTypeName) != this->m_NewMethods.end())
  {
    return m_NewMethods[messageTypeName];
  }

  MITK_ERROR("IGTLMessageFactory") << messageTypeName <<
    " message type is not registered to factory!";
  return nullptr;
}

igtl::MessageBase::Pointer
mitk::IGTLMessageFactory::CreateInstance(std::string messageTypeName)
{
  mitk::IGTLMessageFactory::PointerToMessageBaseNew newPointer =
    this->GetMessageTypeNewPointer(messageTypeName);
  if (newPointer != nullptr)
  {
    return newPointer();
  }
  else
  {
    return nullptr;
  }
}

std::list<std::string>
mitk::IGTLMessageFactory::GetAvailableMessageRequestTypes()
{
  std::list<std::string> allGetMessages;
  for (std::map<std::string, PointerToMessageBaseNew>::const_iterator it =
    this->m_NewMethods.begin();
    it != this->m_NewMethods.end(); ++it)
  {
    if (it->first.find("GET_") != std::string::npos ||
      it->first.find("STT_") != std::string::npos ||
      it->first.find("STP_") != std::string::npos ||
      it->first.find("RTS_") != std::string::npos)
    {
      allGetMessages.push_back(it->first);
    }
  }

  return allGetMessages;
}

igtl::MessageBase::Pointer
mitk::IGTLMessageFactory::CreateInstance(igtl::MessageHeader::Pointer msgHeader)
{
  std::string messageType;

  //check the header
  if (msgHeader.IsNull())
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
  if (this->m_NewMethods.find(messageType)
    != this->m_NewMethods.end())
  {
    if (this->m_NewMethods[messageType] != nullptr)
    {
      // Call tracker New() function if tracker not nullptr
      return (*this->m_NewMethods[messageType])();
    }
    else
      return nullptr;
  }
  else
  {
    MITK_ERROR("IGTLMessageFactory") << "Unknown IGT message type: "
      << messageType;
    return nullptr;
  }
}
