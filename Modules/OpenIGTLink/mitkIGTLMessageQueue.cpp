/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTLMessageQueue.h"
#include <string>
#include "igtlMessageBase.h"

void mitk::IGTLMessageQueue::PushSendMessage(mitk::IGTLMessage::Pointer message)
{
  this->m_Mutex->Lock();
  if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
    m_SendQueue.clear();

  m_SendQueue.push_back(message);
  this->m_Mutex->Unlock();
}

void mitk::IGTLMessageQueue::PushCommandMessage(igtl::MessageBase::Pointer message)
{
  this->m_Mutex->Lock();
  if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
    m_CommandQueue.clear();

  m_CommandQueue.push_back(message);
  this->m_Mutex->Unlock();
}

void mitk::IGTLMessageQueue::PushMessage(igtl::MessageBase::Pointer msg)
{
  this->m_Mutex->Lock();

  std::stringstream infolog;

  infolog << "Received message of type ";

  if (dynamic_cast<igtl::TrackingDataMessage*>(msg.GetPointer()) != nullptr)
  {
    if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
      m_TrackingDataQueue.clear();

    this->m_TrackingDataQueue.push_back(dynamic_cast<igtl::TrackingDataMessage*>(msg.GetPointer()));

    infolog << "TDATA";
  }
  else if (dynamic_cast<igtl::TransformMessage*>(msg.GetPointer()) != nullptr)
  {
    if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
      m_TransformQueue.clear();

    this->m_TransformQueue.push_back(dynamic_cast<igtl::TransformMessage*>(msg.GetPointer()));

    infolog << "TRANSFORM";
  }
  else if (dynamic_cast<igtl::StringMessage*>(msg.GetPointer()) != nullptr)
  {
    if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
      m_StringQueue.clear();

    this->m_StringQueue.push_back(dynamic_cast<igtl::StringMessage*>(msg.GetPointer()));

    infolog << "STRING";
  }
  else if (dynamic_cast<igtl::ImageMessage*>(msg.GetPointer()) != nullptr)
  {
    igtl::ImageMessage::Pointer imageMsg = dynamic_cast<igtl::ImageMessage*>(msg.GetPointer());
    int* dim = new int[3];
    imageMsg->GetDimensions(dim);
    if (dim[2] > 1)
    {
      if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
        m_Image3dQueue.clear();

      this->m_Image3dQueue.push_back(dynamic_cast<igtl::ImageMessage*>(msg.GetPointer()));

      infolog << "IMAGE3D";
    }
    else
    {
      if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
        m_Image2dQueue.clear();

      this->m_Image2dQueue.push_back(dynamic_cast<igtl::ImageMessage*>(msg.GetPointer()));

      infolog << "IMAGE2D";
    }
  }
  else
  {
    if (this->m_BufferingType == IGTLMessageQueue::NoBuffering)
      m_MiscQueue.clear();

    this->m_MiscQueue.push_back(msg);

    infolog << "OTHER";
  }

  m_Latest_Message = msg;

  //MITK_INFO << infolog.str();

  this->m_Mutex->Unlock();
}

