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

#ifndef QmitkPropertiesPreferencePage_h
#define QmitkPropertiesPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <ui_QmitkPropertiesPreferencePage.h>

class QmitkPropertiesPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  static const std::string FILTER_PROPERTIES;
  static const std::string SHOW_ALIASES;
  static const std::string SHOW_DESCRIPTIONS;
  static const std::string SHOW_ALIASES_IN_DESCRIPTION;
  static const std::string DEVELOPER_MODE;

  QmitkPropertiesPreferencePage();
  ~QmitkPropertiesPreferencePage();

  void CreateQtControl(QWidget* parent);
  QWidget* GetQtControl() const;
  void Init(berry::IWorkbench::Pointer workbench);
  bool PerformOk();
  void PerformCancel();
  void Update();

private slots:
  void OnShowDescriptionsStateChanged(int state);

private:
  QWidget* m_Control;
  Ui::QmitkPropertiesPreferencePage m_Controls;
  berry::IPreferences::Pointer m_Preferences;
};

#endif
