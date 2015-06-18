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

#ifndef MITKDATASTORAGESERVICE_H_
#define MITKDATASTORAGESERVICE_H_

#include "../mitkIDataStorageService.h"

namespace mitk {

class DataStorageService : public QObject, public IDataStorageService
{
  Q_OBJECT
  Q_INTERFACES(mitk::IDataStorageService)

public:

  DataStorageService();

  bool IsA(const std::type_info& type) const;

  const std::type_info& GetType() const;

  IDataStorageReference::Pointer CreateDataStorage(const QString& label) override;

  std::vector<IDataStorageReference::Pointer> GetDataStorageReferences() const override;

  IDataStorageReference::Pointer GetDefaultDataStorage() const override;
  IDataStorageReference::Pointer GetDataStorage() const override;

  IDataStorageReference::Pointer GetActiveDataStorage() const override;
  void SetActiveDataStorage(IDataStorageReference::Pointer dataStorageRef) override;

  void AddDataStorageReference(IDataStorageReference::Pointer dataStorageRef) override;
  bool RemoveDataStorageReference(IDataStorageReference::Pointer dataStorageRef) override;

private:

  IDataStorageReference::Pointer m_ActiveDataStorageRef;
  IDataStorageReference::Pointer m_DefaultDataStorageRef;
  std::set<IDataStorageReference::Pointer> m_DataStorageReferences;
};

}

#endif /*MITKDATASTORAGESERVICE_H_*/
