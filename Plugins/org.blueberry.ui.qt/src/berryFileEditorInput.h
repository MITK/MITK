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
