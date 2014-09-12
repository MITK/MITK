/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKCMDLINEMODULERUNNER_H
#define QMITKCMDLINEMODULERUNNER_H

#include <QWidget>
#include <QTimer>
#include <QList>

#include <ctkCmdLineModuleParameter.h>
#include <mitkDataNode.h>

class QVBoxLayout;
class QTemporaryFile;
class QmitkCmdLineModuleGui;
class ctkCmdLineModuleManager;
class ctkCmdLineModuleFutureWatcher;

namespace Ui {
class QmitkCmdLineModuleProgressWidget;
}

namespace mitk {
class DataStorage;
}

/**
 * \class QmitkCmdLineModuleRunner
 * \brief Based on ctkCmdLineModuleExplorerProgressWidget, implements a progress widget
 * with console output, and space for storing the GUI widgets.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 * \sa ctkCmdLineModuleExplorerProgressWidget
 */
class QmitkCmdLineModuleRunner : public QWidget
{
  Q_OBJECT

public:

  QmitkCmdLineModuleRunner(QWidget *parent = 0);
  virtual ~QmitkCmdLineModuleRunner();

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
   * \brief Sets the Output Directory on this widget, and must be called
   * immediately after construction, before using the widget.
   */
  void SetOutputDirectory(const QString& directoryName);

  /**
   * \brief Tells this widget, which module frontend it is running
   * \param frontEnd our QmitkCmdLineModuleGui class derived from ctkCmdLineModuleFrontend
   */
  void SetFrontend(QmitkCmdLineModuleGui* frontEnd);

  /**
   * \brief Runs the module that this widget is currently referring to.
   */
  void Run();

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
   * \brief Simply returns true if this widget is considered as having been started.
   */
  bool IsStarted() const;

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
   * \brief Saves temporary image to file.
   * \param[in] node non-NULL pointer to node containing a non-NULL mitk::Image.
   * \param[out] errorMessage which if not empty means an error occurred.
   * \return QTemporaryFile temporary file that the caller is responsible for deleting.
   *
   * If the returned QTemporaryFile is NULL, check errorMessage.
   * If the returned QTemporaryFile is not-NULL, there could still be data in the errorMessage.
   * It could be that this method tried n file extensions, before finding a successful one.
   * In this case, the returned QTemporaryFile is the successful one, and the errorMessage contains error messages of all the failed attempts.
   */
  QTemporaryFile* SaveTemporaryImage(const ctkCmdLineModuleParameter& parameter, mitk::DataNode::ConstPointer node, QString& errorMessage) const;

  /**
   * \brief Utility method to look up the title from the description.
   */
  QString GetTitle();

  /**
   * \brief Returns <category>.<title>, derived from the ctkCmdLineModuleReference and
   * hence from the ctkCmdLineModuleDescription.
   */
  QString GetFullName() const;

  /**
   * \brief Takes nodeName, and makes sure that it only contains A-Z, a-z, 0-9, hyphen and underscore,
   * and does not use hyphen as the first character.
   *
   * Inspired by <a href="http://www.boost.org/doc/libs/1_43_0/libs/filesystem/doc/portability_guide.htm">boost recommendations</a>.
   */
  QString GetValidNodeName(const QString& nodeName) const;

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
  QString m_OutputDirectoryName;

  /**
   * \brief We instantiate the main widgets from this .ui file.
   */
  Ui::QmitkCmdLineModuleProgressWidget *m_UI;

  /**
   * \brief The m_ParametersGroupBox needs a layout.
   */
  QVBoxLayout *m_Layout;

  /**
   * \brief The QmitkCmdLineModuleGui is created by the QmitkCmdLineModuleFactoryGui outside
   * of this class and injected into this class before being run.
   */
  QmitkCmdLineModuleGui *m_ModuleFrontEnd;

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
  QList<QTemporaryFile*> m_TemporaryFiles;

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

#endif // QMITKCMDLINEMODULERUNNER_H
