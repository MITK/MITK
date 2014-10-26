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
#include <usModuleActivator.h>

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class FiberTrackingModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* /*context*/)
    {

    }

    void Unload(us::ModuleContext*)
    {

    }

  private:


  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::FiberTrackingModuleActivator)
