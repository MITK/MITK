/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMAPRegistrationWrapperIO.h"

#include <usModuleActivator.h>
#include <usModuleContext.h>


namespace mitk {

/*
 * This is the module activator for the "MatchPoint" module.
 */
class MatchPointActivator : public us::ModuleActivator
{
public:

  void Load(us::ModuleContext* ) override
  {
    m_MAPRegistrationWrapperIO = new MAPRegistrationWrapperIO();
  }

  void Unload(us::ModuleContext* ) override
  {
    delete m_MAPRegistrationWrapperIO;
  }

private:

  mitk::MAPRegistrationWrapperIO* m_MAPRegistrationWrapperIO;
};
}

US_EXPORT_MODULE_ACTIVATOR(mitk::MatchPointActivator)
