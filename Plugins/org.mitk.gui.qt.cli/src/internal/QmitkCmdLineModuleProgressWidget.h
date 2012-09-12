/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
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
#include <ctkCmdLineModuleReference.h>

class ctkCmdLineModuleManager;
class ctkCmdLineModuleFrontend;
class ctkCmdLineModuleFuture;
class QmitkCmdLineModuleFactoryGui;
class QVBoxLayout;

namespace Ui {
class QmitkCmdLineModuleProgressWidget;
}

namespace mitk {
class DataStorage;
}

/**
 * \class QmitkCmdLineModuleProgressWidget
 * \brief Based on ctkCmdLineModuleExplorerProgressWidget, implements a progress widget
 * with console output, and space for storing the GUI widgets.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cli_internal
 * \sa ctkCmdLineModuleExplorerProgressWidget
 */
class QmitkCmdLineModuleProgressWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkCmdLineModuleProgressWidget(QWidget *parent = 0);
  ~QmitkCmdLineModuleProgressWidget();

  /**
   * \brief Sets the manager on this object, and must be called immediately
   * after construction, before using the widget.
   */
  void SetManager(ctkCmdLineModuleManager* manager);

  /**
   * \brief Sets the DataStorage on this object, and must be called immediately
   * after construction, before using the widget.
   */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
   * \brief Sets the Temporary Directory on this widget, and must be called
   * immediately after construction, before using the widget.
   */
  void SetTemporaryDirectory(const QString& directoryName);

  /**
   * \brief Tells this widget, which module it is pertaining to, which is looked up via the ModuleManager.
   * \param reference
   */
  void SetModule(const ctkCmdLineModuleReference& reference);

  /**
   * \brief Returns <category>.<title>, derived from the ctkCmdLineModuleReference and
   * hence from the ctkCmdLineModuleDescription.
   */
  QString GetFullName() const;

  /**
   * \brief Called from CommandLineModulesView to see if this job has started or not,
   * as it affects the decision on when to create new widgets.
   */
  bool IsStarted() const;

  /**
   * \brief Resets this widget to the default parameters specified in the
   * command line modules XML file.
   */
  void Reset();

  /**
   * \brief Runs the module that this widget is currently referring to.
   */
  void Run();

  /**
   * \brief Shows the progress bar widgets, which can be turned off / on to save space.
   */
  void ShowProgressBar(bool visible);

  /**
   * \brief Shows the console widget, which can be turned off / on to save space.
   */
  void ShowConsole(bool visible);

Q_SIGNALS:

  // These signals so that container classes such as CommandLineModuleView
  // can keep track of how many modules are running simultaneously.

  void started();   // emmitted when the module is started.
  void finished();  // emmitted when the module is completely finished.

private Q_SLOTS:

  void OnCheckModulePaused();

  void OnPauseButtonToggled(bool toggled);
  void OnRemoveButtonClicked();

  void OnModuleStarted();
  void OnModuleCanceled();
  void OnModuleFinished();
  void OnModuleResumed();
  void OnModuleProgressRangeChanged(int progressMin, int progressMax);
  void OnModuleProgressTextChanged(const QString& progressText);
  void OnModuleProgressValueChanged(int progressValue);
  void OnOutputDataReady();
  void OnErrorDataReady();

private:

  /**
   * \brief Used to write output to the console widget, and also to qDebug().
   */
  void PublishMessage(const QString& message);

  /**
   * \brief Used to write output to the console widget, and also to qDebug().
   */
  void PublishByteArray(const QByteArray& array);

  /**
   * \brief Destroys any images listed in m_TemporaryFileNames.
   */
  void ClearUpTemporaryFiles();

  /**
   * \brief Loads any data listed in m_OutputDataToLoad into the m_DataStorage.
   */
  void LoadOutputData();

  /**
   * \brief Utility method to look up the title from the description.
   */
  QString GetTitle();

  /**
   * \brief This must be injected before the Widget is used.
   */
  ctkCmdLineModuleManager *m_ModuleManager;

  /**
   * \brief This must be injected before the Widget is used.
   */
  mitk::DataStorage *m_DataStorage;

  /**
   * \brief This must be injected before the Widget is used.
   */
  QString m_TemporaryDirectoryName;

  /**
   * \brief We instantiate the main widgets from this .ui file.
   */
  Ui::QmitkCmdLineModuleProgressWidget *m_UI;

  /**
   * \brief The m_ParametersGroupBox needs a layout.
   */
  QVBoxLayout *m_Layout;

  /**
   * \brief The ctkCmdLineModuleFrontend is created by the QmitkCmdLineModuleFactoryGui.
   */
  ctkCmdLineModuleFrontend *m_ModuleFrontEnd;

  /**
   * \brief Main object to keep track of a running command line module.
   */
  ctkCmdLineModuleFutureWatcher *m_FutureWatcher;

  /**
   * \brief Due to Qt bug 12152, we use a timer to correctly check for a paused module.
   */
  QTimer m_PollPauseTimer;

  /**
   * \brief We store a list of temporary file names that are saved to disk before
   * launching a command line app, and then must be cleared up when the command line
   * app successfully finishes.
   */
  QStringList m_TemporaryFileNames;

  /**
   * \brief We store a list of output images, so that on successful completion of
   * the command line module, we automatically load the output data into the mitk::DataStorage.
   */
  QStringList m_OutputDataToLoad;

  /**
   * \brief We track how many times the OnOutputDataReady is called.
   */
  int m_OutputCount;

  /**
   * \brief We track how many times the OnErrorDataReady is called.
   */
  int m_ErrorCount;
};

#endif // QMITKCMDLINEMODULEPROGRESSWIDGET_H
