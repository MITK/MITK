/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef COMMANDLINEMODULESPREFERENCESPAGE_H
#define COMMANDLINEMODULESPREFERENCESPAGE_H

#include "berryIQtPreferencePage.h"
#include "berryIPreferences.h"
#include <org_mitk_gui_qt_cmdlinemodules_Export.h>

class QWidget;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QmitkDirectoryListWidget;
class QmitkFileListWidget;
class ctkDirectoryButton;

/**
 * \class CommandLineModulesPreferencesPage
 * \brief Preference page for CommandLineModulesView
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 */
class CommandLineModulesPreferencesPage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  CommandLineModulesPreferencesPage();
  ~CommandLineModulesPreferencesPage();

  /**
   * \brief Called by framework to initialise this preference page, but currently does nothing.
   * \param workbench The workbench.
   */
  void Init(berry::IWorkbench::Pointer workbench);

  /**
   * \brief Called by framework to create the GUI, and connect signals and slots.
   * \param widget The Qt widget that acts as parent to all GUI components, as this class itself is not derived from QWidget.
   */
  void CreateQtControl(QWidget* widget);

  /**
   * \brief Required by framework to get hold of the GUI.
   * \return QWidget* the top most QWidget for the GUI.
   */
  QWidget* GetQtControl() const;

  /**
   * \see IPreferencePage::PerformOk
   */
  virtual bool PerformOk();

  /**
   * \see IPreferencePage::PerformCancel
   */
  virtual void PerformCancel();

  /**
   * \see IPreferencePage::Update
   */
  virtual void Update();

public slots:

protected:

  QWidget                  *m_MainControl;
  QCheckBox                *m_DebugOutput;
  QCheckBox                *m_ShowAdvancedWidgets;
  ctkDirectoryButton       *m_OutputDirectory;
  ctkDirectoryButton       *m_TemporaryDirectory;
  QmitkDirectoryListWidget *m_ModulesDirectories;
  QmitkFileListWidget      *m_ModulesFiles;
  QGridLayout              *m_GridLayoutForLoadCheckboxes;
  QCheckBox                *m_LoadFromHomeDir;
  QCheckBox                *m_LoadFromHomeDirCliModules;
  QCheckBox                *m_LoadFromCurrentDir;
  QCheckBox                *m_LoadFromCurrentDirCliModules;
  QCheckBox                *m_LoadFromApplicationDir;
  QCheckBox                *m_LoadFromApplicationDirCliModules;
  QCheckBox                *m_LoadFromAutoLoadPathDir;

  QComboBox                *m_ValidationMode;
  QSpinBox                 *m_XmlTimeoutInSeconds;
  QSpinBox                 *m_MaximumNumberProcesses;

  berry::IPreferences::Pointer m_CLIPreferencesNode;

private:

  std::string ConvertToStdString(const QStringList& list);
};

#endif // COMMANDLINEMODULESPREFERENCESPAGE_H
