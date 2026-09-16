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
#include <mitkColorProperty.h>
#include <mitkCoreServices.h>
#include <mitkDataNode.h>
#include <mitkIPropertyTransience.h>
#include <mitkMapperProviderBase.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureMapper2D.h>
#include <mitkPlanarFigureVtkMapper3D.h>
#include <mitkProperties.h>

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Provider for PlanarFigureMapper2D that resets color and opacity.
   *
   * Planar figures are drawn in the color scheme of their own
   * planarfigure.* properties, so the generic node color and opacity are
   * reset even if the node already carries values for them.
   */
  class PlanarFigureMapperProvider : public MapperProviderBase<PlanarFigureMapper2D, PlanarFigure>
  {
  public:
    using MapperProviderBase::MapperProviderBase;

    void SetDefaultProperties(DataNode *node) const override
    {
      MapperProviderBase::SetDefaultProperties(node);

      node->AddProperty("color", ColorProperty::New(1.0, 1.0, 1.0), nullptr, true);
      node->AddProperty("opacity", FloatProperty::New(0.8), nullptr, true);
    }
  };

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
        std::make_unique<PlanarFigureMapperProvider>(BaseRenderer::Standard2D));
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
