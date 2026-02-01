/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRestApiPreferencePage.h"
#include <ui_QmitkRestApiPreferencesControls.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkRestServerConfig.h>
#include <mitkIRestServerService.h>

#include <usModuleRegistry.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("/org.mitk.restapi");
  }
}

QmitkRestApiPreferencePage::QmitkRestApiPreferencePage()
  : m_Control(nullptr),
    m_Ui(new Ui::QmitkRestApiPreferencesControls)
{
}

QmitkRestApiPreferencePage::~QmitkRestApiPreferencePage() = default;

void QmitkRestApiPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkRestApiPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  this->Update();
}

QWidget* QmitkRestApiPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkRestApiPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  // Save to preferences
  prefs->PutBool("enabled", m_Ui->m_EnabledCheckBox->isChecked());
  prefs->PutBool("autoStart", m_Ui->m_AutoStartCheckBox->isChecked());
  prefs->Put("host", m_Ui->m_HostLineEdit->text().toStdString());
  prefs->PutInt("port", m_Ui->m_PortSpinBox->value());
  prefs->PutInt("threadPoolSize", m_Ui->m_ThreadPoolSizeSpinBox->value());
  prefs->PutInt("readTimeoutSeconds", m_Ui->m_ReadTimeoutSpinBox->value());
  prefs->PutInt("writeTimeoutSeconds", m_Ui->m_WriteTimeoutSpinBox->value());

  // Also update the REST server's pending configuration directly
  auto restModule = us::ModuleRegistry::GetModule("MitkRESTAPI");
  if (restModule != nullptr)
  {
    auto* context = restModule->GetModuleContext();
    if (context != nullptr)
    {
      auto refs = context->GetServiceReferences<mitk::IRestServerService>();
      if (!refs.empty())
      {
        auto* service = context->GetService(refs.front());
        if (service != nullptr)
        {
          mitk::RestServerConfig config;
          config.enabled = m_Ui->m_EnabledCheckBox->isChecked();
          config.host = m_Ui->m_HostLineEdit->text().toStdString();
          config.port = m_Ui->m_PortSpinBox->value();
          config.threadPoolSize = m_Ui->m_ThreadPoolSizeSpinBox->value();
          config.readTimeoutSeconds = m_Ui->m_ReadTimeoutSpinBox->value();
          config.writeTimeoutSeconds = m_Ui->m_WriteTimeoutSpinBox->value();

          service->SetConfig(config);
        }
      }
    }
  }

  return true;
}

void QmitkRestApiPreferencePage::PerformCancel()
{
}

void QmitkRestApiPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  // Use RestServerConfig defaults as single source of truth
  mitk::RestServerConfig defaults;

  m_Ui->m_EnabledCheckBox->setChecked(prefs->GetBool("enabled", defaults.enabled));
  m_Ui->m_AutoStartCheckBox->setChecked(prefs->GetBool("autoStart", false));
  m_Ui->m_HostLineEdit->setText(QString::fromStdString(prefs->Get("host", defaults.host)));
  m_Ui->m_PortSpinBox->setValue(prefs->GetInt("port", defaults.port));
  m_Ui->m_ThreadPoolSizeSpinBox->setValue(prefs->GetInt("threadPoolSize", defaults.threadPoolSize));
  m_Ui->m_ReadTimeoutSpinBox->setValue(prefs->GetInt("readTimeoutSeconds", defaults.readTimeoutSeconds));
  m_Ui->m_WriteTimeoutSpinBox->setValue(prefs->GetInt("writeTimeoutSeconds", defaults.writeTimeoutSeconds));
}
