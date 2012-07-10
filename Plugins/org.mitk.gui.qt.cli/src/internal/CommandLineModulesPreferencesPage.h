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
 
#ifndef COMMANDLINEMODULESPREFERENCESPAGE_H
#define COMMANDLINEMODULESPREFERENCESPAGE_H

#include "berryIQtPreferencePage.h"
#include <org_mitk_gui_qt_cli_Export.h>
#include <berryIPreferences.h>

class QWidget;
class ctkDirectoryButton;

class CommandLineModulesPreferencesPage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  CommandLineModulesPreferencesPage();
  ~CommandLineModulesPreferencesPage();

  /**
   * \brief We store the temporary directory preferences node name.
   */
  static const std::string TEMPORARY_DIRECTORY_NODE_NAME;

  /**
   * \brief We store the modules directory preferences node name.
   */
  static const std::string MODULES_DIRECTORY_NODE_NAME;

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

protected:
  QWidget            *m_MainControl;
  ctkDirectoryButton *m_TemporaryDirectory;
  ctkDirectoryButton *m_ModulesDirectory;
  berry::IPreferences::Pointer m_CLIPreferencesNode;
};

#endif // COMMANDLINEMODULESPREFERENCESPAGE_H
