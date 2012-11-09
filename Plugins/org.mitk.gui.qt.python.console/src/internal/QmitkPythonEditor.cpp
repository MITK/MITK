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

#include "QmitkPythonEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPartListener.h>

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

#include <mitkGlobalInteraction.h>

#include <mitkDataStorageEditorInput.h>

#include "berryFileEditorInput.h"

const std::string QmitkPythonEditor::EDITOR_ID = "org.mitk.editors.pythoneditor";

QmitkPythonEditor::QmitkPythonEditor()
{

}

QmitkPythonEditor::~QmitkPythonEditor()
{
}

void QmitkPythonEditor::CreateQtPartControl(QWidget* parent)
{
  m_PythonScriptEditor = new QmitkPythonScriptEditor(parent);

  if (parent->objectName().isEmpty())
    parent->setObjectName(QString::fromUtf8("parent"));
  parent->resize(790, 773);
  parent->setMinimumSize(QSize(0, 0));
  QGridLayout *gridLayout = new QGridLayout(parent);
  gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
  gridLayout->addWidget(m_PythonScriptEditor, 0, 0, 1, 3);

  QMetaObject::connectSlotsByName(parent);
}

void QmitkPythonEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  if (input.Cast<berry::FileEditorInput>().IsNull())
     throw berry::PartInitException("Invalid Input: Must be FileEditorInput");

  this->SetSite(site);
  this->SetInput(input);
}

void QmitkPythonEditor::SetFocus()
{
}

berry::IPartListener::Events::Types QmitkPythonEditor::GetPartEventTypes() const
{
  return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

