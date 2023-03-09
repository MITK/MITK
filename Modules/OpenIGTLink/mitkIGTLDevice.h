/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTLDevice_h
#define mitkIGTLDevice_h

#include <mutex>
#include <thread>

#include "mitkCommon.h"

//itk
#include "itkObject.h"

//igtl
#include "igtlSocket.h"
#include "igtlMessageBase.h"
#include "igtlTransformMessage.h"

//mitkIGTL
#include "MitkOpenIGTLinkExports.h"
#include "mitkIGTLMessageFactory.h"
#include "mitkIGTLMessageQueue.h"
#include "mitkIGTLMessage.h"

#if !defined(MITK_WINDOWS_NO_UNDEF) && defined(SendMessage)
  #undef SendMessage
#endif

namespace mitk {
  /**
  * \brief Interface for all OpenIGTLink Devices
  *
  * Defines the methods that are common for all devices using OpenIGTLink. It
  * can open/close a connection, start/stop a communication and send/receive
  * messages.
  *
  * It uses message queues to store the incoming and outgoing mails. They are
  * configurable, you can set buffering on and off.
  *
  * The device is in one of three different states: Setup, Ready or Running.
  * Setup is the initial state. From this state on you can call
  * OpenConnection() and arrive in the Ready state. From the Ready state you
  * call StartCommunication() to arrive in the Running state. Now the device
  * is continuosly checking for new connections, receiving messages and
  * sending messages. This runs in a seperate thread. To stop the communication
  * call StopCommunication() (to arrive in Ready state) or CloseConnection()
  * (to arrive in the Setup state).
  *
  * \ingroup OpenIGTLink
  *
  */
  class MITKOPENIGTLINK_EXPORT IGTLDevice : public itk::Object
  {
  public:
    mitkClassMacroItkParent(IGTLDevice, itk::Object);

      IGTLDevice(bool ReadFully);

    /**
     * \brief Type for state variable.
     * The IGTLDevice is always in one of these states.
     *
     */
    enum IGTLDeviceState { Setup, Ready, Running };

    /**
     * \brief Opens a connection to the device
     *
     * This may only be called if there is currently no connection to the
     * device. If OpenConnection() is successful, the object will change from
     * Setup state to Ready state.
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
     * This may only be called if the device is in Running state.
     */
    virtual bool StopCommunication();

    /**
     * \brief Starts the communication between the two devices
     *
     * This may only be called if the device is in Ready state.
     */
    bool StartCommunication();

    /**
     * \brief Continuously calls the given function
     *
     * This may only be called if the device is in Running state and only from
     * a seperate thread.
     *
     * \param ComFunction function pointer that specifies the method to be executed
     * \param mutex the mutex that corresponds to the function pointer
     */
    void RunCommunication(void (IGTLDevice::*ComFunction)(void), std::mutex& mutex);


    /**
     * \brief Adds the given message to the sending queue
     *
     * This may only be called after the connection to the device has been
     * established with a call to OpenConnection(). Note that the message
     * is not send directly. This method just adds it to the send queue.
     * \param msg The message to be added to the sending queue
     */
    void SendMessage(mitk::IGTLMessage::Pointer msg);

    /**
     * \brief Returns current object state (Setup, Ready or Running)
     */
    IGTLDeviceState GetState() const;

    /**
     * \brief Returns the oldest message in the command queue
     * \return The oldest message from the command queue.
     */
    igtl::MessageBase::Pointer GetNextCommand();

    /**
     * \brief Returns the oldest message in the receive queue
     * \return The oldest message from the receive queue
     */
    igtl::ImageMessage::Pointer GetNextImage2dMessage();

    igtl::ImageMessage::Pointer GetNextImage3dMessage();

    igtl::TransformMessage::Pointer GetNextTransformMessage();

    igtl::TrackingDataMessage::Pointer GetNextTrackingDataMessage();

    igtl::StringMessage::Pointer GetNextStringMessage();

    igtl::MessageBase::Pointer GetNextMiscMessage();

    /**
     * \brief Sets the port number of the device
     */
    itkSetMacro(PortNumber, int);

    /**
     * \brief Returns the port number of the device
     */
    itkGetMacro(PortNumber, int);

    /**
     * \brief Sets the ip/hostname of the device
     */
    itkSetMacro(Hostname, std::string);

    /**
     * \brief Returns the ip/hostname of the device
     */
    itkGetMacro(Hostname, std::string);

    /**
     * \brief Returns the name of this device
     */
    itkGetConstMacro(Name, std::string);

    /**
     * \brief Sets the name of this device
     */
    itkSetMacro(Name, std::string);

    /**
    * \brief Advises this IGTL Device to always block until the whole message is read.
    */
    itkSetMacro(ReadFully, bool);

    /**
     * \brief Returns a const reference to the receive queue
     */
    itkGetConstMacro(MessageQueue, mitk::IGTLMessageQueue::Pointer);

    /**
     * \brief Returns the message factory
     */
    itkGetMacro(MessageFactory, mitk::IGTLMessageFactory::Pointer);

    /**
    * \brief start method for the sending thread.
    */
    void ThreadStartSending();

    /**
    * \brief start method for the receiving thread.
    */
    void ThreadStartReceiving();

    /**
    * \brief start method for the connection thread.
    */
    void ThreadStartConnecting();

    /**
     * \brief TestConnection() tries to connect to a IGTL device on the current
     * ip and port
     *
     * \todo Implement this method. Send a status message and check the answer.
     *
     * TestConnection() tries to connect to a IGTL server on the current
     * ip and port and returns which device it has found.
     * \return It returns the type of the device that answers. Throws an
     * exception
     * if no device is available on that ip/port.
     * @throw mitk::Exception Throws an exception if there are errors
     * while connecting to the device.
     */
    virtual bool TestConnection();

