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

//OpenIGTLink
#include "igtlMessageBase.h"
#include "igtlImageMessage.h"
#include "igtlStringMessage.h"
#include "igtlTrackingDataMessage.h"
#include "igtlTransformMessage.h"

namespace mitk {
  /**
  * \class IGTLMessageQueue
  * \brief Thread safe message queue to store OpenIGTLink messages.
  *
  * \ingroup OpenIGTLink
  */
  class MITKOPENIGTLINK_EXPORT IGTLMessageQueue : public itk::Object
  {
  public:
    mitkClassMacroItkParent(mitk::IGTLMessageQueue, itk::Object)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /**
       * \brief Different buffering types
       * Infinit buffering means that you can push as many messages as you want
       * NoBuffering means that the queue just stores a single message
       */
    enum BufferingType { Infinit, NoBuffering };

    void PushSendMessage(igtl::MessageBase::Pointer message);

    /**
    * \brief Adds the message to the queue
    */
    void PushMessage(igtl::MessageBase::Pointer message);

    /**
    * \brief Adds the message to the queue
    */
    void PushCommandMessage(igtl::MessageBase::Pointer message);
    /**
    * \brief Returns and removes the oldest message from the queue
    */
    igtl::MessageBase::Pointer PullMiscMessage();
    igtl::ImageMessage::Pointer PullImage2dMessage();
    igtl::ImageMessage::Pointer PullImage3dMessage();
    igtl::TrackingDataMessage::Pointer PullTrackingMessage();
    igtl::MessageBase::Pointer PullCommandMessage();
    igtl::StringMessage::Pointer PullStringMessage();
    igtl::TransformMessage::Pointer PullTransformMessage();
    igtl::MessageBase::Pointer PullSendMessage();

    /**
    * \brief Get the number of messages in the queue
    */
    int GetSize();

    /**
    * \brief Returns a string with information about the oldest message in the
    * queue
    */
    std::string GetNextMsgInformationString();

    /**
    * \brief Returns the device type of the oldest message in the queue
    */
    std::string GetNextMsgDeviceType();

    /**
    * \brief Returns a string with information about the oldest message in the
    * queue
    */
    std::string GetLatestMsgInformationString();

    /**
    * \brief Returns the device type of the oldest message in the queue
    */
    std::string GetLatestMsgDeviceType();

    /**
    * \brief Sets infinite buffering on/off.
    * Initiale value is enabled.
    */
    void EnableInfiniteBuffering(bool enable);

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
    std::deque< igtl::MessageBase::Pointer > m_CommandQueue;
    std::deque< igtl::ImageMessage::Pointer > m_Image2dQueue;
    std::deque< igtl::ImageMessage::Pointer > m_Image3dQueue;
    std::deque< igtl::TransformMessage::Pointer > m_TransformQueue;
    std::deque< igtl::TrackingDataMessage::Pointer > m_TrackingDataQueue;
    std::deque< igtl::StringMessage::Pointer > m_StringQueue;
    std::deque< igtl::MessageBase::Pointer > m_MiscQueue;

    std::deque< igtl::MessageBase::Pointer> m_SendQueue;

    igtl::MessageBase::Pointer m_Latest_Message;

    /**
    * \brief defines the kind of buffering
    */
    BufferingType m_BufferingType;
  };
}

#endif
