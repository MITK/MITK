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

#ifndef QMITKPYTHONSCRIPTEDITOR_H_
#define QMITKPYTHONSCRIPTEDITOR_H_

#include <berryQtEditorPart.h>
#include <berryIPartListener.h>

#include "QmitkPythonTextEditor.h"
//#include <QTextEdit>

#include "mitkQtCommonDll.h"

#include <mitkIDataStorageReference.h>
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>
#include <stdio.h>

#include <QmitkFunctionality.h>
#include "QmitkPythonMediator.h"

class QmitkPythonScriptEditor : public QWidget, public QmitkPythonPasteClient //, berry::QtEditorPart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  berryObjectMacro(QmitkPythonScriptEditor);

  static const std::string EDITOR_ID;

  QmitkPythonScriptEditor(QWidget *parent = 0);
  virtual void paste(const QString& command);
  ~QmitkPythonScriptEditor();

signals:
  void RanScript();

protected slots:
    /// \brief Called when the user clicks the GUI button
  void LoadScript(const char *);
  void SaveScript();
  void OpenScript();
  void RunScript();

protected:

private:
  QTextEdit* m_TextEditor;
  FILE *m_scriptFile;
  QmitkPythonMediator *m_PythonMediator;
};

#endif
