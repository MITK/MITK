/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
