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


#include "RTUIPreferencePage.h"

#include "mitkRTUIConstants.h"

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

//-----------------------------------------------------------------------------
RTUIPreferencePage::RTUIPreferencePage()
: m_MainControl(0), m_Controls(0)
{

}


//-----------------------------------------------------------------------------
RTUIPreferencePage::~RTUIPreferencePage()
{
  delete m_Controls;
}


//-----------------------------------------------------------------------------
void RTUIPreferencePage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void RTUIPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_PreferencesNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_PREFERENCE_NODE_ID);

  m_MainControl = new QWidget(parent);
  m_Controls = new Ui::RTUIPreferencePageControls;
  m_Controls->setupUi( m_MainControl );

  connect(m_Controls->radioDefault, SIGNAL(toggled(bool)), m_Controls->spinDefault, SLOT(setEnabled(bool)));
  connect(m_Controls->radioRelativeToMax, SIGNAL(toggled(bool)), m_Controls->spinRelativeToMax, SLOT(setEnabled(bool)));

  this->Update();
}


//-----------------------------------------------------------------------------
QWidget* RTUIPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

//-----------------------------------------------------------------------------
bool RTUIPreferencePage::PerformOk()
{
  bool useAsDefaultValue =  m_Controls->radioDefault->isChecked();
  m_PreferencesNode->PutBool(mitk::rt::UIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_AS_DEFAULT_ID,useAsDefaultValue);

  if (useAsDefaultValue)
  {
      m_PreferencesNode->PutDouble(mitk::rt::UIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID, m_Controls->spinDefault->value());
  }
  else
  {
    m_PreferencesNode->PutDouble(mitk::rt::UIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID, m_Controls->spinRelativeToMax->value()/100.0);
  }

  return true;
}


//-----------------------------------------------------------------------------
void RTUIPreferencePage::PerformCancel()
{
}


//-----------------------------------------------------------------------------
void RTUIPreferencePage::Update()
{
  bool useAsDefaultValue = m_PreferencesNode->GetBool(mitk::rt::UIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_AS_DEFAULT_ID, true);
  double doseValue = m_PreferencesNode->GetDouble(mitk::rt::UIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID, 50.0);

  m_Controls->radioDefault->setChecked(useAsDefaultValue);
  m_Controls->radioRelativeToMax->setChecked(!useAsDefaultValue);
  m_Controls->spinDefault->setEnabled(useAsDefaultValue);
  m_Controls->spinRelativeToMax->setEnabled(!useAsDefaultValue);

  if (useAsDefaultValue)
  {
    m_Controls->spinDefault->setValue(doseValue);
  }
  else
  {
    m_Controls->spinRelativeToMax->setValue(doseValue*100.0);
  }
}
