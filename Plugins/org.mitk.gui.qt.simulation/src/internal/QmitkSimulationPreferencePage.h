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

#include <berryIQtPreferencePage.h>
#include <berryIPreferences.h>
#include <org_mitk_gui_qt_simulation_Export.h>
#include <ui_QmitkSimulationPreferencePageControls.h>

class SIMULATION_EXPORT QmitkSimulationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkSimulationPreferencePage();
  ~QmitkSimulationPreferencePage();

  void CreateQtControl(QWidget* parent);
  QWidget* GetQtControl() const;
  void Init(berry::IWorkbench::Pointer workbench);
  void PerformCancel();
  bool PerformOk();
  void Update();

private:
  QWidget* m_Control;
  Ui::QmitkSimulationPreferencePageControls m_Controls;
  berry::IPreferences::Pointer m_Preferences;
};

#endif
