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

#ifndef QmitkGnuplotWidget_h
#define QmitkGnuplotWidget_h

#include <itkTimeStamp.h>
#include <MitkQtWidgetsExtExports.h>
#include <QProcess>
#include <QScopedPointer>
#include <QWidget>

class QAction;
class QMenu;

namespace Ui
{
  class QmitkGnuplotWidget;
}

class MitkQtWidgetsExt_EXPORT QmitkGnuplotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkGnuplotWidget(QWidget* parent = NULL);
  ~QmitkGnuplotWidget();

  QString GetGnuplotPath() const;
  void SetGnuplotPath(const QString& path);

  QStringList GetCommands() const;
  void SetCommands(const QStringList& commands);

  void Update();

  QSize sizeHint() const;

protected:
  void contextMenuEvent(QContextMenuEvent* event);
  void resizeEvent(QResizeEvent* event);

private slots:
  void OnProcessStateChanged(QProcess::ProcessState state);
  void OnProcessError(QProcess::ProcessError error);
  void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void OnCopyPlot();
  void OnCopyScript();

private:
  void CreateContextMenu();
  QString CreateSetTermCommand() const;

  QScopedPointer<Ui::QmitkGnuplotWidget> m_Ui;
  QMenu* m_ContextMenu;
  QAction* m_CopyPlotAction;
  QAction* m_CopyScriptAction;
  QProcess* m_Process;
  QString m_GnuplotPath;
  QStringList m_Commands;
  itk::TimeStamp m_ModifiedTime;
  itk::TimeStamp m_UpdateTime;
};

#endif
