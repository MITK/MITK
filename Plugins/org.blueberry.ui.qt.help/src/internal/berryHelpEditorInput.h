/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHELPEDITORINPUT_H_
#define BERRYHELPEDITORINPUT_H_

#include <berryIEditorInput.h>
#include <berryIPersistableElement.h>

#include <berryPlatformObject.h>

#include <QUrl>

namespace berry
{

class HelpEditorInput : public PlatformObject, public IEditorInput, public IPersistableElement
{
public:
  berryObjectMacro(HelpEditorInput);

  HelpEditorInput(const QUrl& url = QUrl());

  bool Exists() const override;
  QString GetName() const override;
  QString GetToolTipText() const override;

  QIcon GetIcon() const override;

  const IPersistableElement* GetPersistable() const override;
  Object* GetAdapter(const QString &adapterType) const override;

  QString GetFactoryId() const override;
  void SaveState(const SmartPointer<IMemento>& memento) const override;

  bool operator==(const berry::Object*) const override;

  QUrl GetUrl() const;

private:

  QUrl url;
};

}

#endif /*BERRYHELPEDITORINPUT_H_*/
