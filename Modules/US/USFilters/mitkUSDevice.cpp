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


mitk::USDevice::USDevice() : itk::Object()
{
 // m_ConnectedProbes = new std::vector<mitk::USProbe::Pointer>();
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

std::vector<mitk::USProbe::Pointer> mitk::USDevice::GetConnectedProbes()
{
  return m_ConnectedProbes;
}