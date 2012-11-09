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

#ifndef MITKDATASTORAGEREFERENCE_H_
#define MITKDATASTORAGEREFERENCE_H_

#include "../mitkIDataStorageReference.h"

namespace mitk
{

class DataStorageReference : public IDataStorageReference
{
public:

  DataStorageReference(DataStorage::Pointer dataStorage, bool isDefault = false);

  DataStorage::Pointer GetDataStorage() const;

  bool IsDefault() const;

  QString GetLabel() const;
  void SetLabel(const QString& label);

  bool operator==(const berry::Object* o) const;

private:

  bool m_Default;
  QString m_Label;
  DataStorage::Pointer m_DataStorage;
};

}

#endif /*MITKDATASTORAGEREFERENCE_H_*/
