/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  const HelpEditorInput* input = dynamic_cast<const HelpEditorInput*>(o);

  if (input)
    return this->url == input->url;

  return false;
}

QUrl HelpEditorInput::GetUrl() const
{
  return url;
}

}
