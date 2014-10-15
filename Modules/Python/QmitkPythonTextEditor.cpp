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

#include "QmitkPythonTextEditor.h"

#include <QList>
#include <QFileInfo>
#include <QUrl>
#include <QtGui>
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <mitkDataNode.h>
#include <usGetModuleContext.h>
#include <mitkIPythonService.h>
#include "QmitkPythonScriptEditorHighlighter.h"

struct QmitkPythonTextEditorData
{
  QString m_FilePath;

  QAction* m_LoadScript;
  QAction* m_SaveScript;
  QAction* m_RunScript;
  QToolBar* m_Toolbar;

  QTextEdit* m_Content;

  QGridLayout* m_Layout;
  mitk::IPythonService* m_PythonService;
  us::ServiceReference<mitk::IPythonService> m_PythonServiceRef;

  QString m_FileName;
};

QmitkPythonTextEditor::QmitkPythonTextEditor(QWidget *parent)
  :QWidget(parent), d(new QmitkPythonTextEditorData)
{
  us::ModuleContext* context = us::GetModuleContext();
  d->m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
  d->m_PythonService = context->GetService<mitk::IPythonService>( d->m_PythonServiceRef );

  d->m_LoadScript = new QAction(this);
  d->m_LoadScript->setToolTip("Load script from disk.");
  d->m_LoadScript->setObjectName(QString::fromUtf8("LoadScript"));
  QIcon icon2;
  icon2.addFile(QString::fromUtf8(":/mitkPython/document-open.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_LoadScript->setIcon(icon2);

  d->m_SaveScript = new QAction(this);
  d->m_SaveScript->setToolTip("Save script to disk.");
  d->m_SaveScript->setObjectName(QString::fromUtf8("SaveScript"));
  QIcon icon3;
  icon3.addFile(QString::fromUtf8(":/mitkPython/document-save.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_SaveScript->setIcon(icon3);

  d->m_RunScript = new QAction(this);
  d->m_RunScript->setToolTip("Run the current script.");
  d->m_RunScript->setObjectName(QString::fromUtf8("RunScript"));
  QIcon icon4;
  icon4.addFile(QString::fromUtf8(":/mitkPython/media-playback-start.png"), QSize(), QIcon::Normal, QIcon::Off);
  d->m_RunScript->setIcon(icon4);

  d->m_Toolbar = new QToolBar;
  d->m_Toolbar->addAction( d->m_LoadScript );
  d->m_Toolbar->addAction( d->m_SaveScript );
  d->m_Toolbar->addAction( d->m_RunScript );

  d->m_Content = new QTextEdit(this);
  d->m_Content->setObjectName(QString::fromUtf8("Content"));

  QmitkPythonScriptEditorHighlighter* highlighter =
      new QmitkPythonScriptEditorHighlighter( d->m_Content->document() );

  d->m_Layout = new QGridLayout;
  d->m_Layout->addWidget( d->m_Toolbar, 0, 0, 1, 1 );
  d->m_Layout->addWidget( d->m_Content, 1, 0, 1, 1 );
  d->m_Layout->setContentsMargins(2,2,2,2);

  this->setLayout(d->m_Layout);
  QMetaObject::connectSlotsByName(this);
}

QmitkPythonTextEditor::~QmitkPythonTextEditor()
{
  us::ModuleContext* context = us::GetModuleContext();
  context->UngetService( d->m_PythonServiceRef );

  delete d;
}

void QmitkPythonTextEditor::dragEnterEvent(QDragEnterEvent *event)
{
  event->accept();
}

void QmitkPythonTextEditor::dropEvent(QDropEvent *event)
{
  QList<QUrl> urls = event->mimeData()->urls();
  for(int i = 0; i < urls.size(); i++)
  {
    this->Paste( urls[i].toString() );
  }
}
/*
bool QmitkPythonTextEditor::canInsertFromMimeData( const QMimeData * ) const
{
  return true;
}
*/

void QmitkPythonTextEditor::Paste(const QString &command)
{
  if( this->isVisible() )
  {
    d->m_Content->insertPlainText(command + "\n");
  }
}


QString QmitkPythonTextEditor::ReadFile(const QString& filename)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    MITK_ERROR << "Could not open file " << filename.toStdString();
    return NULL;
  }

  QByteArray total;
  QByteArray line;
  while (!file.atEnd())
  {
    line = file.read(1024);
    total.append(line);
  }

  return QString(total);
}

void QmitkPythonTextEditor::on_SaveScript_triggered( bool )
{
  d->m_FileName = QFileDialog::getSaveFileName(this,tr("Save File"), d->m_FileName,tr("*.py"));
  if( d->m_FileName.compare("") != 0)
  {
    std::ofstream myfile;
    myfile.open(d->m_FileName.toLocal8Bit().data());
    myfile << d->m_Content->toPlainText().toLocal8Bit().data();
    myfile.close();
  }
}

void QmitkPythonTextEditor::on_LoadScript_triggered( bool )
{
  d->m_FileName = QFileDialog::getOpenFileName( this, "Load Script", d->m_FileName, tr("*.py"));
  if( !d->m_FileName.isEmpty() )
  {
    QString contents = this->ReadFile( d->m_FileName );
    d->m_Content->setText(contents);
  }
}

void QmitkPythonTextEditor::on_RunScript_triggered( bool )
{
  if( !d->m_PythonService )
  {
    MITK_ERROR << "Python service not available.";
    return;
  }

  d->m_PythonService->Execute( d->m_Content->toPlainText().toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND );
}
