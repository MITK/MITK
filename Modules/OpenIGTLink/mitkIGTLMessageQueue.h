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

#ifndef IGTLMessageQueue_H
#define IGTLMessageQueue_H

#include "MitkOpenIGTLinkExports.h"

#include "itkObject.h"
#include "itkFastMutexLock.h"
#include "mitkCommon.h"

#include <deque>

#include "igtlMessageBase.h"


namespace mitk {
  /**
  * \class IGTLMessageQueue
  * \brief Thread safe message queue to store OpenIGTLink messages.
  */
  class MITK_OPENIGTLINK_EXPORT IGTLMessageQueue : public itk::Object
  {
  public:
    mitkClassMacro(mitk::IGTLMessageQueue, itk::Object)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void PushMessage( igtl::MessageBase::Pointer message );
    igtl::MessageBase::Pointer PullMessage();

    int GetSize();

  protected:
    IGTLMessageQueue();
    virtual ~IGTLMessageQueue();


  protected:
    /**
    * \brief Mutex to take car of the queue
    */
    itk::FastMutexLock::Pointer m_Mutex;

    /**
    * \brief the queue that stores pointer to the inserted messages
    */
    std::deque< igtl::MessageBase::Pointer > m_Queue;
  };
}


#endif
