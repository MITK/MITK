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

#ifndef BERRYHELPEDITORINPUT_H_
#define BERRYHELPEDITORINPUT_H_

#include <berryIEditorInput.h>

#include <QUrl>

namespace berry
{

class HelpEditorInput : public berry::IEditorInput
{
public:
  berryObjectMacro(HelpEditorInput)

  HelpEditorInput(const QUrl& url = QUrl());

  bool Exists() const;
  std::string GetName() const;
  std::string GetToolTipText() const;

  bool operator==(const berry::Object*) const;

  QUrl GetUrl() const;

private:

  QUrl url;
};

}

#endif /*BERRYHELPEDITORINPUT_H_*/
