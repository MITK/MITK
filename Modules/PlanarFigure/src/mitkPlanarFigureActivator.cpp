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

#include <mitkCoreServices.h>
#include <mitkIPropertyTransience.h>
#include <mitkPlanarFigure.h>

namespace mitk
{
  /**
   * \brief Module activator for the PlanarFigure module.
   *
   * Registers planar-figure-specific transient properties, i.e. runtime/UI
   * state that must not be persisted to scene files. The generic node
   * "selected" flag is registered centrally by the Core module.
   */
  class PlanarFigureModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *) override
    {
      CoreServicePointer<IPropertyTransience> transience(CoreServices::GetPropertyTransience());
      transience->AddTransient<PlanarFigure>("planarfigure.ishovering");
    }

    void Unload(us::ModuleContext *) override {}
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PlanarFigureModuleActivator)
