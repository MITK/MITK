/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKIGTLCLIENT_H
#define MITKIGTLCLIENT_H

#include "mitkIGTLDevice.h"

#include <MitkOpenIGTLinkExports.h>


namespace mitk
{

  /**
  * \brief Superclass for OpenIGTLink clients
  *
  * Implements the IGTLDevice interface for IGTLClients. In certain points it
  * behaves different than the IGTLServer. The client connects directly to a
  * server (it cannot connect to two different servers). Therefore, it has to
  * check only this one connection.
  *
  * \ingroup OpenIGTLink
  */
  class MITKOPENIGTLINK_EXPORT IGTLClient : public IGTLDevice
  {
  public:
    mitkClassMacro(IGTLClient, IGTLDevice);
    mitkNewMacro1Param(Self, bool);
    itkCloneMacro(Self);

    /**
    * \brief Establishes the connection between this client and the IGTL server.
    *
    * @throw mitk::Exception Throws an exception if the client is not in Setup
    * mode or if it cannot connect to the defined host.
    */
    bool OpenConnection() override;

    /**
    * \brief Returns the number of connections of this device
    *
    * A client can connect to one sever only, therefore, the number is either 0
    * or 1
    */
    unsigned int GetNumberOfConnections() override;

  protected:
    /** Constructor */
    IGTLClient(bool ReadFully);
    /** Destructor */
    ~IGTLClient() override;

    /**
    * \brief Call this method to receive a message.
    *
    * The message will be saved in the receive queue. If the connection is lost
    * it will stop the communication.
    */
    void Receive() override;

    /**
    * \brief Call this method to send a message.
    *
    * The message will be read from the queue.
    */
    void Send() override;

    /**
    * \brief Stops the communication with the given socket.
    *
    * The client uses just one socket. Therefore, calling this function causes
    * the stop of the communication.
    *
    */
    void StopCommunicationWithSocket(igtl::Socket*) override;
  };
} // namespace mitk
#endif /* MITKIGTLCLIENT_H */
