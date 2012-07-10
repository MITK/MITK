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
#include "ui_CommandLineModulesViewControls.h"
#include <ctkCmdLineModuleReference.h>

class ctkCmdLineModuleDescriptionDefaultFactory;
class ctkCmdLineModuleManager;

namespace berry
{
  class IBerryPreferences;
}

/*!
 * \class CommandLineModulesView
 * \brief Provides basic GUI interface to the CTK command line modules.
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
  
  void OnChooseFileButtonPressed();

protected:

  /**
   * \brief Called by framework to set the focus on the right widget
   * when this view has focus, but currently does nothing.
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
   * \brief Creates a module from a file location.
   * \param location The full absolute file name of a command line module.
   */
  void AddModule(const QString& location);

  /**
   * \brief Adds a module to the views tabbed widget, creating a new tab each time.
   * \param moduleRef A ctkCmdLineModuleReference.
   */
  void AddModuleTab(const ctkCmdLineModuleReference& moduleRef);

  /**
   * \brief We store the parent, passed in via CommandLineModulesView::CreateQtPartControl, as this class itself is not a QWidget.
   */
  QWidget *m_Parent;

  /**
   * \brief The GUI controls contain a run/stop button, and a tabbed widget, so the GUI component
   * for each command line module is added to the tabbed widget dynamically at run time.
   */
  Ui::CommandLineModulesViewControls m_Controls;

  /**
   * \brief We maintain a hashmap of tab-number to module reference.
   */
  QHash<int, ctkCmdLineModuleReference> m_MapTabToModuleRef;

  /**
   * \brief The ctkCmdLineModuleDescriptionFactory creates GUI's from an XML description,
   * and here we use a basic implementation called ctkCmdLineModuleDescriptionDefaultFactory
   * which uses QUiLoader to convert an XML document (assumed to be a .ui file) into widgets.
   */
  ctkCmdLineModuleDescriptionDefaultFactory *m_Factory;

  /**
   * \brief The manager is responsible for loading modules ...
   */
  ctkCmdLineModuleManager *m_ModuleManager;

  /**
   * \brief We store a temporary folder name, accessible via user preferences.
   */
  QString m_TemporaryDirectoryName;

  /**
   * \brief We store a folder name, where we look for modules to load.
   */
  QString m_ModulesDirectoryName;

};

#endif // CommandLineModulesView_h

