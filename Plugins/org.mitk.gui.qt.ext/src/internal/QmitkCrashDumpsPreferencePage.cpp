/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCrashDumpsPreferencePage.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStringList>
#include <QVBoxLayout>

#ifdef MITK_HAS_CRASHHANDLING
#include <mitkCrashDumpFacility.h>
#include <mitkCrashDumpSessionOptions.h>

#include <QmitkCrashDumpManagerDialog.h>

#include <QCoreApplication>
#endif

namespace
{
#ifdef MITK_HAS_CRASHHANDLING
  /** Offered when the watchdog is switched on while the stored value is 0. */
  constexpr int kSuggestedWatchdogTimeoutSeconds = 60;

  QString OverrideNote()
  {
    const auto options = mitk::GetCrashDumpSessionOptions();
    QStringList notes;

    if (mitk::CrashDumpOptionSource::CommandLine == options.ArmSource)
      notes << "Crash dumps are disabled for this session by the command-line option --MITK.no-crash-dumps.";
    else if (mitk::CrashDumpOptionSource::EnvironmentVariable == options.ArmSource)
      notes << "Crash dumps are disabled for this session by the environment variable MITK_NO_CRASH_DUMPS.";

    const auto watchdog = options.WatchdogTimeoutSeconds > 0
      ? QString("set to %1 s").arg(options.WatchdogTimeoutSeconds)
      : QString("switched off");

    if (mitk::CrashDumpOptionSource::CommandLine == options.WatchdogSource)
      notes << QString("The watchdog is %1 for this session by the command-line option --MITK.ui-watchdog.").arg(watchdog);
    else if (mitk::CrashDumpOptionSource::EnvironmentVariable == options.WatchdogSource)
      notes << QString("The watchdog is %1 for this session by the environment variable MITK_UI_WATCHDOG.").arg(watchdog);

    return notes.join("<br/>");
  }
#endif
}

void QmitkCrashDumpsPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkCrashDumpsPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  auto* layout = new QVBoxLayout(m_Control);

#ifndef MITK_HAS_CRASHHANDLING
  layout->addWidget(new QLabel("Crash dumps are not available in this build."));
  layout->addStretch();
#else
  using Settings = mitk::CrashDumpSettings;

  const auto databaseDirectory = mitk::CrashDumpFacility::GetDatabaseDirectory();
  const bool available = !databaseDirectory.empty();

  auto* introLabel = new QLabel(QString(
    "Changes take effect at the next start. They apply to every installation of %1 on this computer, "
    "because all of them share one folder for diagnostic data.").arg(QCoreApplication::applicationName()));
  introLabel->setWordWrap(true);
  layout->addWidget(introLabel);

  if (!available)
  {
    auto* unavailableLabel = new QLabel(
      "<b>The folder for diagnostic data could not be determined, so these settings cannot be stored.</b>");
    unavailableLabel->setWordWrap(true);
    layout->addWidget(unavailableLabel);
  }

  const auto overrideNote = OverrideNote();
  if (!overrideNote.isEmpty())
  {
    auto* overrideLabel = new QLabel(overrideNote);
    overrideLabel->setWordWrap(true);
    layout->addWidget(overrideLabel);
  }

  auto* form = new QFormLayout;

  m_EnabledCheckBox = new QCheckBox("Record crash dumps");
  form->addRow(m_EnabledCheckBox);

  m_MaxDumpsSpinBox = new QSpinBox;
  m_MaxDumpsSpinBox->setRange(Settings::MinRetention, Settings::MaxRetention);
  form->addRow("Dumps kept per kind:", m_MaxDumpsSpinBox);

  auto* retentionNote = new QLabel(QString(
    "At most %1, because MITK keeps the logs of only the last %1 sessions and a dump is most useful "
    "together with the log of its session.").arg(Settings::MaxRetention));
  retentionNote->setWordWrap(true);
  form->addRow(retentionNote);

  if (mitk::CrashDumpFacility::SupportsSnapshots())
  {
    m_WatchdogCheckBox = new QCheckBox("Watchdog for unresponsive sessions");
    m_WatchdogCheckBox->setToolTip(
      "Takes a diagnostic snapshot when the user interface stops responding. It is deleted again if "
      "the session recovers, and kept only if MITK has to be terminated.");

    m_WatchdogSpinBox = new QSpinBox;
    m_WatchdogSpinBox->setRange(Settings::MinWatchdogTimeoutSeconds, Settings::MaxWatchdogTimeoutSeconds);
    m_WatchdogSpinBox->setSuffix(" s");

    auto* watchdogLayout = new QHBoxLayout;
    watchdogLayout->addWidget(m_WatchdogCheckBox);
    watchdogLayout->addWidget(m_WatchdogSpinBox);
    watchdogLayout->addStretch();
    form->addRow(watchdogLayout);

    connect(m_WatchdogCheckBox, &QCheckBox::toggled, this, [this] { this->UpdateWatchdogEnabledState(); });
  }
  else
  {
    form->addRow(new QLabel("The watchdog for unresponsive sessions is not available on this platform."));
  }

  auto* folderEdit = new QLineEdit(QString::fromStdWString(databaseDirectory.wstring()));
  folderEdit->setReadOnly(true);

  auto* managerButton = new QPushButton("Open Diagnostic Data...");
  connect(managerButton, &QPushButton::clicked, m_Control, [] { QmitkCrashDumpManagerDialog::ShowManager(); });

  auto* folderLayout = new QHBoxLayout;
  folderLayout->addWidget(folderEdit);
  folderLayout->addWidget(managerButton);
  form->addRow("Folder:", folderLayout);

  layout->addLayout(form);
  layout->addStretch();

  m_EnabledCheckBox->setEnabled(available);
  m_MaxDumpsSpinBox->setEnabled(available);
  if (m_WatchdogCheckBox != nullptr)
    m_WatchdogCheckBox->setEnabled(available);
