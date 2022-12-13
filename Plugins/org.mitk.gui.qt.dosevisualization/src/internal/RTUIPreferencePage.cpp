/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "RTUIPreferencePage.h"

#include "mitkRTUIConstants.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node(mitk::RTUIConstants::ROOT_PREFERENCE_NODE_ID);
  }
}

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
  auto* prefs = GetPreferences();

  bool useAsDefaultValue =  m_Controls->radioDefault->isChecked();
  prefs->PutBool(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_AS_DEFAULT_ID, useAsDefaultValue);

  if (useAsDefaultValue)
  {
    prefs->PutDouble(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID, m_Controls->spinDefault->value());
  }
  else
  {
    prefs->PutDouble(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID, m_Controls->spinRelativeToMax->value()/100.0);
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
  auto* prefs = GetPreferences();

  bool useAsDefaultValue = prefs->GetBool(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_AS_DEFAULT_ID, true);
  double doseValue = prefs->GetDouble(mitk::RTUIConstants::UNKNOWN_PRESCRIBED_DOSE_HANDLING_VALUE_ID, 50.0);

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
