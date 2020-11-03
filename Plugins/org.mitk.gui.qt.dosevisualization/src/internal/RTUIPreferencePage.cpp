/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "RTUIPreferencePage.h"

#include "mitkRTUIConstants.h"

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

//-----------------------------------------------------------------------------
RTUIPreferencePage::RTUIPreferencePage()
  : m_MainControl(nullptr), m_Controls(nullptr)
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
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_PreferencesNode = prefService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_PREFERENCE_NODE_ID.c_str());

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
  m_PreferencesNode->PutBool(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_AS_DEFAULT_ID.c_str(),useAsDefaultValue);

  if (useAsDefaultValue)
  {
    m_PreferencesNode->PutDouble(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID.c_str(), m_Controls->spinDefault->value());
  }
  else
  {
    m_PreferencesNode->PutDouble(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID.c_str(), m_Controls->spinRelativeToMax->value()/100.0);
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
  bool useAsDefaultValue = m_PreferencesNode->GetBool(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_AS_DEFAULT_ID.c_str(), true);
  double doseValue = m_PreferencesNode->GetDouble(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID.c_str(), 50.0);

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
