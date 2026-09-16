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
#include <mitkMapperProviderBase.h>
#include <mitkROI.h>
#include <mitkROIMapper2D.h>
#include <mitkROIMapper3D.h>

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Module activator for the ROI module.
   *
   * Registers the mapper providers for mitk::ROI.
   */
  class ROIModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *) override
    {
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<ROIMapper2D, ROI>>(BaseRenderer::Standard2D));
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<ROIMapper3D, ROI>>(BaseRenderer::Standard3D));
    }

    void Unload(us::ModuleContext *) override
    {
      m_MapperProviders.clear();
    }

  private:
    std::vector<std::unique_ptr<IMapperProvider>> m_MapperProviders;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::ROIModuleActivator)
