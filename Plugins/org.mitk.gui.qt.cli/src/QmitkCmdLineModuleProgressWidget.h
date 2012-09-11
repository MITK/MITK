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

#include <QWidget>
#include <QTimer>
#include <ctkCmdLineModuleFutureWatcher.h>

class ctkCmdLineModuleManager;
class ctkCmdLineModuleFrontend;
class ctkCmdLineModuleFuture;

namespace Ui {
class QmitkCmdLineModuleProgressWidget;
}

namespace mitk {
class DataStorage;
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

  void Run(ctkCmdLineModuleFrontend* moduleInstance);
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

  /**
   * \brief Used to write output to console widget, and qDebug().
   */
  void PublishMessage(const QString& message);

  /**
   * \brief Destroys any images listed in m_TemporaryFileNames.
   */
  void ClearUpTemporaryFiles();

  /**
   * \brief Loads any data listed in m_OutputDataToLoad into the mitk::DataStorage.
   */
  void LoadOutputData();


  Ui::QmitkCmdLineModuleProgressWidget *m_UI;
  ctkCmdLineModuleFutureWatcher m_FutureWatcher;
  ctkCmdLineModuleManager* m_ModuleManager;
  QTimer m_PollPauseTimer;
  mitk::DataStorage* m_DataStorage;
  QString m_TemporaryDirectoryName;

  /**
   * \brief We store a list of temporary file names that are saved to disk before
   * launching a command line app, and then must be cleared up when the command line
   * app successfully finishes.
   */
  QStringList m_TemporaryFileNames;

  /**
   * \brief We store a list of output images, so that on successfull completion of
   * a command line module, we load the output data into the mitk::DataStorage.
   */
  QStringList m_OutputDataToLoad;
};

#endif // QMITKCMDLINEMODULEPROGRESSWIDGET_H
