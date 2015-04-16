/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

  IAdaptable* CreateElement(const SmartPointer<IMemento>& memento);

  static QString GetFactoryId();

  static void SaveState(const SmartPointer<IMemento>& memento, const HelpEditorInput* input);
};

}

#endif // BERRYHELPEDITORINPUTFACTORY_H
