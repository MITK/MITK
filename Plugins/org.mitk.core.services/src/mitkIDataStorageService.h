/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIDATASTORAGESERVICE_H_
#define MITKIDATASTORAGESERVICE_H_

#include <org_mitk_core_services_Export.h>
#include "mitkIDataStorageReference.h"

#include <string>

#include <QtPlugin>

namespace mitk
{

/**
 * \ingroup org_mitk_core_services
 */
struct MITK_CORE_SERVICES_PLUGIN IDataStorageService
{
  virtual ~IDataStorageService();

  virtual IDataStorageReference::Pointer CreateDataStorage(const QString& label) = 0;
  virtual std::vector<IDataStorageReference::Pointer> GetDataStorageReferences() const = 0;

  virtual IDataStorageReference::Pointer GetDefaultDataStorage() const = 0;

  virtual IDataStorageReference::Pointer GetDataStorage() const = 0;

  virtual IDataStorageReference::Pointer GetActiveDataStorage() const = 0;
  virtual void SetActiveDataStorage(IDataStorageReference::Pointer dataStorageRef) = 0;

  virtual void AddDataStorageReference(IDataStorageReference::Pointer dataStorageRef) = 0;
  virtual bool RemoveDataStorageReference(IDataStorageReference::Pointer dataStorageRef) = 0;
};

}

Q_DECLARE_INTERFACE(mitk::IDataStorageService, "org.mitk.service.IDataStorageService")

#endif /*MITKIDATASTORAGESERVICE_H_*/
