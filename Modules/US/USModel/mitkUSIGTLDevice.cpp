/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY{} without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkUSIGTLDevice.h>

mitk::USIGTLDevice::USIGTLDevice(std::string manufacturer, std::string model,
                                 std::string host, int port, bool server)
  : mitk::USDevice(manufacturer, model), m_Host(host), m_Port(port)
{
  if (server)
  {
    m_Device = mitk::IGTLServer::New(true);
  }
  else
  {
    m_Device = mitk::IGTLClient::New(true);
  }
  m_Device->SetPortNumber(m_Port);
  m_Device->SetHostname(m_Host);
  m_Device->SetName(manufacturer + " - " + model);

  m_DeviceSource = mitk::IGTLDeviceSource::New();
  m_DeviceSource->SetIGTLDevice(m_Device);
  m_DeviceSource->RegisterAsMicroservice();

  m_Filter = mitk::IGTLMessageToUSImageFilter::New();
  m_Filter->SetNumberOfExpectedOutputs(1);
  m_Filter->ConnectTo(m_DeviceSource);
}

std::string mitk::USIGTLDevice::GetDeviceClass() { return "IGTL Client"; }

mitk::USImageSource::Pointer mitk::USIGTLDevice::GetUSImageSource()
{
  return m_Filter.GetPointer();
}

bool mitk::USIGTLDevice::OnInitialization() { return true; }

bool mitk::USIGTLDevice::OnConnection() { return m_Device->OpenConnection(); }

bool mitk::USIGTLDevice::OnDisconnection()
{
  return m_Device->CloseConnection();
}

bool mitk::USIGTLDevice::OnActivation()
{
  return m_Device->StartCommunication();
}

bool mitk::USIGTLDevice::OnDeactivation()
{
  return m_Device->StopCommunication();
}
