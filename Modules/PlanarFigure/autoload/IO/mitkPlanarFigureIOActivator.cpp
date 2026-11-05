/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "mitkLegacyPlanarFigureReader.h"
#include "mitkPlanarFigureIO.h"

#include <memory>

namespace mitk
{
  /**
  \brief Registers IO services for PlanarFigure module.
  */
  class PlanarFigureIOModuleActivator : public us::ModuleActivator
  {
    std::unique_ptr<PlanarFigureIO> m_FileIO;
    std::unique_ptr<LegacyPlanarFigureReader> m_LegacyReader;

  public:
    void Load(us::ModuleContext *) override
    {
      m_FileIO = std::make_unique<PlanarFigureIO>();
      m_LegacyReader = std::make_unique<LegacyPlanarFigureReader>();
    }

    void Unload(us::ModuleContext *) override
    {
      m_LegacyReader.reset();
      m_FileIO.reset();
    }
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PlanarFigureIOModuleActivator)
