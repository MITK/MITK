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

#ifndef QmitkSimulationPreferencePage_h
#define QmitkSimulationPreferencePage_h

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>
#include <org_mitk_gui_qt_simulation_Export.h>
#include <ui_QmitkSimulationPreferencePageControls.h>

class SIMULATION_EXPORT QmitkSimulationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage);

public:
  QmitkSimulationPreferencePage();
  ~QmitkSimulationPreferencePage();

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  void Init(berry::IWorkbench::Pointer) override;
  void PerformCancel() override;
  bool PerformOk() override;
  void Update() override;

private slots:
  void OnAddButtonClicked();
  void OnSelectedPluginChanged();
  void OnRemoveButtonClicked();

private:
  berry::IPreferences::Pointer m_Preferences;
  Ui::QmitkSimulationPreferencePageControls m_Controls;
  QWidget* m_Control;
};

#endif
