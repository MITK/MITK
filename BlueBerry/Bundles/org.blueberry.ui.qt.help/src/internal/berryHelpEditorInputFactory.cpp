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

#include "berryHelpEditorInputFactory.h"

#include <berryIMemento.h>

#include "berryHelpEditorInput.h"

namespace berry {

static QString TAG_PATH = "path";
static QString ID_FACTORY = "org.blueberry.ui.internal.HelpEditorInputFactory";

IAdaptable* HelpEditorInputFactory::CreateElement(const SmartPointer<IMemento>& memento)
{
  QString url;
  if (!memento->GetString(TAG_PATH, url))
  {
    return nullptr;
  }

  QUrl qUrl(url);
  if (qUrl.isValid())
  {
    return new HelpEditorInput(qUrl);
  }
  return nullptr;
}

QString HelpEditorInputFactory::GetFactoryId()
{
  return ID_FACTORY;
}

void HelpEditorInputFactory::SaveState(const SmartPointer<IMemento>& memento, const HelpEditorInput* input)
{
  QUrl url = input->GetUrl();
  memento->PutString(TAG_PATH, url.toString());
}

}
