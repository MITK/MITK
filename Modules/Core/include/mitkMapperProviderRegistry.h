/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMapperProviderRegistry_h
#define mitkMapperProviderRegistry_h

#include <mitkMapper.h>
#include <MitkCoreExports.h>

#include <memory>

namespace us
{
  class ModuleContext;
}

namespace mitk
{
  class DataNode;

  /**
   * \brief Lookup facade over the registered IMapperProvider services.
   *
   * Tracks all IMapperProvider registrations in an internal index so that
   * mapper lookup during rendering never has to query the service registry.
   *
   * Provider selection walks the class hierarchy of the node's data from
   * the most derived class to the base class. At the first hierarchy level
   * with matching providers, candidates are tried in the order of service
   * ranking (descending) and registration (ascending); the first provider
   * returning a non-null mapper wins. If all candidates of a level decline,
   * selection falls through to the next, less derived level.
   *
   * The registry is started and stopped by the MitkCore module activator.
   *
   * \sa IMapperProvider
   * \sa MapperProviderBase
   */
  class MITKCORE_EXPORT MapperProviderRegistry
  {
  public:
    static MapperProviderRegistry &GetInstance();

    MapperProviderRegistry(const MapperProviderRegistry &) = delete;
    MapperProviderRegistry &operator=(const MapperProviderRegistry &) = delete;

    /**
     * \brief Create the best matching mapper for the node's data and slot.
     * \param node   The data node requiring a mapper.
     * \param slotId The mapper slot (2D or 3D).
     * \return A new mapper instance, or nullptr if no provider matches.
     */
    Mapper::Pointer CreateMapper(DataNode *node, MapperSlotId slotId) const;

    /**
     * \brief Apply default properties of all providers matching the node's
     *        data type, across all mapper slots.
     *
     * Providers are applied most derived data class first, so with the usual
     * AddProperty(..., overwrite = false) semantics of the mappers' static
     * SetDefaultProperties() the most specific provider determines
     * conflicting property values.
     *
     * \param node The data node to configure with default rendering properties.
     */
    void ApplyDefaultProperties(DataNode *node) const;

    /**
     * \brief Start tracking IMapperProvider services.
     *
     * Called by the MitkCore module activator on load. Must be called before
     * any providers can be found.
     *
     * \param context The module context used for tracking.
     */
    void Start(us::ModuleContext *context);

    /**
     * \brief Stop tracking IMapperProvider services.
     *
     * Called by the MitkCore module activator on unload.
     */
    void Stop();

  private:
    MapperProviderRegistry();
    ~MapperProviderRegistry();

    class Impl;
    std::unique_ptr<Impl> m_Impl;
  };
}

#endif
