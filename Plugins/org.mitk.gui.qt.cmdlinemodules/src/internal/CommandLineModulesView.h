/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef CommandLineModulesView_h
#define CommandLineModulesView_h

#include <QmitkAbstractView.h>
#include <berryIPreferences.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleResult.h>
#include <ctkCmdLineModuleManager.h>

class ctkCmdLineModuleBackendLocalProcess;
class ctkCmdLineModuleDirectoryWatcher;
class CommandLineModulesViewControls;
class QmitkCmdLineModuleRunner;
class QAction;
class QVBoxLayout;

namespace berry
{
  struct IBerryPreferences;
}

/*!
 * \class CommandLineModulesView
 * \brief Provides basic GUI interface to the CTK command line modules.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 * \sa QmitkAbstractView
 */
class CommandLineModulesView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  CommandLineModulesView();
  virtual ~CommandLineModulesView();

  /**
   * \brief Main method, called by framework to create the GUI at the right time.
   * \param parent The parent QWidget, as this class itself is not a QWidget subclass.
   */
  virtual void CreateQtPartControl(QWidget *parent);

  /**
   * \brief Called by the framework to indicate that the preferences have changed.
   * \param prefs not used, as we call RetrievePreferenceValues().
   */
  void OnPreferencesChanged(const berry::IBerryPreferences* prefs);

protected Q_SLOTS:

  /**
   * \brief Called when the ctkMenuComboBox has the menu selection changed,
   * and causes the corresponding GUI to be displayed.
   */
  void OnActionChanged(QAction*);

  /**
   * \brief Slot that is called when the restore defaults button is pressed,
   * to reset the current GUI form to the default values, if the XML specifies defaults.
   */
  void OnRestoreButtonPressed();

  /**
   * \brief Slot that is called when the Run button is pressed to run the current module.
   */
  void OnRunButtonPressed();

  /**
   * \brief Alerts the user of any errors comming out of the directory watcher.
   */
  void OnDirectoryWatcherErrorsDetected(const QString&);

protected:

  /**
   * \brief Called by framework to set the focus on the right widget
   * when this view has focus, so currently, thats the ctkMenuCombo box.
   */
  virtual void SetFocus();

private slots:

  /**
   * \brief Called when the user clicks to close a tab, and removes the front end from m_ListOfModules
   */
  void OnTabCloseRequested(int tabNumber);

  /**
   * \brief Called from QmitkCmdLineModuleProgressWidget to indicate a job has started.
   */
  void OnJobStarted();

  /**
   * \brief Called from QmitkCmdLineModuleProgressWidget to indicate a job has finished.
   */
  void OnJobFinished();

  /**
   * \brief Called when the user hits the 'clear XML cache' button.
   */
  void OnClearCache();

  /**
   * \brief Called when the user hits the 'reload modules' button.
   */
  void OnReloadModules();

private:

  /**
   * \brief Called on startup and by OnPreferencesChanged to load all
   * preferences except the temporary folder into member variables.
   */
  void RetrieveAndStorePreferenceValues();

  /**
   * \brief Called on startup and by OnPreferencesChanged to load the temporary folder
   * preference into member variable m_TemporaryDirectoryName.
   */
  void RetrieveAndStoreTemporaryDirectoryPreferenceValues();

  /**
   * \brief Called on startup and by OnPreferencesChanged to set the validation mode, but will require a restart.
   */
  void RetrieveAndStoreValidationMode();

  /**
   * \brief Called to get hold of the actual preferences node.
   */
  berry::IBerryPreferences::Pointer RetrievePreferences();

  /**
   * \brief Search all modules for the one matching the given identifier.
   * \param fullName The "fullName" is the <category>.<title> from the XML.
   * \return ctkCmdLineModuleReference the reference corresponding to the fullName, or an invalid reference if non found.
   */
  ctkCmdLineModuleReference GetReferenceByFullName(QString fullName);

  /**
   * \brief Raises a message box asking the user to select a module first.
   */
  void AskUserToSelectAModule() const;

  /**
   * \brief Enables or Disables the Run Button.
   */
  void UpdateRunButtonEnabledStatus();

  /**
   * \brief The GUI controls contain a reset and run button, and a QWidget container, and the GUI component
   * for each command line module is added to the QWidget dynamically at run time.
   */
  CommandLineModulesViewControls *m_Controls;

  /**
   * \brief We store the parent, passed in via CommandLineModulesView::CreateQtPartControl,
   * as this class itself is not a QWidget.
   */
  QWidget *m_Parent;

  /**
   * \brief We keep a local layout, and arrange a display of QmitkCmdLineModuleProgressWidget,
   * where each QmitkCmdLineModuleProgressWidget represents a single running job.
   */
  QVBoxLayout *m_Layout;

  /**
   * \brief The manager is responsible for loading and instantiating command line modules.
   */
  ctkCmdLineModuleManager *m_ModuleManager;

  /**
   * \brief We are using a back-end that runs locally installed command line programs.
   */
  ctkCmdLineModuleBackendLocalProcess *m_ModuleBackend;

  /**
   * \brief The ctkCmdLineModuleDirectoryWatcher maintains the list of directories
   * we are using to load modules, to provide automatic updates.
   */
  ctkCmdLineModuleDirectoryWatcher *m_DirectoryWatcher;

  /**
   * \brief We store a temporary folder name, accessible via user preferences.
   */
  QString m_TemporaryDirectoryName;

  /**
   * \brief We store an output folder name, accessible via user preferences for when
   * the file specified in a default output path is not within a writable directory.
   */
  QString m_OutputDirectoryName;

  /**
   * \brief Cache the list of directory paths locally to avoid repeatedly trying to update Directory Watcher.
   */
  QStringList m_DirectoryPaths;

  /**
   * \brief Cache the list of module/executable paths locally to avoid repeatedly trying to update Directory Watcher.
   */
  QStringList m_ModulePaths;

  /**
   * \brief We store the validation mode, accessisble via user preferences.
   */
  ctkCmdLineModuleManager::ValidationMode m_ValidationMode;

  /**
   * \brief We store the maximum number of concurrent processes, and disable the run button accordingly.
   */
  int m_MaximumConcurrentProcesses;

  /**
   * \brief Counts the number of currently running processes.
   */
  int m_CurrentlyRunningProcesses;

  /**
   * \brief Member variable, taken from preference page.
   */
  bool m_DebugOutput;

  /**
   * \brief Member variable, taken from preferences page.
   */
  int m_XmlTimeoutSeconds;

  /**
   * \brief We keep a list of front ends to match the m_TabWidget.
   */
  QList<ctkCmdLineModuleFrontend*> m_ListOfModules;
};

#endif // CommandLineModulesView_h
