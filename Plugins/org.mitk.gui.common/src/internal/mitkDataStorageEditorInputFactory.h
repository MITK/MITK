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
