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

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QRandomGenerator>

#include <sstream>
#include <iomanip>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.restapi");
  }
}

QmitkRestApiPreferencePage::QmitkRestApiPreferencePage()
  : m_Control(nullptr),
    m_Ui(std::make_unique<Ui::QmitkRestApiPreferencesControls>())
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

  // Connect log limit checkbox to spin box
  connect(m_Ui->m_LogLimitEnabledCheckBox, &QCheckBox::toggled,
          m_Ui->m_LogLimitSpinBox, &QSpinBox::setEnabled);

  // Connect security controls
  connect(m_Ui->m_ClientAccessModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &QmitkRestApiPreferencePage::OnClientAccessModeChanged);

  connect(m_Ui->m_ShowTokenButton, &QPushButton::clicked,
          this, &QmitkRestApiPreferencePage::OnShowTokenToggled);

  connect(m_Ui->m_GenerateTokenButton, &QPushButton::clicked,
          this, &QmitkRestApiPreferencePage::OnGenerateToken);

  connect(m_Ui->m_CopyTokenButton, &QPushButton::clicked,
          this, &QmitkRestApiPreferencePage::OnCopyToken);

  connect(m_Ui->m_FileAccessModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &QmitkRestApiPreferencePage::OnFileAccessModeChanged);

  connect(m_Ui->m_AddDirButton, &QPushButton::clicked,
          this, &QmitkRestApiPreferencePage::OnAddDirectory);

  connect(m_Ui->m_RemoveDirButton, &QPushButton::clicked,
          this, &QmitkRestApiPreferencePage::OnRemoveDirectory);

  connect(m_Ui->m_HttpsCheckBox, &QCheckBox::toggled,
          this, &QmitkRestApiPreferencePage::OnHttpsToggled);

  connect(m_Ui->m_BrowseCertButton, &QPushButton::clicked,
          this, &QmitkRestApiPreferencePage::OnBrowseCert);

  connect(m_Ui->m_BrowseKeyButton, &QPushButton::clicked,
          this, &QmitkRestApiPreferencePage::OnBrowseKey);

  connect(m_Ui->m_RateLimitCheckBox, &QCheckBox::toggled,
          m_Ui->m_RateLimitSpinBox, &QSpinBox::setEnabled);

  this->Update();
}

