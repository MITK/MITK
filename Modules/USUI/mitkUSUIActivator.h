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

#ifndef __mitkUSUIActivator_h
#define __mitkUSUIActivator_h

#include "QmitkUSControlsCustomVideoDeviceWidget.h"

// Microservices
#include <usModuleContext.h>
#include <usModuleActivator.h>

namespace mitk
{
  class USUIActivator : public us::ModuleActivator {
  public:

    USUIActivator();
    virtual ~USUIActivator();

    void Load(us::ModuleContext* context);

    void Unload(us::ModuleContext* context);

  protected:
    QmitkUSControlsCustomVideoDeviceWidget   m_CustomVideoDeviceWidget;
  };
}

US_EXPORT_MODULE_ACTIVATOR(MitkUSUI, mitk::USUIActivator)

#endif // __mitkUSUIActivator_h