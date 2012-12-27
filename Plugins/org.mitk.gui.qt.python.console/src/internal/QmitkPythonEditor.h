/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKPYTHONEDITOR_H_
#define QMITKPYTHONEDITOR_H_

#include <berryQtEditorPart.h>
#include <berryIPartListener.h>

#include <QTextEdit>

#include "mitkQtCommonDll.h"

#include <mitkIDataStorageReference.h>
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>
#include "QmitkPythonScriptEditor.h"

class QmitkPythonEditor : public berry::QtEditorPart, virtual public berry::IPartListener
{

  Q_OBJECT

public:
  berryObjectMacro(QmitkPythonEditor);

  static const std::string EDITOR_ID;

  QmitkPythonEditor();
  QmitkPythonEditor(const QmitkPythonEditor& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  ~QmitkPythonEditor();

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);

  void SetFocus();

  void DoSave() {}
  void DoSaveAs() {}
  bool IsDirty() const { return false; }
  bool IsSaveAsAllowed() const { return false; }

protected:

  void CreateQtPartControl(QWidget* parent);

  Events::Types GetPartEventTypes() const;

private:
    QmitkPythonScriptEditor *m_PythonScriptEditor;
};

#endif
