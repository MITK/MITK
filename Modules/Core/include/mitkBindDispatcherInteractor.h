/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#ifndef mitkBindDispatcherInteractor_h
#define mitkBindDispatcherInteractor_h

#include "mitkCommon.h"
#include <MitkCoreExports.h>

#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkDispatcher.h"

namespace mitk
{
  /**
   * \class BindDispatcherInteractor
   * \brief This Class is used to connect a DataStorage with the Dispatcher.
   *
   * This is done by registering for DataStorage Events and sending the Events to the registered Dispatcher.
   * When a DataInteractor is registered with a DataNode the Dispatcher will be notified.
   * Also this class registers the MicroService at which InteractionEventObservers can register to receive events.
   *
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT BindDispatcherInteractor
  {
  public:
    BindDispatcherInteractor(const std::string &);
    ~BindDispatcherInteractor();

    /**
     * Sets the DataStorage to which Events the class subscribes.
     */
    void SetDataStorage(DataStorage::Pointer dataStorage);

    /**
     * Sets Dispatcher which will be notified. By default each RenderWindow gets its own Dispatcher,
     * this function can be used to override this behavior.
     */
    void SetDispatcher(Dispatcher::Pointer dispatcher);

    /**
     * Return currently active Dispatcher.
     */
    Dispatcher::Pointer GetDispatcher() const;

  private:
    /** @brief Registers for events from DataStorage.
     *
     * This way whenever a DataNode is added the Dispatcher is notified about this change, and checks whether a
     * DataInteractor
     * is set for this DataNode
     */
    void RegisterInteractor(const DataNode *dataNode);
    void UnRegisterInteractor(const DataNode *dataNode);
    void RegisterDataStorageEvents();
    void UnRegisterDataStorageEvents();

    Dispatcher::Pointer m_Dispatcher;
    DataStorage::Pointer m_DataStorage;
  };
}
#endif /* mitkBindDispatcherInteractor_h */
