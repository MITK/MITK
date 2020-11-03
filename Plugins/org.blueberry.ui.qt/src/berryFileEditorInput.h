/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYFILEEDITORINPUT_H_
#define BERRYFILEEDITORINPUT_H_

#include "berryIPathEditorInput.h"

#include <org_blueberry_ui_qt_Export.h>

#include <QFileInfo>

namespace berry {

class BERRY_UI_QT FileEditorInput : public IPathEditorInput
{

public:

  berryObjectMacro(FileEditorInput);

  FileEditorInput(const QString& path);

  QString GetPath() const override;

  bool Exists() const override;

  QString GetName() const override ;

  QString GetToolTipText() const override;

  QIcon GetIcon() const override;

  const IPersistableElement* GetPersistable() const override;

  Object* GetAdapter(const QString &adapterType) const override;

  bool operator==(const Object* o) const override;

private:

  QFileInfo m_Path;
};

}

#endif /* BERRYFILEEDITORINPUT_H_ */
