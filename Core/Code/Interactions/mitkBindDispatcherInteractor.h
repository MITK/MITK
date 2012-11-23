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

#ifndef MITKBINDDISPATCHERINTERACTOR_H_
#define MITKBINDDISPATCHERINTERACTOR_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkDispatcher.h"

#include <MitkExports.h>

namespace mitk
{

  class MITK_CORE_EXPORT BindDispatcherInteractor: public itk::Object
  {

  public:
    mitkClassMacro(BindDispatcherInteractor, itk::Object);
    itkNewMacro(Self);

    void SetDataStorage(DataStorage::Pointer dataStorage);

    void SetDispatcher(Dispatcher::Pointer dispatcher);
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
#endif /* MITKBINDDISPATCHERINTERACTOR_H_ */
