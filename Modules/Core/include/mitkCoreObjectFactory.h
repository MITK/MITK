/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCoreObjectFactory_h
#define mitkCoreObjectFactory_h

#include <set>

#include <mitkCoreObjectFactoryBase.h>
#include <MitkCoreExports.h>
namespace mitk
{
  class Event;

  /**
   * \brief The central factory for creating mappers and setting default properties for MITK core data types.
   *
   * Implements the CoreObjectFactoryBase interface. Additional module-specific
   * factories can be registered via RegisterExtraFactory() to extend the set
   * of supported mappers and default properties.
   *
   * \ingroup DataManagement
   * \sa CoreObjectFactoryBase
   */
  class MITKCORE_EXPORT CoreObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(CoreObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(CoreObjectFactory);

    /**
     * \brief Create a mapper for the given data node and mapper slot.
     * \param node   The data node requiring a mapper.
     * \param slotId The mapper slot (2D or 3D).
     * \return A new mapper instance, or nullptr if no mapper is available.
     */
    Mapper::Pointer CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;

    /**
     * \brief Set default properties on the given data node.
     * \param node The data node to configure with default rendering properties.
     */
    void SetDefaultProperties(mitk::DataNode *node) override;

    /**
     * \brief Map a legacy event to an internal event ID.
     * \param event   The event to map.
     * \param eventID The internal event identifier.
     */
    virtual void MapEvent(const mitk::Event *event, const int eventID);

    /**
     * \brief Register an additional module-specific object factory.
     *
     * The extra factory's mappers and default properties become available
     * through this CoreObjectFactory.
     *
     * \param factory The factory to register.
     */
    virtual void RegisterExtraFactory(CoreObjectFactoryBase *factory);

    /**
     * \brief Unregister a previously registered extra factory.
     * \param factory The factory to remove.
     */
    virtual void UnRegisterExtraFactory(CoreObjectFactoryBase *factory);

    /**
     * \brief Return the singleton instance of the CoreObjectFactory.
     * \return The singleton pointer.
     */
    static Pointer GetInstance();

    /** \brief Destructor. */
    ~CoreObjectFactory() override;

  protected:
    /** \brief Constructor. */
    CoreObjectFactory();

    typedef std::set<mitk::CoreObjectFactoryBase::Pointer> ExtraFactoriesContainer;

    ExtraFactoriesContainer m_ExtraFactories;
  };

} // namespace mitk

#endif
