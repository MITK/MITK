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
#include <mitkIPropertyDeserialization.h>
#include <mitkMapperProviderBase.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureVisibilityProperty.h>
#include <mitkPlanarFigureVtkMapper2D.h>
#include <mitkPlanarFigureVtkMapper3D.h>

#include "mitkPlanarFigureLegacyProperties.h"

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Module activator for the PlanarFigure module.
   *
   * Registers the mapper providers for mitk::PlanarFigure, its property types
   * and the migration of properties of previous versions.
   */
  class PlanarFigureModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *) override
    {
      PlanarFigureLegacyProperties::Register();

      CoreServicePointer<IPropertyDeserialization> deserialization(CoreServices::GetPropertyDeserialization());
      deserialization->RegisterProperty<PlanarFigureVisibilityProperty>();

      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<PlanarFigureVtkMapper2D, PlanarFigure>>(BaseRenderer::Standard2D));
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
