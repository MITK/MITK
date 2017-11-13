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
