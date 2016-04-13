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

#ifndef MITKIGTLDevice_H_HEADER_INCLUDED_
#define MITKIGTLDevice_H_HEADER_INCLUDED_

#include <mitkCommon.h>
#include <MitkUSExports.h>
#include <mitkUSDevice.h>
#include <mitkIGTLClient.h>
#include <mitkIGTLDeviceSource.h>
#include <mitkIGTL2DImageDeviceSource.h>
#include <mitkIGTLTransformDeviceSource.h>
#include <mitkIGTLMessageToUSImageFilter.h>

namespace mitk
{
  /**
  * \brief A mitk::USIGTLDevice is a USDevice to receive images over an OpenIGTLink
  * connection. It registers an OIGTL device as a Microservice to receive image messages
  * and transforms them to mitk::Images. It can act both as a server (listening for
  * incoming connections) and as a client (connecting to an existing OIGTL server).
  *
  * \ingroup US
  */
  class MITKUS_EXPORT USIGTLDevice : public mitk::USDevice
  {
  public:
    mitkClassMacro(USIGTLDevice, mitk::USDevice);
    // To open a device (Manufacturer, Model, Hostname, Port, IsServer)
    mitkNewMacro5Param(Self, std::string, std::string, std::string, int, bool);

    virtual std::string GetDeviceClass();
    virtual USImageSource::Pointer GetUSImageSource();

    USIGTLDevice(std::string manufacturer, std::string model, std::string host,
      int port, bool server);

  protected:
    virtual bool OnInitialization();
    virtual bool OnConnection();
    virtual bool OnDisconnection();
    virtual bool OnActivation();
    virtual bool OnDeactivation();

  private:
    std::string m_Host;
    int m_Port;
    mitk::IGTLDevice::Pointer m_Device;
    mitk::IGTL2DImageDeviceSource::Pointer m_DeviceSource;
    mitk::IGTLTransformDeviceSource::Pointer m_TransformDeviceSource;
    mitk::IGTLMessageToUSImageFilter::Pointer m_Filter;
  };
}  // namespace mitk

#endif  // MITKIGTLDevice_H_HEADER_INCLUDED_
