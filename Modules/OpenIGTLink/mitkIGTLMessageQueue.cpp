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
  this->m_Mutex->Delete();
}
