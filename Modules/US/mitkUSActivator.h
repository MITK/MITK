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

#ifndef __mitkUSActivator_h
#define __mitkUSActivator_h

#include "mitkUSVideoDevice.h"

// Microservices
#include <usModuleContext.h>
#include <usModuleActivator.h>

namespace mitk
{
/**
  * \brief Module activator for the US module.
  * Loads mitk::USVideoDevice objects from hard disk on module load and write
  * them to hard disk on module unload.
  *
  * Pointers to mitk::USDevice objects are held to make sure that they
  * will not be deleted while the module is loaded. A service event listener is
  * registered, so that pointers to devices which are registered into micro
  * service from a plugin for example can be held here, too.
  */
class USActivator : public us::ModuleActivator {
public:

  USActivator();
  virtual ~USActivator();

  /**
    * \brief The mitk::USVideoDevice obejcts are loaded from hard disk and registered into micro service.
    */
  void Load(us::ModuleContext* context);

  /**
    * \brief Registered mitk::USVideoDevice objects are stored to hard disk an deregistered from micro service.
    */
  void Unload(us::ModuleContext* context);

protected:
  /**
    *\brief Listens to ServiceRegistry changes and updates the list of mitk::USDevice object accordingly.
    */
  void OnServiceEvent(const us::ServiceEvent event);

  us::ModuleContext*                    m_Context;
  std::vector<USDevice::Pointer>        m_Devices;
};
} // namespace mitk

US_EXPORT_MODULE_ACTIVATOR(mitk::USActivator)

#endif // __mitkUSActivator_h