QWidget* QmitkRestApiPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkRestApiPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  // Save server settings
  prefs->PutBool("enabled", m_Ui->m_EnabledCheckBox->isChecked());
  prefs->PutBool("autoStart", m_Ui->m_AutoStartCheckBox->isChecked());
  prefs->Put("host", m_Ui->m_HostLineEdit->text().toStdString());
  prefs->PutInt("port", m_Ui->m_PortSpinBox->value());
  prefs->PutInt("threadPoolSize", m_Ui->m_ThreadPoolSizeSpinBox->value());
  prefs->PutInt("readTimeoutSeconds", m_Ui->m_ReadTimeoutSpinBox->value());
  prefs->PutInt("writeTimeoutSeconds", m_Ui->m_WriteTimeoutSpinBox->value());
  prefs->PutBool("logLimitEnabled", m_Ui->m_LogLimitEnabledCheckBox->isChecked());
  prefs->PutInt("logLimit", m_Ui->m_LogLimitSpinBox->value());

  // Save security settings
  prefs->PutInt("clientAccessMode", m_Ui->m_ClientAccessModeComboBox->currentIndex());
  prefs->Put("allowedClientIPs", m_Ui->m_AllowedIPsTextEdit->toPlainText().toStdString());
  prefs->PutBool("requireAuth", m_Ui->m_RequireAuthCheckBox->isChecked());
  prefs->Put("apiToken", m_Ui->m_ApiTokenLineEdit->text().toStdString());
  prefs->PutInt("maxPayloadSizeMB", m_Ui->m_MaxPayloadSpinBox->value());
  prefs->PutInt("fileAccessMode", m_Ui->m_FileAccessModeComboBox->currentIndex());

  // Save allowed directories as semicolon-separated string
  QStringList dirs;
  for (int i = 0; i < m_Ui->m_AllowedDirsListWidget->count(); ++i)
  {
    dirs.append(m_Ui->m_AllowedDirsListWidget->item(i)->text());
  }
  prefs->Put("allowedFileDirectories", dirs.join(";").toStdString());

  // Save rate limiting settings
  prefs->PutBool("rateLimitEnabled", m_Ui->m_RateLimitCheckBox->isChecked());
  prefs->PutInt("rateLimitPerMinute", m_Ui->m_RateLimitSpinBox->value());

  // Save HTTPS settings
  prefs->PutBool("httpsEnabled", m_Ui->m_HttpsCheckBox->isChecked());
  prefs->Put("sslCertPath", m_Ui->m_SslCertLineEdit->text().toStdString());
  prefs->Put("sslKeyPath", m_Ui->m_SslKeyLineEdit->text().toStdString());

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

          // Security settings
          config.clientAccessMode =
            static_cast<mitk::ClientAccessMode>(m_Ui->m_ClientAccessModeComboBox->currentIndex());

          const auto ipText = m_Ui->m_AllowedIPsTextEdit->toPlainText();
          if (!ipText.isEmpty())
          {
            const auto ipLines = ipText.split('\n', Qt::SkipEmptyParts);
            for (const auto& ip : ipLines)
            {
              const auto trimmed = ip.trimmed();
              if (!trimmed.isEmpty())
              {
                config.allowedClientIPs.push_back(trimmed.toStdString());
              }
            }
          }

          config.requireAuth = m_Ui->m_RequireAuthCheckBox->isChecked();
          config.apiToken = m_Ui->m_ApiTokenLineEdit->text().toStdString();
          config.maxPayloadSizeMB = m_Ui->m_MaxPayloadSpinBox->value();

          config.fileAccessMode =
            static_cast<mitk::FileAccessMode>(m_Ui->m_FileAccessModeComboBox->currentIndex());

          for (int i = 0; i < m_Ui->m_AllowedDirsListWidget->count(); ++i)
          {
            config.allowedFileDirectories.push_back(
              m_Ui->m_AllowedDirsListWidget->item(i)->text().toStdString());
          }

          config.rateLimitEnabled = m_Ui->m_RateLimitCheckBox->isChecked();
          config.rateLimitPerMinute = m_Ui->m_RateLimitSpinBox->value();

          config.httpsEnabled = m_Ui->m_HttpsCheckBox->isChecked();
          config.sslCertPath = m_Ui->m_SslCertLineEdit->text().toStdString();
          config.sslKeyPath = m_Ui->m_SslKeyLineEdit->text().toStdString();

          service->SetConfig(config);

          // Apply log limit setting
          if (m_Ui->m_LogLimitEnabledCheckBox->isChecked())
          {
            service->SetLogLimit(static_cast<unsigned int>(m_Ui->m_LogLimitSpinBox->value()));
          }
          else
          {
            service->SetLogLimit(std::nullopt);
          }
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

  // Server settings
  m_Ui->m_EnabledCheckBox->setChecked(prefs->GetBool("enabled", defaults.enabled));
  m_Ui->m_AutoStartCheckBox->setChecked(prefs->GetBool("autoStart", false));
  m_Ui->m_HostLineEdit->setText(QString::fromStdString(prefs->Get("host", defaults.host)));
  m_Ui->m_PortSpinBox->setValue(prefs->GetInt("port", defaults.port));
  m_Ui->m_ThreadPoolSizeSpinBox->setValue(prefs->GetInt("threadPoolSize", defaults.threadPoolSize));
  m_Ui->m_ReadTimeoutSpinBox->setValue(prefs->GetInt("readTimeoutSeconds", defaults.readTimeoutSeconds));
  m_Ui->m_WriteTimeoutSpinBox->setValue(prefs->GetInt("writeTimeoutSeconds", defaults.writeTimeoutSeconds));

  // Log limit settings
  const bool logLimitEnabled = prefs->GetBool("logLimitEnabled", false);
  m_Ui->m_LogLimitEnabledCheckBox->setChecked(logLimitEnabled);
  m_Ui->m_LogLimitSpinBox->setValue(prefs->GetInt("logLimit", 100));
  m_Ui->m_LogLimitSpinBox->setEnabled(logLimitEnabled);

  // Security settings
  const int clientAccessMode = prefs->GetInt("clientAccessMode",
    static_cast<int>(defaults.clientAccessMode));
  m_Ui->m_ClientAccessModeComboBox->setCurrentIndex(clientAccessMode);

  const auto allowedIPs = QString::fromStdString(prefs->Get("allowedClientIPs", ""));
  m_Ui->m_AllowedIPsTextEdit->setPlainText(allowedIPs);

  m_Ui->m_RequireAuthCheckBox->setChecked(prefs->GetBool("requireAuth", defaults.requireAuth));
  m_Ui->m_ApiTokenLineEdit->setText(QString::fromStdString(prefs->Get("apiToken", defaults.apiToken)));

  m_Ui->m_MaxPayloadSpinBox->setValue(prefs->GetInt("maxPayloadSizeMB", defaults.maxPayloadSizeMB));

  // File access settings
  const int fileAccessMode = prefs->GetInt("fileAccessMode",
    static_cast<int>(defaults.fileAccessMode));
  m_Ui->m_FileAccessModeComboBox->setCurrentIndex(fileAccessMode);

  const auto allowedDirsStr = QString::fromStdString(prefs->Get("allowedFileDirectories", ""));
  m_Ui->m_AllowedDirsListWidget->clear();
  if (!allowedDirsStr.isEmpty())
  {
    const auto dirList = allowedDirsStr.split(";", Qt::SkipEmptyParts);
    for (const auto& dir : dirList)
    {
      m_Ui->m_AllowedDirsListWidget->addItem(dir);
    }
  }

  // Rate limiting settings
  const bool rateLimitEnabled = prefs->GetBool("rateLimitEnabled", defaults.rateLimitEnabled);
  m_Ui->m_RateLimitCheckBox->setChecked(rateLimitEnabled);
  m_Ui->m_RateLimitSpinBox->setValue(prefs->GetInt("rateLimitPerMinute", defaults.rateLimitPerMinute));
  m_Ui->m_RateLimitSpinBox->setEnabled(rateLimitEnabled);

  // HTTPS settings
  const bool httpsEnabled = prefs->GetBool("httpsEnabled", defaults.httpsEnabled);
  m_Ui->m_HttpsCheckBox->setChecked(httpsEnabled);
  m_Ui->m_SslCertLineEdit->setText(QString::fromStdString(prefs->Get("sslCertPath", defaults.sslCertPath)));
  m_Ui->m_SslKeyLineEdit->setText(QString::fromStdString(prefs->Get("sslKeyPath", defaults.sslKeyPath)));

  // Set initial visibility based on loaded values
  this->OnClientAccessModeChanged(clientAccessMode);
  this->OnFileAccessModeChanged(fileAccessMode);
  this->OnHttpsToggled(httpsEnabled);
}

