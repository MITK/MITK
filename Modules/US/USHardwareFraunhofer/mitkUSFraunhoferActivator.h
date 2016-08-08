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

#ifndef __mitkUSFraunhoferActivator_h
#define __mitkUSFraunhoferActivator_h

#include "mitkUSFraunhoferDevice.h"

// Microservices
#include <usModuleContext.h>
#include <usModuleActivator.h>

namespace mitk
{
  class USFraunhoferActivator : public us::ModuleActivator {
  public:

    USFraunhoferActivator();
    virtual ~USFraunhoferActivator();

    /**
      * \brief Fraunhofer device is created and initialized on module load.
      * Service registration is done during the initialization process.
      */
    void Load(us::ModuleContext* context);

    /**
      * \brief Device pointer is removed on module unload.
      * Service deregistration is done in the device destructor.
      */
    void Unload(us::ModuleContext* context);

  protected:
    USFraunhoferDevice::Pointer m_Device;
  };
} // namespace mitk

US_EXPORT_MODULE_ACTIVATOR(mitk::USFraunhoferActivator)

#endif // __mitkUSFraunhoferActivator_h