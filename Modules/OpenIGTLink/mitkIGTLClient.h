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


#ifndef MITKIGTLCLIENT_H
#define MITKIGTLCLIENT_H

#include "mitkIGTLDevice.h"

#include <MitkOpenIGTLinkExports.h>


namespace mitk
{

  /** Documentation
  * \brief superclass for open IGT link client
  *
  * implements the IGTLDevice interface for IGTL clients
  *
  * \ingroup IGT
  */
  class MITK_OPENIGTLINK_EXPORT IGTLClient : public IGTLDevice
  {
  public:
    mitkClassMacro(IGTLClient, IGTLDevice)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * \brief initialize the connection to the IGTL device
    *
    * \todo check this description
    *
    * OpenConnection() establishes the connection to the IGTL server by:
    * - connection to the IGTL device
    * - initializing the device
    * @throw mitk::IGTHardwareException Throws an exception if there are errors while connecting to the device.
    * @throw mitk::IGTException Throws a normal IGT exception if an error occures which is not related to the hardware.
    */
    virtual bool OpenConnection();


  protected:
    IGTLClient();          ///< Constructor
    virtual ~IGTLClient(); ///< Destructor

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
    * \brief Call this method to check for other devices that want to connect
    * to this one.
    *
    * In case of a client this method is doing nothing. In case of a server it
    * is checking for other devices and if there is one it establishes a
    * connection.
    */
    virtual void Connect();

    /**
      * \brief Stops the communication with the given socket.
      *
      * The client uses just one socket. Therefore, calling this function causes
      * the stop of the communication.
      *
      */
    virtual void StopCommunicationWithSocket(igtl::Socket*);
  };
} // namespace mitk
#endif /* MITKIGTLCLIENT_H */
