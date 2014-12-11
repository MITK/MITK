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
#include <usModuleContext.h>

#include "mitkTubeGraphIO.h"

namespace mitk
{
  /**
  \brief Registers services for tube graph module.
  */
  class TubeGraphModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context)
    {
      //context->RegisterService(* TUBEGRAPH_MIMETYPE);

        m_TubeGraphIO = new TubeGraphIO();
    }

    void Unload(us::ModuleContext*)
    {
      delete m_TubeGraphIO;
    }

  private:
    TubeGraphIO * m_TubeGraphIO;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::TubeGraphModuleActivator)
