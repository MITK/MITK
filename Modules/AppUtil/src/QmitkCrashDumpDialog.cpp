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
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace
{
  constexpr int kMinimumWidth = 480;
}

QmitkCrashDumpDialog::QmitkCrashDumpDialog(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent)
  : QDialog(parent)
{
  this->setObjectName("QmitkCrashDumpDialog");
  this->setWindowTitle("Diagnostic Data From Previous Session");

  const bool crashed = std::any_of(dumps.begin(), dumps.end(),
    [](const mitk::CrashDumpInfo& dump) { return mitk::DumpKind::Crash == dump.Kind; });
  const bool terminated = std::any_of(dumps.begin(), dumps.end(),
    [](const mitk::CrashDumpInfo& dump) { return mitk::DumpKind::UnresponsiveTerminated == dump.Kind; });

  QString whatHappened = "MITK closed unexpectedly or stopped responding.";
  if (crashed && !terminated)
    whatHappened = "MITK closed unexpectedly.";
  else if (terminated && !crashed)
    whatHappened = "MITK stopped responding and had to be closed.";

  auto* header = QmitkCrashDumpUi::CreateHeader("Your previous MITK session ended unexpectedly",
    whatHappened + " We are sorry for the interruption and for any work it may have cost you.");

  const bool single = dumps.size() == 1;

  auto* messageLabel = new QLabel(QmitkCrashDumpUi::Paragraph(single
    ? "A crash dump of that session was saved. Attached to a problem report, it helps us find the cause."
    : "Crash dumps of that session were saved. Attached to a problem report, they help us find the cause."));
  messageLabel->setWordWrap(true);

  auto* dumpList = new QmitkCrashDumpListWidget;
  dumpList->SetSelectionEnabled(false);
  dumpList->SetDumps(dumps);
  // Every dump this dialog shows is new.
  dumpList->SetColumnHidden(QmitkCrashDumpListWidget::StatusColumn, true);
  dumpList->FitToContents();

  // Sets the list off from the text around it, on a surface of its own in
  // the dark theme.
  auto* dumpGroupBox = new QGroupBox;
  auto* dumpGroupLayout = new QVBoxLayout(dumpGroupBox);
  dumpGroupLayout->addWidget(dumpList);

  auto* privacyLabel = new QLabel(QmitkCrashDumpUi::PrivacyNote());
  privacyLabel->setWordWrap(true);

  auto* deletionLabel = new QLabel(QmitkCrashDumpUi::Paragraph(QmitkCrashDumpUi::Warning(single
    ? "Unless you keep it, this crash dump is deleted now."
    : "Unless you keep them, these crash dumps are deleted now.")));
  deletionLabel->setObjectName("deletionLabel");
  deletionLabel->setWordWrap(true);

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

  auto* content = new QVBoxLayout;
  content->setContentsMargins(16, 12, 16, 14);
  content->addWidget(messageLabel);
  content->addSpacing(4);
  content->addWidget(dumpGroupBox);
  content->addSpacing(14);
  content->addWidget(privacyLabel);
  content->addSpacing(14);
  content->addWidget(deletionLabel);
  content->addSpacing(12);
  content->addStretch();
  content->addWidget(buttonBox);

  // The header band runs edge to edge; only the content below it is inset.
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(header);
  layout->addLayout(content);

  // The table decides the width and the text wraps to it; the floor keeps
  // the text readable next to a narrow table. Sized via heightForWidth()
  // because the size hint of word-wrapped labels assumes another width than
  // they get, which would leave the dialog with empty space at the bottom.
  // Both measured in the fonts of the style sheet, which only polishing
  // applies before the dialog is shown. Set as the minimum, too, because the
  // one the layout derives takes every word-wrapped label at its narrowest
  // and would add empty space all the same.
  this->ensurePolished();
  const int width = std::max(kMinimumWidth, this->minimumSizeHint().width());
  const int height = this->heightForWidth(width);
  if (height > 0)
  {
    this->setMinimumSize(width, height);
    this->resize(width, height);
  }
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
