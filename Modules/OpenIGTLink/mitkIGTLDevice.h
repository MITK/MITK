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


#ifndef MITKIGTLDEVICE_H
#define MITKIGTLDEVICE_H

#include <MitkOpenIGTLinkExports.h>
#include "itkObject.h"
#include "mitkCommon.h"
#include "itkFastMutexLock.h"
#include <itkMultiThreader.h>
//#include <vector>
#include "igtlSocket.h"
#include "igtlMessageBase.h"
#include "igtlTransformMessage.h"
#include "mitkIGTLMessageFactory.h"
#include "mitkIGTLMessageQueue.h"


namespace mitk {
    /**Documentation
    * \brief Interface for all Open IGT Link Devices
    *
    * Defines the methods that are common for all devices using Open IGT Link.
    *
    */
    class MITK_OPENIGTLINK_EXPORT IGTLDevice : public itk::Object
    {
    public:
      mitkClassMacro(IGTLDevice, itk::Object)

      /**
       * Type for state variable. The IGTLDevice is always in one of these states
      */
      enum IGTLDeviceState {Setup, Ready, Running};

      /**
       * \brief Opens a connection to the device
       *
       * This may only be called if there is currently no connection to the
       * device. If OpenConnection() is successful, the object will change from
       * Setup state to Ready state
       */
      virtual bool OpenConnection() = 0;

      /**
       * \brief Closes the connection to the device
       *
       * This may only be called if there is currently a connection to the
       * device, but device is not running (e.g. object is in Ready state)
       */
      virtual bool CloseConnection();

      /**
       * \brief Stops the communication between the two devices
       *
       * This may only be called if there the device is in Running state
       */
      virtual bool StopCommunication();

      bool StartCommunication();

      void RunCommunication();

      /**
       * \brief Sends a message via the open IGT Link.
       *
       * This may only be called after the connection to the device has been
       * established with a call to OpenConnection(). Note that the message
       * is not send directly. This method just adds it to the send queue.
       */
      void SendMessage(igtl::MessageBase::Pointer msg);

      /**
       * \brief return current object state (Setup, Ready or Running)
       */
      IGTLDeviceState GetState() const;

      /**
       * \brief Returns the latest received message
       * \param msg A smartpointer to the message base where the latest message
       * shall be copied into
       * \retval true The latest message is stored in msg
       * \retval false The latest message could not been copied, do not use this
       * data
       */
      igtl::MessageBase::Pointer GetLatestMessage();

      /**
       * \brief Returns information about the oldest message in the receive queue
       */
      std::string GetOldestMessageInformation();

      /**
       * \brief return device data
       */
//      igtl::MessageBase::Pointer GetData() const;

      /**
       * \brief set device data
       */
//      void SetData(IGTLDeviceData data);

      /**
       * \brief Sets the port number of the device
       */
      itkSetMacro(PortNumber,int);

      /**
       * \brief Returns the port number of the device
       */
      itkGetMacro(PortNumber,int);

      /**
       * \brief Sets the ip/hostname of the device
       */
      itkSetMacro(Hostname,std::string);

      /**
       * \brief Returns the ip/hostname of the device
       */
      itkGetMacro(Hostname,std::string);

      /**
       * \brief Returns the name of this device
       */
      itkGetConstMacro(Name, std::string);

      /**
       * \brief Sets the name of this device
       */
      itkSetMacro(Name, std::string);

      /**
       * \brief Returns the message factory
       */
      itkGetMacro(MessageFactory, mitk::IGTLMessageFactory::Pointer);

      /**
       * \brief static start method for the tracking thread.
       */
      static ITK_THREAD_RETURN_TYPE ThreadStartCommunication(void* data);

     /**
      * \brief TestConnection() tries to connect to a IGTL server on the current
      * ip and port
      *
      * \todo check this description
      *
      * TestConnection() tries to connect to a IGTL server on the current
      * ip and port and returns which device it has found.
      * \return It returns the type of the device that answers. Throws an exception
      * if no device is available on that ip/port.
      * @throw mitk::IGTHardwareException Throws an exception if there are errors
      * while connecting to the device.
      */
      virtual bool TestConnection();

    protected:
      /**
       * \brief Sends a message via the open IGT Link.
       *
       * This may only be called after the connection to the device has been
       * established with a call to OpenConnection()
       */
      bool SendMessagePrivate(igtl::MessageBase::Pointer msg);


      /**
      * \brief Call this method to receive a message. The message will be saved
      * in the queue
      */
      void Receive();

      /**
      * \brief Call this method to send a message. The message will be read from
      * the queue
      */
      void Send();

      /**
      * \brief  change object state
      */
      void SetState(IGTLDeviceState state);


      IGTLDevice();
      virtual ~IGTLDevice();

//      IGTLDeviceData m_Data; ///< current device Data
      IGTLDeviceState m_State; ///< current object state (Setup, Ready or Running)
      bool m_StopCommunication;       ///< signal stop to thread
      /** mutex to control access to m_StopThread */
      itk::FastMutexLock::Pointer m_StopCommunicationMutex;
      /** mutex to manage control flow of StopTracking() */
      itk::FastMutexLock::Pointer m_CommunicationFinishedMutex;
      /** mutex to control access to m_State */
      itk::FastMutexLock::Pointer m_StateMutex;
      /** mutex to control access to m_Socket */
//      itk::FastMutexLock::Pointer m_SocketMutex;
      /** mutex to control access to m_ReceiveQueue */
//      itk::FastMutexLock::Pointer m_ReceiveQueueMutex;
      /** mutex to control access to m_SendQueue */
//      itk::FastMutexLock::Pointer m_SendQueueMutex;
      /** the hostname or ip of the device */
      std::string m_Hostname;
      /** the port number of the device */
      int m_PortNumber;
      /** the socket used to communicate with other IGTL devices */
      igtl::Socket::Pointer m_Socket;
      /** the latest received message */
//      igtl::MessageBase::Pointer m_LatestMessage;
      /** The message receive queue */
      mitk::IGTLMessageQueue::Pointer m_ReceiveQueue;
      /** The message send queue */
      mitk::IGTLMessageQueue::Pointer m_SendQueue;
      /** the latest received message */
      mitk::IGTLMessageFactory::Pointer m_MessageFactory;
      /** the name of this device */
      std::string m_Name;

    private:
      /** creates worker thread that continuously polls interface for new
      messages */
      itk::MultiThreader::Pointer m_MultiThreader;
      int m_ThreadID; ///< ID of polling thread
    };

    /**Documentation
    * @brief connect to this Event to get noticed when a message was received
    * */
    itkEventMacro( MessageReceivedEvent , itk::AnyEvent );

} // namespace mitk

#endif /* MITKIGTLDEVICE_H */
