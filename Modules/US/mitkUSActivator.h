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

#include "mitkUSVideoDevice.h"

// Microservices
#include <usModuleContext.h>
#include <usModuleActivator.h>

namespace mitk
{
  class USActivator : public us::ModuleActivator {
  public:

    USActivator();
    virtual ~USActivator();

    /**
      * \brief Telemed device is created an initialized on module load.
      * Service registration is done during the initialization process.
      */
    void Load(us::ModuleContext* context);

    /**
      * \brief Device pointer is removed on module unload.
      * Service deregistration is done in the device destructor.
      */
    void Unload(us::ModuleContext* context);

  protected:
    /**
    *\brief This Function listens to ServiceRegistry changes and updates the list of us devices accordingly.
    */
    void OnServiceEvent(const us::ServiceEvent event);

    us::ModuleContext*                    m_Context;
    std::vector<USDevice::Pointer>        m_Devices;
  };
}

US_EXPORT_MODULE_ACTIVATOR(MitkUS, mitk::USActivator)

#endif // __mitkUSTelemedActivator_h
