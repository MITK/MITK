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

#include <berryPlatform.h>

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

std::string HelpEditorInput::GetName() const
{
  if (url.isEmpty()) return "Untitled";
  return url.toString().toStdString();
}

std::string HelpEditorInput::GetToolTipText() const
{
  return url.toString().toStdString();
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
