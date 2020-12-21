/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSUIActivator.h"
#include "QmitkUSControlsCustomVideoDeviceWidget.h"

mitk::USUIActivator::USUIActivator()
{
}

mitk::USUIActivator::~USUIActivator()
{
}

void mitk::USUIActivator::Load(us::ModuleContext* context)
{
  m_USCustomWidgets.push_back(new QmitkUSControlsCustomVideoDeviceWidget());

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