mitk::IGTLMessage::Pointer mitk::IGTLMessageQueue::PullSendMessage()
{
  mitk::IGTLMessage::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_SendQueue.size() > 0)
  {
    ret = this->m_SendQueue.front();
    this->m_SendQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

igtl::MessageBase::Pointer mitk::IGTLMessageQueue::PullMiscMessage()
{
  igtl::MessageBase::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_MiscQueue.size() > 0)
  {
    ret = this->m_MiscQueue.front();
    this->m_MiscQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

igtl::ImageMessage::Pointer mitk::IGTLMessageQueue::PullImage2dMessage()
{
  igtl::ImageMessage::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_Image2dQueue.size() > 0)
  {
    ret = this->m_Image2dQueue.front();
    this->m_Image2dQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

igtl::ImageMessage::Pointer mitk::IGTLMessageQueue::PullImage3dMessage()
{
  igtl::ImageMessage::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_Image3dQueue.size() > 0)
  {
    ret = this->m_Image3dQueue.front();
    this->m_Image3dQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

igtl::TrackingDataMessage::Pointer mitk::IGTLMessageQueue::PullTrackingMessage()
{
  igtl::TrackingDataMessage::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_TrackingDataQueue.size() > 0)
  {
    ret = this->m_TrackingDataQueue.front();
    this->m_TrackingDataQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

igtl::MessageBase::Pointer mitk::IGTLMessageQueue::PullCommandMessage()
{
  igtl::MessageBase::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_CommandQueue.size() > 0)
  {
    ret = this->m_CommandQueue.front();
    this->m_CommandQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

igtl::StringMessage::Pointer mitk::IGTLMessageQueue::PullStringMessage()
{
  igtl::StringMessage::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_StringQueue.size() > 0)
  {
    ret = this->m_StringQueue.front();
    this->m_StringQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

igtl::TransformMessage::Pointer mitk::IGTLMessageQueue::PullTransformMessage()
{
  igtl::TransformMessage::Pointer ret = nullptr;
  this->m_Mutex->Lock();
  if (this->m_TransformQueue.size() > 0)
  {
    ret = this->m_TransformQueue.front();
    this->m_TransformQueue.pop_front();
  }
  this->m_Mutex->Unlock();
  return ret;
}

std::string mitk::IGTLMessageQueue::GetNextMsgInformationString()
{
  this->m_Mutex->Lock();
  std::stringstream s;
  if (this->m_Latest_Message != nullptr)
  {
    s << "Device Type: " << this->m_Latest_Message->GetDeviceType() << std::endl;
    s << "Device Name: " << this->m_Latest_Message->GetDeviceName() << std::endl;
  }
  else
  {
    s << "No Msg";
  }
  this->m_Mutex->Unlock();
  return s.str();
}

std::string mitk::IGTLMessageQueue::GetNextMsgDeviceType()
{
  this->m_Mutex->Lock();
  std::stringstream s;
  if (m_Latest_Message != nullptr)
  {
    s << this->m_Latest_Message->GetDeviceType();
  }
  else
  {
    s << "";
  }
  this->m_Mutex->Unlock();
  return s.str();
}

std::string mitk::IGTLMessageQueue::GetLatestMsgInformationString()
{
  this->m_Mutex->Lock();
  std::stringstream s;
  if (m_Latest_Message != nullptr)
  {
    s << "Device Type: " << this->m_Latest_Message->GetDeviceType() << std::endl;
    s << "Device Name: " << this->m_Latest_Message->GetDeviceName() << std::endl;
  }
  else
  {
    s << "No Msg";
  }
  this->m_Mutex->Unlock();
  return s.str();
}

std::string mitk::IGTLMessageQueue::GetLatestMsgDeviceType()
{
  this->m_Mutex->Lock();
  std::stringstream s;
  if (m_Latest_Message != nullptr)
  {
    s << this->m_Latest_Message->GetDeviceType();
  }
  else
  {
    s << "";
  }
  this->m_Mutex->Unlock();
  return s.str();
}

int mitk::IGTLMessageQueue::GetSize()
{
  return (this->m_CommandQueue.size() + this->m_Image2dQueue.size() + this->m_Image3dQueue.size() + this->m_MiscQueue.size()
    + this->m_StringQueue.size() + this->m_TrackingDataQueue.size() + this->m_TransformQueue.size());
}

void mitk::IGTLMessageQueue::EnableNoBufferingMode(bool enable)
{
  this->m_Mutex->Lock();
  if (enable)
    this->m_BufferingType = IGTLMessageQueue::BufferingType::NoBuffering;
  else
    this->m_BufferingType = IGTLMessageQueue::BufferingType::Infinit;
  this->m_Mutex->Unlock();
}

mitk::IGTLMessageQueue::IGTLMessageQueue()
{
  this->m_Mutex = itk::FastMutexLock::New();
  this->m_BufferingType = IGTLMessageQueue::NoBuffering;
}

mitk::IGTLMessageQueue::~IGTLMessageQueue()
{
  this->m_Mutex->Unlock();
}
