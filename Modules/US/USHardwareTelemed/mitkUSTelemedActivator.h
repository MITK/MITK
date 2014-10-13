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

#ifndef __mitkUSTelemedActivator_h
#define __mitkUSTelemedActivator_h

#include "mitkUSTelemedDevice.h"

// Microservices
#include <usModuleContext.h>
#include <usModuleActivator.h>

namespace mitk
{
  class USTelemedActivator : public us::ModuleActivator {
  public:

    USTelemedActivator();
    virtual ~USTelemedActivator();

    /**
      * \brief Telemed device is created and initialized on module load.
      * Service registration is done during the initialization process.
      */
    void Load(us::ModuleContext* context);

    /**
      * \brief Device pointer is removed on module unload.
      * Service deregistration is done in the device destructor.
      */
    void Unload(us::ModuleContext* context);

  protected:
    USTelemedDevice::Pointer m_Device;
  };
} // namespace mitk

US_EXPORT_MODULE_ACTIVATOR(mitk::USTelemedActivator)

#endif // __mitkUSTelemedActivator_h