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

#ifndef QMITKDATAMANAGERHOTKEYSPREFPAGE_H
#define QMITKDATAMANAGERHOTKEYSPREFPAGE_H

#include <org_mitk_gui_qt_datamanager_Export.h>

// blueberry ui qt plugin
#include <berryIQtPreferencePage.h>

// qt
#include <QWidget>

// c++
#include <map>

class QmitkHotkeyLineEdit;

struct MITK_QT_DATAMANAGER QmitkDataManagerHotkeysPrefPage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkDataManagerHotkeysPrefPage();

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;

  QWidget* GetQtControl() const override;

  /**
  * @brief \see IPreferencePage::PerformOk()
  */
  bool PerformOk() override;
  /**
  * @brief \see IPreferencePage::PerformCancel()
  */
  void PerformCancel() override;
  /**
  * @brief \see IPreferencePage::Update()
  */
  void Update() override;

protected:
  /**
  * @brief The node from which the properties are taken (will be catched from the preferences service in ctor)
  *
  *
  */
  berry::IPreferences::WeakPtr m_DataManagerHotkeysPreferencesNode;
  /**
  * @brief Maps a label to hotkey lineedit, e.g. "Toggle Visibility of selected nodes" => QmitkHotkeyLineEdit
  *
  *
  */
  std::map<QString, QmitkHotkeyLineEdit*> m_HotkeyEditors;

  QWidget* m_MainControl;
};

#endif // QMITKDATAMANAGERHOTKEYSPREFPAGE_H
