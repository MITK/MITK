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

#include "mitkUSUIActivator.h"

mitk::USUIActivator::USUIActivator()
{

}

mitk::USUIActivator::~USUIActivator()
{

}

void mitk::USUIActivator::Load(us::ModuleContext* context)
{
  m_ServiceRegistration = m_CustomVideoDeviceWidget.RegisterService(context);
}

void mitk::USUIActivator::Unload(us::ModuleContext* /*context*/)
{
  m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}
