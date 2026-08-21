/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMapperProviderBase_h
#define mitkMapperProviderBase_h

#include <mitkBaseData.h>
#include <mitkIMapperProvider.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

#include <concepts>

namespace mitk
{
  /**
   * \brief Self-registering default implementation of IMapperProvider.
   *
   * Registers itself as an IMapperProvider service on construction and
   * unregisters on destruction. Hold instances in your module activator:
   *
   * \code{.cpp}
   * void Load(us::ModuleContext*) override
   * {
   *   m_MapperProviders.push_back(
   *     std::make_unique<mitk::MapperProviderBase<ExampleMapper2D, ExampleData>>(
   *       mitk::BaseRenderer::Standard2D));
   * }
   *
   * void Unload(us::ModuleContext*) override
   * {
   *   m_MapperProviders.clear();
   * }
   *
   * std::vector<std::unique_ptr<mitk::IMapperProvider>> m_MapperProviders;
   * \endcode
   *
   * \tparam TMapper The mapper class; must provide New() and a static
   *                 SetDefaultProperties(DataNode*, BaseRenderer*, bool).
   * \tparam TData   The most derived data class this mapper targets; its
   *                 GetStaticNameOfClass() becomes PROP_BASEDATA_TYPE().
   *
   * \sa IMapperProvider
   */
  template <typename TMapper, typename TData>
    requires std::derived_from<TMapper, Mapper> && std::derived_from<TData, BaseData>
  class MapperProviderBase : public IMapperProvider
  {
  public:
    /**
     * \brief Register this provider with the calling module's context.
     *
     * \param slotId  The mapper slot served by TMapper, e.g.
     *                BaseRenderer::Standard2D or BaseRenderer::Standard3D.
     * \param ranking Tie-breaker between providers registered for the same
     *                data class and slot; higher wins. MITK's own providers
     *                use the default of 0.
     * \param context The module context to register with. The default
     *                evaluates at the call site, i.e. in the registering
     *                module's translation unit.
     */
    explicit MapperProviderBase(MapperSlotId slotId,
                                int ranking = 0,
                                us::ModuleContext *context = us::GetModuleContext())
    {
      us::ServiceProperties props;
      props[IMapperProvider::PROP_BASEDATA_TYPE()] = std::string(TData::GetStaticNameOfClass());
      props[IMapperProvider::PROP_SLOT_ID()] = static_cast<int>(slotId);
      props[us::ServiceConstants::SERVICE_RANKING()] = ranking;

      m_Registration = context->RegisterService<IMapperProvider>(this, props);
    }

    ~MapperProviderBase() override
    {
      // Check availability, not just validity: stopping the module already
      // unregisters leftover services, and unregistering twice throws.
      if (m_Registration.IsAvailable())
        m_Registration.Unregister();
    }

    MapperProviderBase(const MapperProviderBase &) = delete;
    MapperProviderBase &operator=(const MapperProviderBase &) = delete;

    Mapper::Pointer CreateMapper(DataNode *node) const override
    {
      auto mapper = TMapper::New();
      mapper->SetDataNode(node);
      return mapper.GetPointer();
    }

    void SetDefaultProperties(DataNode *node) const override
    {
      TMapper::SetDefaultProperties(node);
    }

  private:
    us::ServiceRegistration<IMapperProvider> m_Registration;
  };
}

#endif