void QmitkRestApiPreferencePage::OnClientAccessModeChanged(int index)
{
  // 0 = Localhost Only, 1 = Allow All, 2 = IP Whitelist
  m_Ui->m_AllowedIPsTextEdit->setVisible(index == 2);
  m_Ui->m_AccessModeWarningLabel->setVisible(index == 1);
}

void QmitkRestApiPreferencePage::OnShowTokenToggled()
{
  if (m_Ui->m_ApiTokenLineEdit->echoMode() == QLineEdit::Password)
  {
    m_Ui->m_ApiTokenLineEdit->setEchoMode(QLineEdit::Normal);
    m_Ui->m_ShowTokenButton->setText("Hide");
  }
  else
  {
    m_Ui->m_ApiTokenLineEdit->setEchoMode(QLineEdit::Password);
    m_Ui->m_ShowTokenButton->setText("Show");
  }
}

void QmitkRestApiPreferencePage::OnGenerateToken()
{
  // Use Qt's CSPRNG to generate a 256-bit token (64 hex characters).
  // QRandomGenerator::securelySeeded() uses the platform CSPRNG (BCryptGenRandom
  // on Windows, /dev/urandom on POSIX) and is suitable for security tokens.
  auto rng = QRandomGenerator::securelySeeded();

  std::ostringstream oss;
  // Generate 64 hex chars (4 x 16 hex chars from 4 x uint64_t)
  for (int i = 0; i < 4; ++i)
  {
    const quint64 value = (static_cast<quint64>(rng.generate()) << 32) | rng.generate();
    oss << std::hex << std::setfill('0') << std::setw(16) << value;
  }

  m_Ui->m_ApiTokenLineEdit->setText(QString::fromStdString(oss.str()));
}

void QmitkRestApiPreferencePage::OnCopyToken()
{
  const auto token = m_Ui->m_ApiTokenLineEdit->text();
  if (!token.isEmpty())
  {
    QApplication::clipboard()->setText(token);
  }
}

void QmitkRestApiPreferencePage::OnFileAccessModeChanged(int index)
{
  // 0 = Unrestricted, 1 = Allowed Directories
  const bool showDirList = (index == 1);
  m_Ui->m_AllowedDirsListWidget->setVisible(showDirList);
  m_Ui->m_AddDirButton->setVisible(showDirList);
  m_Ui->m_RemoveDirButton->setVisible(showDirList);
}

void QmitkRestApiPreferencePage::OnAddDirectory()
{
  const auto dir = QFileDialog::getExistingDirectory(m_Control, "Select Allowed Directory");
  if (!dir.isEmpty())
  {
    m_Ui->m_AllowedDirsListWidget->addItem(dir);
  }
}

void QmitkRestApiPreferencePage::OnRemoveDirectory()
{
  auto* currentItem = m_Ui->m_AllowedDirsListWidget->currentItem();
  if (currentItem != nullptr)
  {
    delete m_Ui->m_AllowedDirsListWidget->takeItem(m_Ui->m_AllowedDirsListWidget->row(currentItem));
  }
}

void QmitkRestApiPreferencePage::OnHttpsToggled(bool checked)
{
  m_Ui->m_SslCertLineEdit->setEnabled(checked);
  m_Ui->m_BrowseCertButton->setEnabled(checked);
  m_Ui->m_SslKeyLineEdit->setEnabled(checked);
  m_Ui->m_BrowseKeyButton->setEnabled(checked);
  m_Ui->httpsInfoLabel->setEnabled(checked);
}

void QmitkRestApiPreferencePage::OnBrowseCert()
{
  const auto file = QFileDialog::getOpenFileName(
    m_Control, "Select SSL Certificate", QString(), "PEM Files (*.pem);;All Files (*)");
  if (!file.isEmpty())
  {
    m_Ui->m_SslCertLineEdit->setText(file);
  }
}

void QmitkRestApiPreferencePage::OnBrowseKey()
{
  const auto file = QFileDialog::getOpenFileName(
    m_Control, "Select SSL Private Key", QString(), "PEM Files (*.pem);;All Files (*)");
  if (!file.isEmpty())
  {
    m_Ui->m_SslKeyLineEdit->setText(file);
  }
}
