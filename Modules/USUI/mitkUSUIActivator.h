/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUSUIActivator_h
#define mitkUSUIActivator_h

#include "QmitkUSAbstractCustomWidget.h"

// Microservices
#include <usModuleContext.h>
#include <usModuleActivator.h>

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
    ~USUIActivator() override;

    /**
      * Custom video device widget is registered as a micro service on module
      * load. A plugin can get this widget then when using a
      * mitk::USVideoDevice.
      */
    void Load(us::ModuleContext* context) override;

    /**
      * Custom video device widget is deregistered from micro service on module
      * unload.
      */
    void Unload(us::ModuleContext* context) override;

  protected:

    std::vector<QmitkUSAbstractCustomWidget*> m_USCustomWidgets; ///< Contains the currently available US custom controls widgets which are loaded on Load()
  };
} // namespace mitk

US_EXPORT_MODULE_ACTIVATOR(mitk::USUIActivator)

#endif
