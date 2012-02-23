/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPythonScriptEditor.h"
//#include <QmitkPythonVariableStack.h>

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
#include <QFileDialog>
#include <QString>
#include <QTextEdit>

#include <mitkGlobalInteraction.h>
#include <mitkCoreObjectFactory.h>
#include <mitkDataStorageEditorInput.h>

#include "berryFileEditorInput.h"
#include <iostream>
#include <fstream>

#include "QmitkPythonConsoleView.h"
#include "QmitkPythonVariableStack.h"
#include "QmitkPythonScriptEditorHighlighter.h"

#include "QmitkPythonMediator.h"

const std::string QmitkPythonScriptEditor::EDITOR_ID = "org.mitk.editors.pythonscripteditor";

QmitkPythonScriptEditor::QmitkPythonScriptEditor(QWidget *parent)
:QWidget(parent)
{
  QGridLayout *gridLayout;
  QPushButton *buttonLoadScript;
  QPushButton *buttonSaveScript;
  QPushButton *buttonRunScript;

  if (parent->objectName().isEmpty())
    parent->setObjectName(QString::fromUtf8("parent"));
  parent->resize(790, 773);
  parent->setMinimumSize(QSize(0, 0));
  gridLayout = new QGridLayout(parent);
  gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
  m_TextEditor = new QTextEdit(parent);
  m_TextEditor->setObjectName(QString::fromUtf8("m_TextEditor"));
  m_TextEditor->viewport()->setAcceptDrops(true);
  
  QmitkPythonScriptEditorHighlighter *highlighter = new QmitkPythonScriptEditorHighlighter(m_TextEditor->document());
  
  gridLayout->addWidget(m_TextEditor, 0, 0, 1, 3);

  buttonLoadScript = new QPushButton(parent);
  buttonLoadScript->setObjectName(QString::fromUtf8("buttonLoadScript"));

  gridLayout->addWidget(buttonLoadScript, 1, 0, 1, 1);

  buttonSaveScript = new QPushButton(parent);
  buttonSaveScript->setObjectName(QString::fromUtf8("buttonSaveScript"));

  gridLayout->addWidget(buttonSaveScript, 1, 1, 1, 1);

  buttonRunScript = new QPushButton(parent);
  buttonRunScript->setObjectName(QString::fromUtf8("buttonRunScript"));

  gridLayout->addWidget(buttonRunScript, 1, 2, 1, 1);
  parent->setWindowTitle(QApplication::translate("parent", "QmitkTemplate", 0, QApplication::UnicodeUTF8));
  buttonLoadScript->setText(QApplication::translate("parent", "Load Script", 0, QApplication::UnicodeUTF8));
  buttonSaveScript->setText(QApplication::translate("parent", "Save Script", 0, QApplication::UnicodeUTF8));
  buttonRunScript->setText(QApplication::translate("parent", "Run Script", 0, QApplication::UnicodeUTF8));

  QMetaObject::connectSlotsByName(parent);

  connect(buttonLoadScript, SIGNAL(clicked()), this, SLOT(OpenScript()));
  connect(buttonSaveScript, SIGNAL(clicked()), this, SLOT(SaveScript()));
  connect(buttonRunScript, SIGNAL(clicked()), this, SLOT(RunScript()));
  QmitkPythonMediator::getInstance()->Initialize();
  QmitkPythonMediator::getInstance()->registerPasteCommandClient( this );
}

QmitkPythonScriptEditor::~QmitkPythonScriptEditor()
{
  QmitkPythonMediator::getInstance()->unregisterPasteCommandClient( this );
  QmitkPythonMediator::getInstance()->Finalize();
}

void QmitkPythonScriptEditor::paste(const QString& command)
{
  if( this->isVisible() )
    m_TextEditor->append(command);
}
void QmitkPythonScriptEditor::LoadScript(const char* filename){
  std::istream* fileStream = new std::ifstream(filename);
  char line[255];
  QString qline;
  m_TextEditor->setText("");
  m_scriptFile = fopen(filename, "r");
  if(fileStream)
  {
    while(!fileStream->eof())
    {
      fileStream->getline(line,255);
      qline = line;
      m_TextEditor->append(qline);
    }
  }
  else
  {
  }
}

void QmitkPythonScriptEditor::SaveScript(){
  QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),"",tr("*.py"));
  if(fileName.compare("") != 0)
  {
    ofstream myfile;
    myfile.open(fileName.toLocal8Bit().data());
    myfile << m_TextEditor->toPlainText().toLocal8Bit().data();
    myfile.close();
  }
}

void QmitkPythonScriptEditor::OpenScript(){
  QString fileName = QFileDialog::getOpenFileName(NULL,mitk::CoreObjectFactory::GetInstance()->GetFileExtensions(),"",tr("*.py"));
  if(fileName.compare("") != 0)
    LoadScript(fileName.toStdString().c_str());
}

void QmitkPythonScriptEditor::RunScript(){
  QmitkPythonMediator::runSimpleString(m_TextEditor->toPlainText().toLocal8Bit().data());
  QmitkPythonMediator::getInstance()->update();
}

