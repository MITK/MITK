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

#ifndef QMITKCMDLINEMODULEPROGRESSWIDGET_H
#define QMITKCMDLINEMODULEPROGRESSWIDGET_H

#include "ctkCmdLineModuleResult.h"
#include "ctkCmdLineModuleFutureWatcher.h"

#include <QWidget>
#include <QFutureWatcher>
#include <QTimer>

class ctkCmdLineModuleFuture;

namespace Ui {
class QmitkCmdLineModuleProgressWidget;
}

/**
 * \class QmitkCmdLineModuleProgressWidget
 * \brief Based on ctkCmdLineModuleExplorerProgressWidget, implements a progress widget
 * with console output, and space for storing the GUI widget once the module is running.
 */
class QmitkCmdLineModuleProgressWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkCmdLineModuleProgressWidget(QWidget *parent = 0);
  ~QmitkCmdLineModuleProgressWidget();

  void setFuture(const ctkCmdLineModuleFuture& future);

  void setTitle(const QString& title);

Q_SIGNALS:

  void clicked();

protected:

  void mouseReleaseEvent(QMouseEvent*);

private Q_SLOTS:

  void on_PauseButton_toggled(bool toggled);
  void on_RemoveButton_clicked();

  void checkModulePaused();

  void moduleStarted();
  void moduleCanceled();
  void moduleFinished();
  void moduleResumed();
  void moduleProgressRangeChanged(int progressMin, int progressMax);
  void moduleProgressTextChanged(const QString& progressText);
  void moduleProgressValueChanged(int progressValue);

private:
  Ui::QmitkCmdLineModuleProgressWidget *m_UI;

  ctkCmdLineModuleFutureWatcher m_FutureWatcher;
  QTimer m_PollPauseTimer;

};

#endif // QMITKCMDLINEMODULEPROGRESSWIDGET_H
