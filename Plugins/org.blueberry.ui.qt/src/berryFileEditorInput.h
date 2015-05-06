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

  QString GetPath() const;

  bool Exists() const;

  QString GetName() const ;

  QString GetToolTipText() const;

  QIcon GetIcon() const;

  const IPersistableElement* GetPersistable() const;

  Object* GetAdapter(const QString &adapterType) const;

  bool operator==(const Object* o) const;

private:

  QFileInfo m_Path;
};

}

#endif /* BERRYFILEEDITORINPUT_H_ */
