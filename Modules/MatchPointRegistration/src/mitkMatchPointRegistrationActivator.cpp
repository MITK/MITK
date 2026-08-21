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
#include <mitkMAPRegistrationWrapper.h>
#include <mitkMapperProviderBase.h>
#include <mitkRegEvaluationMapper2D.h>
#include <mitkRegEvaluationObject.h>
#include <mitkRegistrationWrapperMapper2D.h>
#include <mitkRegistrationWrapperMapper3D.h>

#include <memory>
#include <vector>

namespace mitk
{
  /**
   * \brief Provider for RegEvaluationMapper2D that does not apply any default
   *        properties.
   *
   * The evaluation views apply the mapper's default properties themselves at
   * the appropriate time, so no defaults are set when data is assigned to a
   * node.
   */
  class RegEvaluationMapperProvider : public MapperProviderBase<RegEvaluationMapper2D, RegEvaluationObject>
  {
  public:
    using MapperProviderBase::MapperProviderBase;

    void SetDefaultProperties(DataNode *) const override
    {
    }
  };

  /**
   * \brief Module activator for the MatchPointRegistration module.
   *
   * Registers the mapper providers for mitk::MAPRegistrationWrapper and
   * mitk::RegEvaluationObject.
   */
  class MatchPointRegistrationModuleActivator : public us::ModuleActivator
  {
  public:
    void Load(us::ModuleContext *) override
    {
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<MITKRegistrationWrapperMapper2D, MAPRegistrationWrapper>>(
          BaseRenderer::Standard2D));
      m_MapperProviders.push_back(
        std::make_unique<MapperProviderBase<MITKRegistrationWrapperMapper3D, MAPRegistrationWrapper>>(
          BaseRenderer::Standard3D));
      m_MapperProviders.push_back(
        std::make_unique<RegEvaluationMapperProvider>(BaseRenderer::Standard2D));
    }

    void Unload(us::ModuleContext *) override
    {
      m_MapperProviders.clear();
    }

  private:
    std::vector<std::unique_ptr<IMapperProvider>> m_MapperProviders;
  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::MatchPointRegistrationModuleActivator)
