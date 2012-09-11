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

class QmitkCmdLineModuleFactoryGui;
class ctkCmdLineModuleManager;
class ctkCmdLineModuleFrontend;
class ctkCmdLineModuleBackendLocalProcess;
class ctkCmdLineModuleDirectoryWatcher;
class ctkCmdLineModuleFutureWatcher;
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
   * meaning that a new GUI page be created in another tab.
   */
  void OnActionChanged(QAction*);

  /**
   * \brief Slot that is called when the restore defaults button is pressed,
   * to reset the current GUI form to the default values.
   */
  void OnRestoreButtonPressed();

protected:

  /**
   * \brief Called by framework to set the focus on the right widget
   * when this view has focus, so currently, thats the combo box.
   */
  virtual void SetFocus();

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
   * \brief Called to get hold of the actual preferences node.
   */
  berry::IBerryPreferences::Pointer RetrievePreferences();

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
   * \brief The ctkCmdLineModuleDirectoryWatcher maintains the list of directories
   * we are using to load modules, to provide automatic updates.
   */
  ctkCmdLineModuleDirectoryWatcher *m_DirectoryWatcher;

  /**
   * \brief We use this map to decide if we want to create more tabs or not.
   */
  QHash<int, ctkCmdLineModuleFrontend*> m_MapTabToModuleInstance;

  /**
   * \brief We store a temporary folder name, accessible via user preferences.
   */
  QString m_TemporaryDirectoryName;

  /**
   * \brief Member variable, taken from preference page.
   */
  bool m_DebugOutput;
};

#endif // CommandLineModulesView_h

