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

  /** Documentation
  * \brief superclass for open IGT link server
  *
  * implements the IGTLDevice interface for IGTL servers
  *
  * \ingroup IGT
  */
  class MITK_OPENIGTLINK_EXPORT IGTLServer : public IGTLDevice
  {
  public:
    mitkClassMacro(IGTLServer, IGTLDevice)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * \brief initialize the connection for the IGTL device
    *
    * \todo check this description
    *
    * OpenConnection() starts the IGTL server so that clients can connect to it
    * @throw mitk::IGTHardwareException Throws an exception if there are errors
    * while connecting to the device.
    * @throw mitk::IGTException Throws a normal IGT exception if an error occures
    *  which is not related to the hardware.
    */
    virtual bool OpenConnection();


  protected:
    IGTLServer();          ///< Constructor
    virtual ~IGTLServer(); ///< Destructor

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
    * \brief Call this method to receive a message.
    *
    * The message will be saved in the receive queue.
    */
    virtual void Receive();

    /**
    * \brief Call this method to send a message. The message will be read from
    * the queue
    */
    virtual void Send();

    /**
      * \brief Stops the communication with the given socket.
      *
      * This method removes the given socket from the registered clients list
      *
      */
    virtual void StopCommunicationWithSocket(igtl::Socket* client);

    /**
     * @brief A list with all registered clients
     */
    std::list<igtl::Socket::Pointer> m_RegisteredClients;
  };
} // namespace mitk
#endif /* MITKIGTLSERVER_H */
