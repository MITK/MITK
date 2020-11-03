/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
