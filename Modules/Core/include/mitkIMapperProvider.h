/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIMapperProvider_h
#define mitkIMapperProvider_h

#include <mitkMapper.h>
#include <mitkServiceInterface.h>
#include <MitkCoreExports.h>

#include <string>

namespace mitk
{
  class DataNode;

  /**
   * \ingroup MicroServices_Interfaces
   * \brief Provider service interface for exactly one mapper class in one mapper slot.
   *
   * One IMapperProvider registration corresponds to one pair of mapper class
   * and mapper slot. The targeted data type and slot are expressed as service
   * properties, so provider selection never needs to instantiate mappers
   * speculatively. A mapper serving both the 2D and the 3D slot is expressed
   * as two registrations.
   *
   * Providers are usually registered via MapperProviderBase from the module
   * activator of the module defining the mapper.
   *
   * Provider selection for a data node walks the class hierarchy of the
   * node's data from the most derived class to the base class: a provider
   * registered for a more derived data class always wins over a provider
   * registered for a base class, regardless of service ranking. The service
   * ranking (us::ServiceConstants::SERVICE_RANKING()) only breaks ties
   * between providers registered for the same data class and slot; use a
   * ranking greater than zero to override a default provider registered by
   * MITK for the same data class.
   *
   * \sa MapperProviderBase
   * \sa MapperProviderRegistry
   */
  class MITKCORE_EXPORT IMapperProvider
  {
  public:
    virtual ~IMapperProvider();

    /**
     * \brief Create a new mapper for the given node.
     *
     * The returned mapper is already attached to the node via SetDataNode().
     * A provider may decline a node by returning nullptr; provider selection
     * then falls through to the next candidate or the next, less derived
     * class hierarchy level.
     *
     * \param node The data node requiring a mapper.
     * \return A new mapper instance, or nullptr to decline the node.
     */
    virtual Mapper::Pointer CreateMapper(DataNode *node) const = 0;

    /**
     * \brief Apply the mapper's default properties to the given node.
     *
     * Called when data is assigned to a node, before any renderer is known.
     *
     * \param node The data node to configure with default rendering properties.
     */
    virtual void SetDefaultProperties(DataNode *node) const = 0;

    /**
     * \brief Service property key for the targeted data class.
     *
     * The property value must be of type std::string and correspond to the
     * class name returned by BaseData::GetNameOfClass() for the most derived
     * data class the provider targets.
     */
    static std::string PROP_BASEDATA_TYPE();

    /**
     * \brief Service property key for the targeted mapper slot.
     *
     * The property value must be of type int and correspond to a
     * MapperSlotId, e.g. BaseRenderer::Standard2D or BaseRenderer::Standard3D.
     */
    static std::string PROP_SLOT_ID();
  };
}

MITK_DECLARE_SERVICE_INTERFACE(mitk::IMapperProvider, "org.mitk.IMapperProvider")

#endif
