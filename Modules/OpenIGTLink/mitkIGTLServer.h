/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTLServer_h
#define mitkIGTLServer_h

#include "mitkIGTLDevice.h"

#include <MitkOpenIGTLinkExports.h>

namespace mitk
{
  /**
  * \brief Superclass for OpenIGTLink server
  *
  * Implements the IGTLDevice interface for IGTLServers. In certain points it
  * behaves different than the IGTLClient. The client connects directly to a
  * server (it cannot connect to two different servers) while the server can
  * connect to several clients. Therefore, it is necessary for the server to
  * have a list with registered sockets.
  *
  * \ingroup OpenIGTLink
  */
  class MITKOPENIGTLINK_EXPORT IGTLServer : public IGTLDevice
  {
  public:
    mitkClassMacro(IGTLServer, IGTLDevice);
    mitkNewMacro1Param(Self, bool);
    itkCloneMacro(Self);

    typedef std::list<igtl::Socket::Pointer> SocketListType;
    typedef SocketListType::iterator SocketListIteratorType;

    /**
    * \brief Initialize the connection for the IGTLServer
    *
    *
    * OpenConnection() starts the IGTLServer socket so that clients can connect
    * to it.
    * @throw mitk::Exception Throws an exception if the given port is occupied.
    */
    bool OpenConnection() override;

    /**
     * \brief Closes the connection to the device
     *
     * This may only be called if there is currently a connection to the
     * device, but device is not running (e.g. object is in Ready state)
     */
    bool CloseConnection() override;

    /**
    * \brief Returns the number of client connections of this device
    */
    unsigned int GetNumberOfConnections() override;

  protected:
    /** Constructor */
    IGTLServer(bool ReadFully);
    /** Destructor */
    ~IGTLServer() override;

    /**
    * \brief Call this method to check for other devices that want to connect
    * to this one.
    *
    * In case of a client this method is doing nothing. In case of a server it
    * is checking for other devices and if there is one it establishes a
    * connection and adds the socket to m_RegisteredClients.
    */
    void Connect() override;

    /**
    * \brief Call this method to receive a message.
    *
    * The message will be saved in the receive queue.
    */
    void Receive() override;

    /**
    * \brief Call this method to send a message.
    * The message will be read from the queue. So far the message is send to all
    * connected sockets (broadcast).
    */
    void Send() override;

    /**
      * \brief Stops the communication with the given sockets.
      *
      * This method removes the given sockets from the registered clients list
      *
      */
    virtual void StopCommunicationWithSocket(SocketListType& toBeRemovedSockets);

    /**
      * \brief Stops the communication with the given socket.
      *
      * This method removes the given socket from the registered clients list
      *
      */
    void StopCommunicationWithSocket(igtl::Socket* client) override;

    /**
     * \brief A list with all registered clients
     */
    SocketListType m_RegisteredClients;

    /** mutex to control access to m_RegisteredClients */
    std::mutex m_ReceiveListMutex;

    /** mutex to control access to m_RegisteredClients */
    std::mutex m_SentListMutex;
  };
} // namespace mitk
#endif