#endif

  this->Update();
}

QWidget* QmitkCrashDumpsPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkCrashDumpsPreferencePage::PerformOk()
{
#ifdef MITK_HAS_CRASHHANDLING
  if (mitk::CrashDumpFacility::GetDatabaseDirectory().empty())
    return true;

  mitk::CrashDumpSettings settings = mitk::CrashDumpFacility::ReadSettings();
  settings.Enabled = m_EnabledCheckBox->isChecked();
  settings.MaxDumpsPerKind = m_MaxDumpsSpinBox->value();

  if (m_WatchdogCheckBox != nullptr)
    settings.WatchdogTimeoutSeconds = m_WatchdogCheckBox->isChecked() ? m_WatchdogSpinBox->value() : 0;

  if (!mitk::CrashDumpFacility::WriteSettings(settings))
  {
    QMessageBox::warning(m_Control, "Crash Dumps",
      "The crash-dump settings could not be saved to\n" +
      QString::fromStdWString(mitk::CrashDumpFacility::GetDatabaseDirectory().wstring()));
    return false;
  }
#endif

  return true;
}

void QmitkCrashDumpsPreferencePage::PerformCancel()
{
}

void QmitkCrashDumpsPreferencePage::Update()
{
#ifdef MITK_HAS_CRASHHANDLING
  const auto settings = mitk::CrashDumpFacility::ReadSettings();

  m_EnabledCheckBox->setChecked(settings.Enabled);
  m_MaxDumpsSpinBox->setValue(settings.MaxDumpsPerKind);

  if (m_WatchdogCheckBox != nullptr)
  {
    m_WatchdogCheckBox->setChecked(settings.WatchdogTimeoutSeconds > 0);
    m_WatchdogSpinBox->setValue(settings.WatchdogTimeoutSeconds > 0
      ? settings.WatchdogTimeoutSeconds
      : kSuggestedWatchdogTimeoutSeconds);
  }

  this->UpdateWatchdogEnabledState();
#endif
}

void QmitkCrashDumpsPreferencePage::UpdateWatchdogEnabledState()
{
  if (m_WatchdogCheckBox != nullptr)
    m_WatchdogSpinBox->setEnabled(m_WatchdogCheckBox->isEnabled() && m_WatchdogCheckBox->isChecked());
}
