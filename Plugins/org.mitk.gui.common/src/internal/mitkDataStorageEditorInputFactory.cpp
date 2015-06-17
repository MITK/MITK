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

#include "mitkDataStorageEditorInputFactory.h"

#include <mitkDataStorageEditorInput.h>

#include <berryIMemento.h>

namespace mitk {

static QString TAG_PATH = "path";
static QString TAG_LABEL = "label";
static QString ID_FACTORY = "org.mitk.ui.DataStorageEditorInputFactory";

berry::IAdaptable* DataStorageEditorInputFactory::CreateElement(const berry::IMemento::Pointer& memento)
{
  memento->
}

QString DataStorageEditorInputFactory::GetFactoryId()
{
  return ID_FACTORY;
}

void DataStorageEditorInputFactory::SaveState(const berry::IMemento::Pointer& memento, const DataStorageEditorInput* input)
{
  IDataStorageReference::Pointer dataStorageRef = input->GetDataStorageReference();
  if (dataStorageRef)
  {
    QString label = dataStorageRef->GetLabel();
    DataStorage::Pointer dataStorage = dataStorageRef->GetDataStorage();
    if (dataStorage)
    {
      memento->PutString(TAG_LABEL, label);
      auto nodes = dataStorage->GetAll();
      for(auto nodeIter = nodes.begin(); nodeIter != nodes.end(); ++nodeIter)
      {
        //(*nodeIter)->GetP
      }
    }
  }
}

}
