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

#include "mitkCommon.h"

//itk
#include "itkObject.h"
#include "itkFastMutexLock.h"
#include "itkMultiThreader.h"

//igtl
#include "igtlSocket.h"
#include "igtlMessageBase.h"
#include "igtlTransformMessage.h"

//mitkIGTL
#include "MitkOpenIGTLinkExports.h"
#include "mitkIGTLMessageFactory.h"
#include "mitkIGTLMessageQueue.h"
#include "mitkIGTLMessage.h"


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
       * \brief Sends a message via the open IGT Link.
       *
       * Convenience function to work with mitk::IGTLMessage directly.
       */
      void SendMessage(const IGTLMessage* msg);

      /**
       * \brief return current object state (Setup, Ready or Running)
       */
      IGTLDeviceState GetState() const;

      /**
       * \brief Returns the oldest command message
       * \param msg A smartpointer to the message base where the oldest command
       * shall be copied into
       * \retval true The latest message is stored in msg
       * \retval false The latest message could not been copied, do not use this
       * data
       */
      igtl::MessageBase::Pointer GetNextCommand();

      /**
       * \brief Returns the oldest received message
       * \param msg A smartpointer to the message base where the latest message
       * shall be copied into
       * \retval true The latest message is stored in msg
       * \retval false The latest message could not been copied, do not use this
       * data
       */
      igtl::MessageBase::Pointer GetNextMessage();

//      /**
//       * \brief Returns information about the oldest message in the receive queue
//       */
//      std::string GetNextMessageInformation();

//      /**
//       * \brief Returns device type about the oldest message in the receive queue
//       */
//      std::string GetNextMessageDeviceType();

//      /**
//       * \brief Returns information about the oldest message in the command queue
//       */
//      std::string GetNextCommandInformation();

//      /**
//       * \brief Returns device type about the oldest message in the command queue
//       */
//      std::string GetNextCommandDeviceType();

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
       * \brief Returns a const reference to the receive queue
       */
      itkGetConstMacro(ReceiveQueue, mitk::IGTLMessageQueue::Pointer);

      /**
       * \brief Returns a const reference to the command queue
       */
      itkGetConstMacro(CommandQueue, mitk::IGTLMessageQueue::Pointer);

      /**
       * \brief Returns a const reference to the send queue
       */
      itkGetConstMacro(SendQueue, mitk::IGTLMessageQueue::Pointer);


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

      /**
       * \brief Send RTS message of given type
       */
      bool SendRTSMessage(const char* type);

    protected:
      /**
       * \brief Sends a message via the open IGT Link.
       *
       * This may only be called after the connection to the device has been
       * established with a call to OpenConnection()
       *
       * \retval IGTL_STATUS_OK the message was sent
       * \retval IGTL_STATUS_UNKONWN_ERROR the message was not sent because an
       * unknown error occurred
       */
      unsigned int SendMessagePrivate(igtl::MessageBase::Pointer msg,
                                      igtl::Socket::Pointer socket);


      /**
      * \brief Call this method to receive a message.
      *
      * The message will be saved in the receive queue.
      */
      virtual void Receive() = 0;

      /**
      * \brief Call this method to receive a message from the given client.
      *
      * The message will be saved in the receive queue.
      *
      * \retval IGTL_STATUS_OK a message or a command was received
      * \retval IGTL_STATUS_NOT_PRESENT the socket is not connected anymore
      * \retval IGTL_STATUS_TIME_OUT the socket timed out
      * \retval IGTL_STATUS_CHECKSUM_ERROR the checksum of the received msg was
      * incorrect
      * \retval IGTL_STATUS_UNKNOWN_ERROR an unknown error occurred
      */
      unsigned int ReceivePrivate(igtl::Socket* client);

      /**
      * \brief Call this method to send a message. The message will be read from
      * the queue
      */
      virtual void Send() = 0;

      /**
      * \brief Call this method to check for other devices that want to connect
      * to this one.
      *
      * In case of a client this method is doing nothing. In case of a server it
      * is checking for other devices and if there is one it establishes a
      * connection.
      */
      virtual void Connect() = 0;

      /**
      * \brief Stops the communication with the given socket
      *
      */
      virtual void StopCommunicationWithSocket(igtl::Socket* socket) = 0;

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
      /** A queue that stores just command messages received by this device */
      mitk::IGTLMessageQueue::Pointer m_CommandQueue;
      /** A message factory that provides the New() method for all msg types */
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

    /**Documentation
    * @brief connect to this Event to get noticed when a command was received
    * */
    itkEventMacro( CommandReceivedEvent , itk::AnyEvent );

    /**Documentation
    * @brief connect to this Event to get noticed when another igtl device
    * connects with this device.
    * */
    itkEventMacro( NewClientConnectionEvent , itk::AnyEvent );

} // namespace mitk

#endif /* MITKIGTLDEVICE_H */
