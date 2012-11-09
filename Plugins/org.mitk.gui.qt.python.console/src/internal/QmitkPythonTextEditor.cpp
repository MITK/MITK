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


QmitkPythonTextEditor::QmitkPythonTextEditor(QWidget *parent)
:QTextEdit(parent)
{
  QmitkPythonMediator::getInstance()->registerPasteCommandClient( this );
}

QmitkPythonTextEditor::~QmitkPythonTextEditor()
{
  QmitkPythonMediator::getInstance()->unregisterPasteCommandClient( this );
}

void QmitkPythonTextEditor::dragEnterEvent(QDragEnterEvent *event)
{
  event->accept();
}

void QmitkPythonTextEditor::paste(const QString& command)
{
  this->insertPlainText(command + "\n");
}
void QmitkPythonTextEditor::dropEvent(QDropEvent *event)
{
  //QDropEvent drop(event->pos(),Qt::MoveAction, event->mimeData(), event->mouseButtons(), Qt::ShiftModifier, event->type());
  //QTextEdit::dropEvent(&drop);
  QList<QUrl> urls = event->mimeData()->urls();
  for(int i = 0; i < urls.size(); i++)
  {
    this->insertPlainText(urls[i].toString().append("\n"));
  }

}

bool QmitkPythonTextEditor::canInsertFromMimeData( const QMimeData *source ) const
{
  return true;
}

/*
void QmitkPythonTextEditor::insertFromMimeData( const QMimeData *source )
{
  if (source->hasImage())
  {
    QImage image = qvariant_cast<QImage>(source->imageData());
    QTextCursor cursor = this->textCursor();
    QTextDocument *document = this->document();
    document->addResource(QTextDocument::ImageResource, QUrl("image"), image);
    cursor.insertImage("image");
  }
}*/
