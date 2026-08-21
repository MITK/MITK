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

#include <mitkBaseRenderer.h>
#include <mitkCoreServices.h>
#include <mitkIPropertyTransience.h>
#include <mitkMapperProviderBase.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureMapper2D.h>
#include <mitkPlanarFigureVtkMapper3D.h>

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Module activator for the PlanarFigure module.
   *
   * Registers the mapper providers for mitk::PlanarFigure and
   * planar-figure-specific transient properties, i.e. runtime/UI
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

      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<PlanarFigureMapper2D, PlanarFigure>>(BaseRenderer::Standard2D));
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<PlanarFigureVtkMapper3D, PlanarFigure>>(BaseRenderer::Standard3D));
    }

    void Unload(us::ModuleContext *) override
    {
      m_MapperProviders.clear();
    }

  private:
    std::vector<std::unique_ptr<IMapperProvider>> m_MapperProviders;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::PlanarFigureModuleActivator)
