/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBindDispatcherInteractor_h
#define mitkBindDispatcherInteractor_h

#include <mitkCommon.h>
#include <MitkCoreExports.h>

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkDispatcher.h>

namespace mitk
{
  /**
   * \class BindDispatcherInteractor
   * \brief Connects a DataStorage with a Dispatcher for interaction event routing.
   *
   * This is done by registering for DataStorage events and forwarding them to the
   * registered Dispatcher. When a DataInteractor is registered with a DataNode,
   * the Dispatcher will be notified. This class also registers the MicroService
   * through which InteractionEventObservers can register to receive events.
   *
   * \ingroup Interaction
   * \sa Dispatcher DataStorage DataInteractor
   */
  class MITKCORE_EXPORT BindDispatcherInteractor
  {
  public:
    /**
     * \brief Construct a BindDispatcherInteractor for the given renderer.
     * \param rendererName Name of the renderer whose Dispatcher is created.
     */
    BindDispatcherInteractor(const std::string &rendererName);

    /** \brief Destructor. Unregisters from the current DataStorage. */
    ~BindDispatcherInteractor();

    /**
     * \brief Set the DataStorage whose events this class subscribes to.
     *
     * When changed, the class unregisters from the previous DataStorage and
     * registers with the new one.
     *
     * \param dataStorage The DataStorage to observe.
     */
    void SetDataStorage(DataStorage::Pointer dataStorage);

    /**
     * \brief Set the Dispatcher to be notified of data node changes.
     *
     * By default each RenderWindow gets its own Dispatcher; this function
     * can be used to override that behavior.
     *
     * \param dispatcher The Dispatcher to use.
     */
    void SetDispatcher(Dispatcher::Pointer dispatcher);

    /**
     * \brief Return the currently active Dispatcher.
     * \return The current Dispatcher instance.
     */
    Dispatcher::Pointer GetDispatcher() const;

  private:
    /**
     * \brief Registers for events from DataStorage.
     *
     * Whenever a DataNode is added, the Dispatcher is notified about this
     * change and checks whether a DataInteractor is set for this DataNode.
     */
    void RegisterInteractor(const DataNode *dataNode);

    /** \brief Removes the given data node's interactor from the Dispatcher. */
    void UnRegisterInteractor(const DataNode *dataNode);

    /** \brief Subscribes to AddNodeEvent, RemoveNodeEvent, and InteractorChangedNodeEvent. */
    void RegisterDataStorageEvents();

    /** \brief Unsubscribes from all DataStorage events. */
    void UnRegisterDataStorageEvents();

    Dispatcher::Pointer m_Dispatcher;
    DataStorage::Pointer m_DataStorage;
  };
}
#endif
