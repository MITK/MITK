/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkCrashDumpDialog.h>

#include "QmitkCrashDumpUiUtils.h"

#include <QmitkCrashDumpListWidget.h>

#include <mitkCrashDumpFacility.h>
#include <mitkICrashReportService.h>
#include <mitkLog.h>
#include <mitkLogBackend.h>

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

QmitkCrashDumpDialog::QmitkCrashDumpDialog(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent)
  : QDialog(parent)
{
  this->setObjectName("QmitkCrashDumpDialog");
  this->setWindowTitle("Diagnostic Data From Previous Session");

  auto* messageLabel = new QLabel(
    "<b>We are sorry that MITK closed unexpectedly or became unresponsive during your "
    "previous session, and for any inconvenience this may have caused.</b><br/><br/>"
    "A diagnostic snapshot (crash dump) of that session was saved. If possible, please "
    "keep it and hand it in with a problem report. It helps us find the cause and make "
    "MITK more robust and reliable.");
  messageLabel->setWordWrap(true);

  auto* dumpList = new QmitkCrashDumpListWidget;
  dumpList->SetSelectionEnabled(false);
  dumpList->SetDumps(dumps);

  auto* privacyLabel = new QLabel(QmitkCrashDumpUi::PrivacyNote() +
    " Kept dumps can be found later under <i>Help &gt; Diagnostic Data...</i>"
    "<br/><br/><b>Unless you keep it, it is deleted now.</b>");
  privacyLabel->setWordWrap(true);

  auto* buttonBox = new QDialogButtonBox;
  auto* removeButton = buttonBox->addButton("Remove", QDialogButtonBox::RejectRole);
  auto* keepButton = buttonBox->addButton("Keep for later", QDialogButtonBox::AcceptRole);
  removeButton->setObjectName("removeButton");
  keepButton->setObjectName("keepButton");
  removeButton->setDefault(true);
  keepButton->setAutoDefault(false);

  connect(keepButton, &QPushButton::clicked, this, [this] { m_Choice = Keep; this->accept(); });
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  if (mitk::GetCrashReportService() != nullptr)
  {
    auto* reportButton = buttonBox->addButton("File report...", QDialogButtonBox::ActionRole);
    reportButton->setObjectName("fileReportButton");
    reportButton->setAutoDefault(false);
    connect(reportButton, &QPushButton::clicked, this, [this] { m_Choice = FileReport; this->accept(); });
  }

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(messageLabel);
  layout->addWidget(dumpList);
  layout->addWidget(privacyLabel);
  layout->addWidget(buttonBox);

  this->setMinimumWidth(560);
}

void QmitkCrashDumpDialog::ShowIfCrashedLastRun(QWidget* parent)
{
  // Before the dialog can block, so that a crash while it is open still
  // records the log. The log is opened by org.mitk.core.services, which has
  // started by the time a window opens.
  mitk::CrashDumpFacility::SetSessionLogFile(mitk::LogBackend::GetLogFile());

  if (mitk::CrashDumpFacility::GetDatabaseDirectory().empty())
    return; // facility never initialized

  const auto dumps = mitk::CrashDumpFacility::ListUnacknowledgedDumps();

  if (dumps.empty())
  {
    if (mitk::CrashDumpFacility::CrashedLastRun())
    {
      MITK_WARN << "The previous run ended in a crash, but no new crash dump was found.";
      mitk::CrashDumpFacility::ClearCrashedLastRun();
    }

    return;
  }

  QmitkCrashDumpDialog dialog(dumps, parent);

  // From inside exec(): showing the dialog before exec() would keep it from
  // becoming modal. The main window of this process is already in front, so
  // where the platform refuses to hand over focus this merely flashes the
  // taskbar.
  QTimer::singleShot(0, &dialog, [&dialog] {
    dialog.raise();
    dialog.activateWindow();
  });
  dialog.exec();

  // Acknowledge before acting on the choice: surfaced dumps must never
  // surface again, whatever happens to them.
  mitk::CrashDumpFacility::ClearCrashedLastRun();

  switch (dialog.m_Choice)
  {
    case Keep:
      break;

    case FileReport:
      // After the dialog has closed: the report flow may outlive this call,
      // the stack-allocated dialog does not.
      QmitkCrashDumpUi::FileReport(dumps, parent);
      break;

    case Remove:
    default:
      for (const auto& dump : dumps)
      {
        if (!mitk::CrashDumpFacility::DeleteDump(dump.Path))
          MITK_WARN << "Could not delete crash dump '" << dump.Path.string() << "'.";
      }
      break;
  }
}
