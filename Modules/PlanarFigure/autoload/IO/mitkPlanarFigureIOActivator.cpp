/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkPlanarFigureIO.h"

namespace mitk
{
  /**
  \brief Registers IO services for PlanarFigure module.
  */
  class PlanarFigureIOModuleActivator : public us::ModuleActivator
  {
    std::vector<AbstractFileIO *> m_FileIOs;

  public:
    void Load(us::ModuleContext * /*context*/) override
    {
      m_FileIOs.push_back(new PlanarFigureIO());
    }
    void Unload(us::ModuleContext *) override
    {
      for (auto &elem : m_FileIOs)
      {
        delete elem;
      }
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PlanarFigureIOModuleActivator)
