/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkGnuplotWidget_h
#define QmitkGnuplotWidget_h

#include <MitkQtWidgetsExtExports.h>
#include <QProcess>
#include <QScopedPointer>
#include <QWidget>
#include <itkTimeStamp.h>

class QAction;
class QMenu;

namespace Ui
{
  class QmitkGnuplotWidget;
}

class MITKQTWIDGETSEXT_EXPORT QmitkGnuplotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkGnuplotWidget(QWidget *parent = nullptr);
  ~QmitkGnuplotWidget() override;

  QString GetGnuplotPath() const;
  void SetGnuplotPath(const QString &path);

  QStringList GetCommands() const;
  void SetCommands(const QStringList &commands);

  void Update();

  QSize sizeHint() const override;

protected:
  void contextMenuEvent(QContextMenuEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

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
  QMenu *m_ContextMenu;
  QAction *m_CopyPlotAction;
  QAction *m_CopyScriptAction;
  QProcess *m_Process;
  QString m_GnuplotPath;
  QStringList m_Commands;
  itk::TimeStamp m_ModifiedTime;
  itk::TimeStamp m_UpdateTime;
};

#endif
