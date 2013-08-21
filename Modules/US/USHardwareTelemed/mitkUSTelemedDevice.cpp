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

#include "mitkUSTelemedDevice.h"


mitk::USTelemedDevice::USTelemedDevice(std::string manufacturer, std::string model)
  : mitk::USDevice(manufacturer, model),
    m_Source(mitk::USImageVideoSource::New()),
    m_IsActive(false)
{
  SetNumberOfOutputs(1);
  SetNthOutput(0, this->MakeOutput(0));

}

virtual std::string mitk::USTelemedDevice::GetDeviceClass()
{
  return "org.mitk.modules.us.USTelemedDevice";
}

virtual bool mitk::USTelemedDevice::OnConnection()
{

}

virtual bool mitk::USTelemedDevice::OnDisconnection()
{

}

virtual bool mitk::USTelemedDevice::OnActivation()
{

}

virtual void mitk::USTelemedDevice::OnDeactivation()
{

}

void mitk::USTelemedDevice::GenerateData()
{

}
