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


#ifndef MITKIGTLSERVER_H
#define MITKIGTLSERVER_H

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
    mitkClassMacro(IGTLServer, IGTLDevice)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

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
    virtual bool OpenConnection() override;

    /**
     * \brief Closes the connection to the device
     *
     * This may only be called if there is currently a connection to the
     * device, but device is not running (e.g. object is in Ready state)
     */
    virtual bool CloseConnection() override;

    /**
    * \brief Returns the number of client connections of this device
    */
    virtual unsigned int GetNumberOfConnections() override;

  protected:
    /** Constructor */
    IGTLServer();
    /** Destructor */
    virtual ~IGTLServer();

    /**
    * \brief Call this method to check for other devices that want to connect
    * to this one.
    *
    * In case of a client this method is doing nothing. In case of a server it
    * is checking for other devices and if there is one it establishes a
    * connection and adds the socket to m_RegisteredClients.
    */
    virtual void Connect() override;

    /**
    * \brief Call this method to receive a message.
    *
    * The message will be saved in the receive queue.
    */
    virtual void Receive() override;

    /**
    * \brief Call this method to send a message.
    * The message will be read from the queue. So far the message is send to all
    * connected sockets (broadcast).
    */
    virtual void Send() override;

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
    virtual void StopCommunicationWithSocket(igtl::Socket* client) override;

    /**
     * \brief A list with all registered clients
     */
    SocketListType m_RegisteredClients;
  };
} // namespace mitk
#endif /* MITKIGTLSERVER_H */
