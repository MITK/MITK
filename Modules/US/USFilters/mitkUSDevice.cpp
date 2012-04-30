/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUSDevice.h"
#include "mitkUSImageMetadata.h"


mitk::USDevice::USDevice(std::string manufacturer, std::string model, bool isVideoOnly) : itk::Object()
{
  m_Metadata = mitk::USImageMetadata::New();
  m_Metadata->SetDeviceManufacturer(manufacturer);
  m_Metadata->SetDeviceModel(model);
  m_Metadata->SetDeviceIsVideoOnly(isVideoOnly);
}

mitk::USDevice::~USDevice()
{

}

void mitk::USDevice::AddProbe(mitk::USProbe::Pointer probe)
{
  for(int i = 0; i < m_ConnectedProbes.size(); i++)
  {
    if (m_ConnectedProbes[i]->IsEqualToProbe(probe)) return;
  }
  this->m_ConnectedProbes.push_back(probe);
}

void mitk::USDevice::ActivateProbe(mitk::USProbe::Pointer probe){
  // currently, we may just add the probe. This behaviour must be changed, should more complicated SDK applications emerge 
  AddProbe(probe);
  int index = -1;
  for(int i = 0; i < m_ConnectedProbes.size(); i++)
  {
    if (m_ConnectedProbes[i]->IsEqualToProbe(probe)) index = i;
  }
  // index now contains the position of the original instance of this probe
  m_ActiveProbe = m_ConnectedProbes[index];
}

void mitk::USDevice::DeactivateProbe(){
  m_ActiveProbe = 0;
}

 //########### GETTER & SETTER ##################//

std::string mitk::USDevice::GetDeviceManufacturer(){
  return this->m_Metadata->GetDeviceManufacturer();
}

std::string mitk::USDevice::GetDeviceModel(){
  return this->m_Metadata->GetDeviceModel();
}

std::string mitk::USDevice::GetDeviceComment(){
  return this->m_Metadata->GetDeviceComment();
}

bool mitk::USDevice::GetIsVideoOnly(){
  return this->m_Metadata->GetDeviceIsVideoOnly();
}

std::vector<mitk::USProbe::Pointer> mitk::USDevice::GetConnectedProbes()
{
  return m_ConnectedProbes;
}
