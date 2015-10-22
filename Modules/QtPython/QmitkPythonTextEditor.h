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
#include <MitkQtPythonExports.h>
struct QmitkPythonTextEditorData;

///
/// this is a python text editor with syntax highlightning
class MITKQTPYTHON_EXPORT QmitkPythonTextEditor : public QWidget
{
  Q_OBJECT

public:
  QmitkPythonTextEditor(QWidget *parent = 0);
  virtual ~QmitkPythonTextEditor();
public slots:
  void Paste(const QString& command);

protected slots:
  void on_SaveScript_triggered(bool checked=false);
  void on_LoadScript_triggered(bool checked=false);
  void on_RunScript_triggered(bool checked=false);

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void dropEvent(QDropEvent *event);
  //bool canInsertFromMimeData( const QMimeData *source ) const;
  QString ReadFile(const QString &filename);

private:
  QmitkPythonTextEditorData* d;
};

#endif
