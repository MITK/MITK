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

#include "mitkIGTLMessageQueue.h"
#include <string>
#include "igtlMessageBase.h"

void mitk::IGTLMessageQueue::PushMessage(igtl::MessageBase::Pointer message)
{
  this->m_Mutex->Lock();
  if (this->m_BufferingType == IGTLMessageQueue::Infinit)
  {
    this->m_Queue.push_back(message);
  }
  else //NoBuffering
  {
    this->m_Queue.clear();
    this->m_Queue.push_back(message);
  }

  this->m_Mutex->Unlock();
}

igtl::MessageBase::Pointer mitk::IGTLMessageQueue::PullMessage()
{
  this->m_Mutex->Lock();
  igtl::MessageBase::Pointer ret = nullptr;
  if (this->m_Queue.size() > 0)
  {
    ret = this->m_Queue.front();
    this->m_Queue.pop_front();
  }
  this->m_Mutex->Unlock();

  return ret;
}

std::string mitk::IGTLMessageQueue::GetNextMsgInformationString()
{
  this->m_Mutex->Lock();
  std::stringstream s;
  if (this->m_Queue.size() > 0)
  {
    s << "Device Type: " << this->m_Queue.front()->GetDeviceType() << std::endl;
    s << "Device Name: " << this->m_Queue.front()->GetDeviceName() << std::endl;
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
  if (this->m_Queue.size() > 0)
  {
    s << this->m_Queue.front()->GetDeviceType();
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
  if (this->m_Queue.size() > 0)
  {
    s << "Device Type: " << this->m_Queue.back()->GetDeviceType() << std::endl;
    s << "Device Name: " << this->m_Queue.back()->GetDeviceName() << std::endl;
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
  if (this->m_Queue.size() > 0)
  {
    s << this->m_Queue.back()->GetDeviceType();
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
  return this->m_Queue.size();
}

void mitk::IGTLMessageQueue::EnableInfiniteBuffering(bool enable)
{
  this->m_Mutex->Lock();
  if (enable)
    this->m_BufferingType = IGTLMessageQueue::Infinit;
  else
    this->m_BufferingType = IGTLMessageQueue::NoBuffering;
  this->m_Mutex->Unlock();
}

mitk::IGTLMessageQueue::IGTLMessageQueue()
{
  this->m_Mutex = itk::FastMutexLock::New();
  this->m_BufferingType = IGTLMessageQueue::Infinit;
}

mitk::IGTLMessageQueue::~IGTLMessageQueue()
{
  this->m_Mutex->Unlock();
}