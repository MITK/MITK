/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkCrashDumpDialog.h>

#include <mitkCrashDumpFacility.h>
#include <mitkLog.h>

#include <QDateTime>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QLocale>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include <chrono>

namespace
{
  QString FormatDump(const mitk::CrashDumpInfo& dump)
  {
    const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(dump.LastWriteTime);
    const auto dateTime = QDateTime::fromSecsSinceEpoch(
      std::chrono::duration_cast<std::chrono::seconds>(systemTime.time_since_epoch()).count());

    return QString("%1  (%2, %3)")
      .arg(QString::fromStdWString(dump.Path.filename().wstring()))
      .arg(QLocale().toString(dateTime, QLocale::ShortFormat))
      .arg(QLocale().formattedDataSize(static_cast<qint64>(dump.SizeInBytes)));
  }
}

QmitkCrashDumpDialog::QmitkCrashDumpDialog(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget* parent)
  : QDialog(parent)
{
  this->setWindowTitle("Diagnostic Data From Previous Session");

  auto* messageLabel = new QLabel(
    "<b>We are sorry that MITK closed unexpectedly or became unresponsive during your "
    "previous session, and for any inconvenience this may have caused.</b><br/><br/>"
    "A diagnostic snapshot (crash dump) of that session was saved. If possible, please "
    "keep it and hand it in with a problem report. It helps us find the cause and make "
    "MITK more robust and reliable.");
  messageLabel->setWordWrap(true);

  auto* dumpList = new QListWidget;
  dumpList->setSelectionMode(QAbstractItemView::NoSelection);
  dumpList->setFocusPolicy(Qt::NoFocus);

  for (const auto& dump : dumps)
    dumpList->addItem(FormatDump(dump));

  auto* privacyLabel = new QLabel(
    "A crash dump contains parts of the application's memory from that session and "
    "may therefore include patient data, and there is no way to verify that it does "
    "not. MITK never uploads it; it stays on this computer. If you keep it, share it "
    "only through your usual process for handling patient data. "
    "<b>Unless you keep it, it is deleted now.</b>");
  privacyLabel->setWordWrap(true);

  auto* buttonBox = new QDialogButtonBox;
  auto* keepButton = buttonBox->addButton("Keep / Show in Folder", QDialogButtonBox::AcceptRole);
  auto* discardButton = buttonBox->addButton("Discard", QDialogButtonBox::RejectRole);
  discardButton->setDefault(true);
  keepButton->setAutoDefault(false);

  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(messageLabel);
  layout->addWidget(dumpList);
  layout->addWidget(privacyLabel);
  layout->addWidget(buttonBox);

  this->setMinimumWidth(480);
}

void QmitkCrashDumpDialog::ShowIfCrashedLastRun(QWidget* parent)
{
  if (mitk::CrashDumpFacility::GetDatabaseDirectory().empty())
    return; // facility disabled or never initialized

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
  const bool keep = dialog.exec() == QDialog::Accepted;

  // Acknowledge before acting on the choice: surfaced dumps must never
  // surface again, kept or not.
  mitk::CrashDumpFacility::ClearCrashedLastRun();

  if (keep)
  {
    QDesktopServices::openUrl(QUrl::fromLocalFile(
      QString::fromStdWString(dumps.front().Path.parent_path().wstring())));
  }
  else
  {
    for (const auto& dump : dumps)
    {
      if (!mitk::CrashDumpFacility::DeleteDump(dump.Path))
        MITK_WARN << "Could not delete crash dump '" << dump.Path.string() << "'.";
    }
  }
}
