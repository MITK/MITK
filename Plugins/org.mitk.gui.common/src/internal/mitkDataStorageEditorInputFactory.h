/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKDATASTORAGEEDITORINPUTFACTORY_H
#define MITKDATASTORAGEEDITORINPUTFACTORY_H

#include <berryIElementFactory.h>

namespace mitk {

class DataStorageEditorInput;

class DataStorageEditorInputFactory : public QObject, public berry::IElementFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IElementFactory)

public:

  berry::IAdaptable* CreateElement(const berry::IMemento::Pointer& memento) override;

  static QString GetFactoryId();

  static void SaveState(const berry::IMemento::Pointer& memento, const DataStorageEditorInput* input);
};

}

#endif // MITKDATASTORAGEEDITORINPUTFACTORY_H