    /**
     * \brief Send RTS message of given type
     */
    bool SendRTSMessage(const char* type);

    /**
    * \brief Sets the buffering mode of the given queue
    */
    void EnableNoBufferingMode(mitk::IGTLMessageQueue::Pointer queue,
      bool enable = true);

    void EnableNoBufferingMode(bool enable = true);

    /**
    * \brief Returns the number of connections of this device
    */
    virtual unsigned int GetNumberOfConnections() = 0;

    itkGetMacro(LogMessages, bool);
    itkSetMacro(LogMessages, bool);

  protected:
    /**
     * \brief Sends a message.
     *
     * This may only be called after the connection to the device has been
     * established with a call to OpenConnection(). This method uses the given
     * socket to send the given MessageReceivedEvent
     *
     * \param msg the message to be sent
     * \param socket the socket used to communicate with the other device
     *
     * \retval IGTL_STATUS_OK the message was sent
     * \retval IGTL_STATUS_UNKONWN_ERROR the message was not sent because an
     * unknown error occurred
     */
    unsigned int SendMessagePrivate(mitk::IGTLMessage::Pointer msg,
      igtl::Socket::Pointer socket);

    /**
    * \brief Call this method to receive a message.
    *
    * The message will be saved in the receive queue.
    */
    virtual void Receive() = 0;

    /**
    * \brief Call this method to receive a message from the given device.
    *
    * The message will be saved in the receive queue.
    *
    * \param device the socket that connects this device with the other one.
    *
    * \retval IGTL_STATUS_OK a message or a command was received
    * \retval IGTL_STATUS_NOT_PRESENT the socket is not connected anymore
    * \retval IGTL_STATUS_TIME_OUT the socket timed out
    * \retval IGTL_STATUS_CHECKSUM_ERROR the checksum of the received msg was
    * incorrect
    * \retval IGTL_STATUS_UNKNOWN_ERROR an unknown error occurred
    */
    unsigned int ReceivePrivate(igtl::Socket* device);

    /**
    * \brief Call this method to send a message. The message will be read from
    * the queue.
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
    virtual void Connect();

    /**
    * \brief Stops the communication with the given socket
    *
    */
    virtual void StopCommunicationWithSocket(igtl::Socket* socket) = 0;

    /**
    * \brief change object state
    */
    void SetState(IGTLDeviceState state);

    IGTLDevice();
    ~IGTLDevice() override;

    /** current object state (Setup, Ready or Running) */
    IGTLDeviceState m_State;
    /** the name of this device */
    std::string m_Name;

    /** signal used to stop the thread*/
    bool m_StopCommunication;
    /** mutex to control access to m_StopCommunication */
    std::mutex m_StopCommunicationMutex;
    /** mutex used to make sure that the send thread is just started once */
    std::mutex m_SendingFinishedMutex;
    /** mutex used to make sure that the receive thread is just started once */
    std::mutex m_ReceivingFinishedMutex;
    /** mutex used to make sure that the connect thread is just started once */
    std::mutex m_ConnectingFinishedMutex;
    /** mutex to control access to m_State */
    mutable std::mutex m_StateMutex;

    /** the hostname or ip of the device */
    std::string m_Hostname;
    /** the port number of the device */
    int m_PortNumber;
    /** the socket used to communicate with other IGTL devices */
    igtl::Socket::Pointer m_Socket;

    /** The message receive queue */
    mitk::IGTLMessageQueue::Pointer m_MessageQueue;

    /** A message factory that provides the New() method for all msg types */
    mitk::IGTLMessageFactory::Pointer m_MessageFactory;

    bool m_LogMessages;

  private:

    /** Sending thread */
    std::thread m_SendThread;
    /** Receiving thread */
    std::thread m_ReceiveThread;
    /** Connecting thread */
    std::thread m_ConnectThread;
    /** Always try to read the full message. */
    bool m_ReadFully;
  };

  /**
  * \brief connect to this Event to get notified when a message was successfully sent
  *
  * \note This event is invoked in the communication thread, therefore do not use it to make
  * changes in the GUI!!! Use the QT signal slot system to decouple this call from the com thread
  * */
  itkEventMacroDeclaration(MessageSentEvent, itk::AnyEvent);

  /**
  * \brief connect to this Event to get notified when a message was received
  *
  * \note Check if you can invoke this events like this or if you have to make
  * it thread-safe. They are not invoked in the main thread!!!
  * */
  itkEventMacroDeclaration(MessageReceivedEvent, itk::AnyEvent);

  /**
  * \brief connect to this Event to get notified when a command was received
  *
  * \note Check if you can invoke this events like this or if you have to make
  * it thread-safe. They are not invoked in the main thread!!!
  * */
  itkEventMacroDeclaration(CommandReceivedEvent, itk::AnyEvent);

  /**
  * \brief connect to this Event to get notified when another igtl device
  * connects with this device.
  *
  * \note Check if you can invoke this events like this or if you have to make
  * it thread-safe. They are not invoked in the main thread!!!
  * */
  itkEventMacroDeclaration(NewClientConnectionEvent, itk::AnyEvent);

  /**
  * \brief connect to this Event to get notified when this device looses the
  * connection to a socket.
  *
  * \note Check if you can invoke this events like this or if you have to make
  * it thread-safe. They are not invoked in the main thread!!!
  * */
  itkEventMacroDeclaration(LostConnectionEvent, itk::AnyEvent);
} // namespace mitk

#endif
