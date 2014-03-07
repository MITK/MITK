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

#include "mitkUSControlInterfaceProbes.h"
#include "mitkUSDevice.h"

mitk::USControlInterfaceProbes::USControlInterfaceProbes(itk::SmartPointer<USDevice> device)
  : mitk::USAbstractControlInterface(device)
{
}

mitk::USControlInterfaceProbes::~USControlInterfaceProbes()
{
}

void mitk::USControlInterfaceProbes::SelectProbe( unsigned int index )
{
  this->OnSelectProbe(index);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_PROBES_SELECTED,
                                  this->GetSelectedProbe()->GetName());

}

void mitk::USControlInterfaceProbes::SelectProbe( USProbe::Pointer probe )
{
  this->OnSelectProbe(probe);
  m_Device->UpdateServiceProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_PROBES_SELECTED,
                                  this->GetSelectedProbe()->GetName());
}
