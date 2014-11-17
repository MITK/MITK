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



void mitk::IGTLMessageQueue::PushMessage( igtl::MessageBase::Pointer message )
{
  this->m_Mutex->Lock();
  this->m_Queue.push_back( message );
  this->m_Mutex->Unlock();
}

igtl::MessageBase::Pointer mitk::IGTLMessageQueue::PullMessage()
{
  this->m_Mutex->Lock();
  igtl::MessageBase::Pointer ret = NULL;
  if ( this->m_Queue.size() > 0 )
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
  if ( this->m_Queue.size() > 0 )
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
  if ( this->m_Queue.size() > 0 )
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
  if ( this->m_Queue.size() > 0 )
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
  if ( this->m_Queue.size() > 0 )
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

mitk::IGTLMessageQueue::IGTLMessageQueue()
{
  this->m_Mutex = itk::FastMutexLock::New();
}

mitk::IGTLMessageQueue::~IGTLMessageQueue()
{
  this->m_Mutex->Unlock();
}
