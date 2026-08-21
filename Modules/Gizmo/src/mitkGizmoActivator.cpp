/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkBaseRenderer.h>
#include <mitkGizmo.h>
#include <mitkMapperProviderBase.h>

#include "mitkGizmoMapper2D.h"

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Module activator for the Gizmo module.
   *
   * Registers the mapper provider for mitk::Gizmo. In the 3D slot, gizmos
   * are rendered by the Surface mapper registered by the Core module.
   */
  class GizmoModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *) override
    {
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<GizmoMapper2D, Gizmo>>(BaseRenderer::Standard2D));
    }

    void Unload(us::ModuleContext *) override
    {
      m_MapperProviders.clear();
    }

  private:
    std::vector<std::unique_ptr<IMapperProvider>> m_MapperProviders;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::GizmoModuleActivator)
