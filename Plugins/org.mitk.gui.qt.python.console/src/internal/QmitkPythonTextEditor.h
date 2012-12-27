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

#ifndef QMITKPYTHONTEXTEDITOR_H_
#define QMITKPYTHONTEXTEDITOR_H_

#include <QTextEdit>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "QmitkPythonMediator.h"

class QmitkPythonTextEditor : public QTextEdit, public QmitkPythonPasteClient
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  QmitkPythonTextEditor(QWidget *parent = 0);
  virtual ~QmitkPythonTextEditor();

  virtual void paste(const QString& command);
signals:

protected slots:

protected:
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);

  virtual bool canInsertFromMimeData( const QMimeData *source ) const;
  //virtual void QmitkPythonTextEditor::insertFromMimeData( const QMimeData *source );

private:
};

#endif
