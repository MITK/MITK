/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataStorageEditorInputFactory.h"

#include <mitkDataStorageEditorInput.h>
#include <mitkDataStorageReference.h>

#include <berryIMemento.h>

namespace mitk {

static QString TAG_PATH = "path";
static QString TAG_LABEL = "label";
static QString ID_FACTORY = "org.mitk.ui.DataStorageEditorInputFactory";

berry::IAdaptable* DataStorageEditorInputFactory::CreateElement(const berry::IMemento::Pointer& /*memento*/)
{
  // TODO: Implement restoration of DataStorageEditorInput from memento
  //
  // old code:
  // memento->
  //
  // Remark this implementation was never finished until now and it seems also not currently compiling...
  return nullptr;
}

QString DataStorageEditorInputFactory::GetFactoryId()
{
  return ID_FACTORY;
}

void DataStorageEditorInputFactory::SaveState(const berry::IMemento::Pointer& memento, const DataStorageEditorInput* input)
{
  DataStorageReference dsRef = input->GetDataStorageReference();
  if (dsRef.IsValid())
  {
    QString label = QString::fromStdString(dsRef.GetLabel());
    DataStorage::Pointer dataStorage = dsRef.GetStorage();
    if (dataStorage)
    {
      memento->PutString(TAG_LABEL, label);
      auto nodes = dataStorage->GetAll();
      for(auto nodeIter = nodes.begin(); nodeIter != nodes.end(); ++nodeIter)
      {
        // TODO: Implement node serialization
      }
    }
  }
}

}
