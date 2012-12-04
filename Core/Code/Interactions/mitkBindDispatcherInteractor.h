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

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkDispatcher.h"

#include <MitkExports.h>

/**
 * This Class is used to connect a DataStorage with the Dispatcher.
 * This is done by registering for DataStorage Events and sending the Events to the registered Dispatcher.
 * When an Interactor is registered with a DataNode the Dispatcher will be notified.
 */

namespace mitk
{

  class MITK_CORE_EXPORT BindDispatcherInteractor: public itk::Object
  {

  public:
    mitkClassMacro(BindDispatcherInteractor, itk::Object);
    itkNewMacro(Self);

    /**
     * Sets the DataStorage to which Events the class subscribes.
     */
    void SetDataStorage(DataStorage::Pointer dataStorage);

    /**
     * Sets Dispatcher which will be notified.
     */
    void SetDispatcher(Dispatcher::Pointer dispatcher);

    /**
     * Return current Dispatcher.
     */
    Dispatcher::Pointer GetDispatcher();

  protected:
    BindDispatcherInteractor();
    virtual ~BindDispatcherInteractor();

  private:

    void RegisterInteractor(const DataNode*  dataNode);
    void UnRegisterInteractor(const DataNode*  dataNode);
    void RegisterDataStorageEvents();
    void UnRegisterDataStorageEvents();

    Dispatcher::Pointer m_Dispatcher;
    DataStorage::Pointer m_DataStorage;
  };

} /* namespace mitk */
#endif /* mitkBindDispatcherInteractor_h */
