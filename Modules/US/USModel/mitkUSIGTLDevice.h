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
#include <mitkIGTLMessageToUSImageFilter.h>

namespace mitk
{
class MITKUS_EXPORT USIGTLDevice : public mitk::USDevice
{
 public:
  mitkClassMacro(USIGTLDevice, mitk::USDevice);
  // To open a device (Manufacturer, Model, Hostname, Port)
  mitkNewMacro4Param(Self, std::string, std::string, std::string, int);

  virtual std::string GetDeviceClass();
  virtual USImageSource::Pointer GetUSImageSource();

  USIGTLDevice(std::string manufacturer, std::string model, std::string host,
               int port);
  USIGTLDevice(std::string manufacturer, std::string model);

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
  mitk::IGTLDeviceSource::Pointer m_DeviceSource;
  mitk::IGTLMessageToUSImageFilter::Pointer m_Filter;
};

}  // namespace mitk

#endif  // MITKIGTLDevice_H_HEADER_INCLUDED_
