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
#include "QmitkUSControlsCustomVideoDeviceWidget.h"
#include "QmitkUSControlsCustomDiPhASDeviceWidget.h"

mitk::USUIActivator::USUIActivator()
{
}

mitk::USUIActivator::~USUIActivator()
{
}

void mitk::USUIActivator::Load(us::ModuleContext* context)
{
  m_USCustomWidgets.push_back(new QmitkUSControlsCustomVideoDeviceWidget());
  m_USCustomWidgets.push_back(new QmitkUSControlsCustomDiPhASDeviceWidget());

  for (auto &widget : m_USCustomWidgets)
  {
    context->RegisterService(widget, widget->GetServiceProperties());
  }
}

void mitk::USUIActivator::Unload(us::ModuleContext* /*context*/)
{
  for (auto &widget : m_USCustomWidgets)
  {
    delete widget;
  }
}
