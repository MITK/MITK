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
#include <ctkCmdLineModuleReference.h>
#include <ctkCmdLineModuleResult.h>
#include <QFutureWatcher>

class ctkCmdLineModuleManager;
class QmitkCmdLineModuleFactoryGui;
class ctkCmdLineModuleMenuFactoryQtGui;
class ctkCmdLineModuleFrontend;
class ctkCmdLineModuleBackendLocalProcess;
class ctkCmdLineModuleDirectoryWatcher;
class CommandLineModulesViewControls;
class QAction;

namespace berry
{
  class IBerryPreferences;
}

/*!
 * \class CommandLineModulesView
 * \brief Provides basic GUI interface to the CTK command line modules.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cli_internal
 *
 * This class is a basic interface to the CTK command line modules library.
 * Ideally, an MITK base application would load command line modules dynamically
 * at runtime, and promote them to the main window menu bar to appear as fully
 * fledged Views. However, currently this is not supported, and so this view
 * provides a simplified interface, contained within one plugin in order to
 * facilitate testing, debugging, and understanding of use-cases.
 *
 * \sa QmitkAbstractView
 */
class CommandLineModulesView : public QmitkAbstractView
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
public:

  static const std::string VIEW_ID;

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
   * meaning that a new GUI page be created in another tab.
   */
  void OnActionChanged(QAction*);

  /**
   * \brief Slot to launch the command line module.
   */
  void OnRunButtonPressed();

  /**
   * \brief Slot to stop (kill) the command line module.
   */
  void OnStopButtonPressed();

  /**
   * \brief Slot to restore the form to the default parameters.
   */
  void OnRestoreDefaultsButtonPressed();

  /**
   * \brief Slot called from QFutureWatcher<ctkCmdLineModuleResult> when the module has been launched.
   */
  void OnModuleStarted();

  /**
   * \brief Slot called from QFutureWatcher<ctkCmdLineModuleResult> when the module has finished.
   */
  void OnModuleFinished();

  /**
   * \brief Slot called from QFutureWatcher<ctkCmdLineModuleResult> to notify of any progress,
   * currently just printing console debug messages.
   */
  void OnModuleProgressValueChanged(int);

  /**
   * \brief Slot called from QFutureWatcher<ctkCmdLineModuleResult> to notify of any progress,
   * currently just printing console debug messages.
   */
  void OnModuleProgressTextChanged(QString);

  /**
   * \brief We register this slot with the ctkCmdLineModuleManager so that if
   * the modulesAdded or modulesRemoved signals are emmitted from ctkCmdLineModuleManager
   * we rebuild the whole menu. In future this could be made more efficient, and only add/remove single items.
   */
  void OnModulesChanged();

protected:

  /**
   * \brief Called by framework to set the focus on the right widget
   * when this view has focus, so currently, thats the combo box.
   */
  virtual void SetFocus();

private:

  /**
   * \brief Called on startup and by OnPreferencesChanged to load the preferences into member variables.
   */
  void RetrievePreferenceValues();

  /**
   * \brief Search the internal datastructure (QHash) to find the reference that matches the identifier.
   * \param identifier The identifier used in the front end combo box widget, currently title.
   * \return ctkCmdLineModuleReference the reference corresponding to the identifier, or an invalid reference if non found.
   */
  ctkCmdLineModuleReference GetReferenceByIdentifier(QString identifier);

  /**
   * \brief Adds a module to the views tabbed widget, creating a new tab each time.
   * \param moduleRef A ctkCmdLineModuleReference.
   */
  void AddModuleTab(const ctkCmdLineModuleReference& moduleRef);

  /**
   * \brief Destroys any images listed in m_TemporaryFileNames, silently failing.
   */
  void ClearUpTemporaryFiles();

  /**
   * \brief Loads any data listed in m_OutputDataToLoad into the mitk::DataStorage.
   */
  void LoadOutputData();

  /**
   * \brief The GUI controls contain a run/stop button, and a tabbed widget, and the GUI component
   * for each command line module is added to the tabbed widget dynamically at run time.
   */
  CommandLineModulesViewControls *m_Controls;

  /**
   * \brief We store the parent, passed in via CommandLineModulesView::CreateQtPartControl, as this class itself is not a QWidget.
   */
  QWidget *m_Parent;

  /**
   * \brief The manager is responsible for loading and instantiating command line modules.
   */
  ctkCmdLineModuleManager *m_ModuleManager;

  /**
   * \brief We are using a back-end that runs locally installed command line programs.
   */
  ctkCmdLineModuleBackendLocalProcess *m_ModuleBackend;

  /**
   * \brief The QmitkCmdLineModuleFactoryGui builds a gui for each plugin.
   */
  QmitkCmdLineModuleFactoryGui *m_ModuleFactory;

  /**
   * \brief The ctkCmdLineModuleMenuFactoryQtGui will build a QMenu from the list of available modules.
   */
  ctkCmdLineModuleMenuFactoryQtGui *m_MenuFactory;

  /**
   * \brief The ctkCmdLineModuleDirectoryWatcher maintains the list of directories
   * we are using to load modules, to provide automatic updates.
   */
  ctkCmdLineModuleDirectoryWatcher *m_DirectoryWatcher;

  /**
   * \brief This is the QFutureWatcher that will watch for when the process has finished, and call OnFutureFinished() slot.
   */
  QFutureWatcher<ctkCmdLineModuleResult>* m_Watcher;

  /**
   * \brief We use this map to decide if we want to create more tabs or not.
   */
  QHash<int, ctkCmdLineModuleFrontend*> m_MapTabToModuleInstance;

  /**
   * \brief We store a temporary folder name, accessible via user preferences.
   */
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

  /**
   * \brief Member variable, taken from preference page.
   */
  bool m_DebugOutput;

};

#endif // CommandLineModulesView_h

