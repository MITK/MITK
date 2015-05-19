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

#include "berryHelpEditorInput.h"
#include "berryHelpEditorInputFactory.h"

#include <berryPlatform.h>

#include <QIcon>

namespace berry
{

HelpEditorInput::HelpEditorInput(const QUrl &url)
  : url(url)
{

}

bool HelpEditorInput::Exists() const
{
  return !url.isEmpty();
}

QString HelpEditorInput::GetName() const
{
  if (url.isEmpty()) return "Untitled";
  return url.toString();
}

QString HelpEditorInput::GetToolTipText() const
{
  return url.toString();
}

QIcon HelpEditorInput::GetIcon() const
{
  return QIcon();
}

const IPersistableElement* HelpEditorInput::GetPersistable() const
{
  return this;
}

Object* HelpEditorInput::GetAdapter(const QString& adapterType) const
{
  return PlatformObject::GetAdapter(adapterType);
}

QString HelpEditorInput::GetFactoryId() const
{
  return HelpEditorInputFactory::GetFactoryId();
}

void HelpEditorInput::SaveState(const SmartPointer<IMemento>& memento) const
{
  HelpEditorInputFactory::SaveState(memento, this);
}

bool HelpEditorInput::operator==(const berry::Object* o) const
{
  if (const HelpEditorInput* input = dynamic_cast<const HelpEditorInput*>(o))
    return this->url == input->url;

  return false;
}

QUrl HelpEditorInput::GetUrl() const
{
  return url;
}

}
