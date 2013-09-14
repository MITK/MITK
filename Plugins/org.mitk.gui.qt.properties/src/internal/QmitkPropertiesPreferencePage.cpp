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

#include "QmitkPropertiesPreferencePage.h"
#include <berryIPreferencesService.h>

const std::string QmitkPropertiesPreferencePage::FILTER_PROPERTIES = "filter properties";
const std::string QmitkPropertiesPreferencePage::SHOW_ALIASES = "show aliases";
const std::string QmitkPropertiesPreferencePage::SHOW_DESCRIPTIONS = "show descriptions";
const std::string QmitkPropertiesPreferencePage::SHOW_ALIASES_IN_DESCRIPTION = "show aliases in description";

QmitkPropertiesPreferencePage::QmitkPropertiesPreferencePage()
  : m_Control(NULL),
    m_Preferences(berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID)->GetSystemPreferences()->Node("/org.mitk.views.properties"))
{
}

QmitkPropertiesPreferencePage::~QmitkPropertiesPreferencePage()
{
}

void QmitkPropertiesPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Controls.setupUi(m_Control);

  connect(m_Controls.showDescriptionsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(OnShowDescriptionsStateChanged(int)));

  this->Update();
}

QWidget* QmitkPropertiesPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkPropertiesPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkPropertiesPreferencePage::OnShowDescriptionsStateChanged(int state)
{
  m_Controls.showAliasesInDescriptionCheckBox->setEnabled(state != Qt::Unchecked);
}

bool QmitkPropertiesPreferencePage::PerformOk()
{
  m_Preferences->PutBool(FILTER_PROPERTIES, m_Controls.filterPropertiesCheckBox->isChecked());
  m_Preferences->PutBool(SHOW_ALIASES, m_Controls.showAliasesCheckBox->isChecked());
  m_Preferences->PutBool(SHOW_DESCRIPTIONS, m_Controls.showDescriptionsCheckBox->isChecked());
  m_Preferences->PutBool(SHOW_ALIASES_IN_DESCRIPTION, m_Controls.showAliasesInDescriptionCheckBox->isChecked());

  return true;
}

void QmitkPropertiesPreferencePage::PerformCancel()
{
}

void QmitkPropertiesPreferencePage::Update()
{
  m_Controls.filterPropertiesCheckBox->setChecked(m_Preferences->GetBool(FILTER_PROPERTIES, true));
  m_Controls.showAliasesCheckBox->setChecked(m_Preferences->GetBool(SHOW_ALIASES, true));
  m_Controls.showDescriptionsCheckBox->setChecked(m_Preferences->GetBool(SHOW_DESCRIPTIONS, true));
  m_Controls.showAliasesInDescriptionCheckBox->setChecked(m_Preferences->GetBool(SHOW_ALIASES_IN_DESCRIPTION, true));
}
