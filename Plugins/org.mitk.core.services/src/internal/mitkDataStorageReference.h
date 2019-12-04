/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKDATASTORAGEREFERENCE_H_
#define MITKDATASTORAGEREFERENCE_H_

#include "../mitkIDataStorageReference.h"

namespace mitk
{

class DataStorageReference : public IDataStorageReference
{
public:

  DataStorageReference(DataStorage::Pointer dataStorage, bool isDefault = false);

  DataStorage::Pointer GetDataStorage() const override;

  bool IsDefault() const override;

  QString GetLabel() const override;
  void SetLabel(const QString& label) override;

  bool operator==(const berry::Object* o) const override;

private:

  bool m_Default;
  QString m_Label;
  DataStorage::Pointer m_DataStorage;
};

}

#endif /*MITKDATASTORAGEREFERENCE_H_*/
