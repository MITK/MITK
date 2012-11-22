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

#include <MitkExports.h>

namespace mitk
{

  class MITK_CORE_EXPORT BindDispatcherInteractor: public itk::Object
  {

  public:
    mitkClassMacro(BindDispatcherInteractor, itk::Object)
    ;itkNewMacro(Self)
    ;

    void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
    void SetDispatcher(mitk::Dispatcher::Pointer dispatcher);

  protected:
    BindDispatcherInteractor();
    virtual ~BindDispatcherInteractor();

  private:

    void RegisterInteractor(mitk::DataNode::Pointer  dataNode);
    void UnRegisterInteractor(mitk::DataNode::Pointer  dataNode);
    void RegisterDataStorageEvents();
    void UnRegisterDataStorageEvents();

    mitk::Dispatcher::Pointer m_Dispatcher;
    mitk::DataStorage::Pointer m_DataStorage;
  };

} /* namespace mitk */
#endif /* MITKBINDDISPATCHERINTERACTOR_H_ */
