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

mitk::USIGTLDevice::USIGTLDevice(std::string manufacturer, std::string model)
  : USIGTLDevice(manufacturer, model, "localhost", 18944)
{
}

mitk::USIGTLDevice::USIGTLDevice(std::string manufacturer, std::string model,
                                 std::string host, int port)
  : mitk::USDevice(manufacturer, model), m_Host(host), m_Port(port)
{
  m_Client = mitk::IGTLClient::New();
  // TODO: Set Name
  m_Client->SetName("OIGTL Test device");
  m_Client->SetPortNumber(m_Port);
  m_Client->SetHostname(m_Host);

  m_DeviceSource = mitk::IGTLDeviceSource::New();
  m_DeviceSource->SetIGTLDevice(m_Client);
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

bool mitk::USIGTLDevice::OnConnection() { return m_Client->OpenConnection(); }

bool mitk::USIGTLDevice::OnDisconnection()
{
  return m_Client->CloseConnection();
}

bool mitk::USIGTLDevice::OnActivation()
{
  return m_Client->StartCommunication();
}

bool mitk::USIGTLDevice::OnDeactivation()
{
  return m_Client->StopCommunication();
}
