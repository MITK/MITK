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

#ifndef CommandLineModulesView_h
#define CommandLineModulesView_h

#include <QmitkAbstractView.h>
#include "CommandLineModulesViewControls.h"
#include <ctkCmdLineModuleReference.h>

class ctkCmdLineModuleManager;
class ctkCmdLineModuleInstance;
class QAction;

namespace berry
{
  class IBerryPreferences;
}

/*!
 * \class CommandLineModulesView
 * \brief Provides basic GUI interface to the CTK command line modules.
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 *
 * The intention of this class is that it is an intermediate integration point
 * for the CTK command line modules. Ideally, an MITK base application would
 * load command line modules dynamically at runtime, and promote them to the
 * main window menu bar to appear as fully fledged views. However, currently
 * this is not supported, and so this view provides a simplified interface
 * in order to facilitate testing, debugging, and understanding of use-cases.
 *
 * \sa QmitkAbstractView
 * \ingroup org_mitk_gui_qt_cli_internal
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

protected slots:
  
  void OnActionChanged(QAction*);
  void OnRunButtonPressed();
  void OnStopButtonPressed();
  void OnRestoreDefaultsButtonPressed();
  void OnFutureFinished();

protected:

  /**
   * \brief Called by framework to set the focus on the right widget
   * when this view has focus, so currently, thats the combo box to select a module.
   */
  virtual void SetFocus();

private:

  /**
   * \brief Called by the framework to indicate that the preferences have changed.
   * \param prefs not used.
   */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* prefs);

  /**
   * \brief Called on startup and by OnPreferencesChanged to load the preferences into member variables.
   */
  void RetrievePreferenceValues();

  /**
   * \brief Given a list of absolute or relative paths, will search for modules to load.
   * \param paths QStringList containing a list of paths to search.
   * \return QHash<QString, ctkCmdLineModuleReference> Hash map of filename to reference.
   */
  QHash<QString, ctkCmdLineModuleReference> LoadModuleReferencesFromPaths(QStringList& paths);

  /**
   * \brief Constructs a nested menu, for all the items in the QHash.
   * \param references Hash map of filename to reference.
   * \return QMenu* a nested menu.
   */
  QMenu* CreateMenuFromReferences(QHash<QString, ctkCmdLineModuleReference>& references);

  /**
   * \brief Search the internal datastructure (QHash) to find the reference that matches the identifier (title).
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
   * \brief The GUI controls contain a run/stop button, and a tabbed widget, so the GUI component
   * for each command line module is added to the tabbed widget dynamically at run time.
   */
  CommandLineModulesViewControls *m_Controls;

  /**
   * \brief We store the parent, passed in via CommandLineModulesView::CreateQtPartControl, as this class itself is not a QWidget.
   */
  QWidget *m_Parent;

  /**
   * \brief The manager is responsible for loading modules ...
   */
  ctkCmdLineModuleManager *m_ModuleManager;

  /**
   * \brief We store a map of filename to reference, and use the reference to populate the combo box.
   */
  QHash<QString, ctkCmdLineModuleReference> m_MapFilenameToReference;

  /**
   * \brief We use this map to decide if we want to create more tabs or not.
   */
  QHash<int, ctkCmdLineModuleInstance*> m_MapTabToModuleInstance;

  /**
   * \brief We store a temporary folder name, accessible via user preferences.
   */
  QString m_TemporaryDirectoryName;

  /**
   * \brief We store list of folder names, to avoid re-loading modules if
   * RetrievePreferenceValues is called multiple times.
   */
  QStringList m_ModulesDirectoryNames;

  /**
   * \brief Member variable, taken from preference page.
   */
  bool m_DebugOutput;

};

#endif // CommandLineModulesView_h

