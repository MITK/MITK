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

#include "QmitkUSAbstractCustomWidget.h"

// Microservices
#include <usModuleContext.h>
#include <usModuleActivator.h>

class QmitkUSControlsCustomVideoDeviceWidget;

namespace mitk
{
  class USUICustomWidgetFactory;

  /**
    * \brief Module activator for the USUI module.
    * Registers custom widget for mitk::USVideoDevice as microservice.
    */
  class USUIActivator : public us::ModuleActivator {
  public:

    USUIActivator();
    virtual ~USUIActivator();

    /**
      * Custom video device widget is registered as a micro service on module
      * load. A plugin can get this widget then when using a
      * mitk::USVideoDevice.
      */
    void Load(us::ModuleContext* context);

    /**
      * Custom video device widget is deregistered from micro service on module
      * unload.
      */
    void Unload(us::ModuleContext* context);

  protected:
    us::ServiceRegistration<QmitkUSAbstractCustomWidget>  m_ServiceRegistration;

    USUICustomWidgetFactory*                              m_CustomWidgetFactory;
    QmitkUSControlsCustomVideoDeviceWidget*               m_CustomVideoDeviceWidget;
  };
} // namespace mitk

US_EXPORT_MODULE_ACTIVATOR(mitk::USUIActivator)

#endif // __mitkUSUIActivator_h
