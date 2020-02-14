/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  QmitkPythonTextEditor(QWidget *parent = nullptr);
  ~QmitkPythonTextEditor() override;
public slots:
  void Paste(const QString& command);

protected slots:
  void on_SaveScript_triggered(bool checked=false);
  void on_LoadScript_triggered(bool checked=false);
  void on_RunScript_triggered(bool checked=false);

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  //bool canInsertFromMimeData( const QMimeData *source ) const;
  QString ReadFile(const QString &filename);

private:
  QmitkPythonTextEditorData* d;
};

#endif
