/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHELPEDITORINPUTFACTORY_H
#define BERRYHELPEDITORINPUTFACTORY_H

#include <berryIElementFactory.h>

namespace berry {

class HelpEditorInput;

class HelpEditorInputFactory : public QObject, public IElementFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IElementFactory)

public:

  IAdaptable* CreateElement(const SmartPointer<IMemento>& memento) override;

  static QString GetFactoryId();

  static void SaveState(const SmartPointer<IMemento>& memento, const HelpEditorInput* input);
};

}

#endif // BERRYHELPEDITORINPUTFACTORY_H
