/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkCrashDumpManagerDialog.h>

#include "QmitkCrashDumpUiUtils.h"

#include <QmitkCrashDumpListWidget.h>

#include <algorithm>

#include <mitkICrashReportService.h>
#include <mitkLog.h>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>

namespace
{
  QPointer<QmitkCrashDumpManagerDialog> s_Instance;
}

void QmitkCrashDumpManagerDialog::ShowManager(QWidget* parent)
{
  // A non-modal window outside the active modal dialog would not take
  // input until that dialog closes.
  auto* modal = QApplication::activeModalWidget();

  if (s_Instance.isNull())
  {
    QWidget* owner = modal;
    if (owner == nullptr)
      owner = parent != nullptr ? parent : QApplication::activeWindow();

    s_Instance = new QmitkCrashDumpManagerDialog(owner);
  }
  else
  {
    if (modal != nullptr && s_Instance->parentWidget() != modal)
    {
      const QPointer<QWidget> previousParent = s_Instance->parentWidget();
      s_Instance->setParent(modal, s_Instance->windowFlags());

      // Otherwise the modal dialog would destroy the manager with its
      // children when it is deleted. QDialog::finished is emitted before that.
      if (auto* modalDialog = qobject_cast<QDialog*>(modal))
      {
        QObject::connect(modalDialog, &QDialog::finished, s_Instance.data(),
          [modal, previousParent] {
            if (s_Instance.isNull() || s_Instance->parentWidget() != modal)
              return;

            const bool wasVisible = s_Instance->isVisible();
            s_Instance->setParent(previousParent.data(), s_Instance->windowFlags());

            // setParent hides the widget
            if (wasVisible)
              s_Instance->show();
          },
          Qt::SingleShotConnection);
      }
    }

    s_Instance->Refresh();
  }

  s_Instance->show();
  s_Instance->raise();
  s_Instance->activateWindow();
}

QmitkCrashDumpManagerDialog::QmitkCrashDumpManagerDialog(QWidget* parent)
  : QDialog(parent),
    m_List(new QmitkCrashDumpListWidget),
    m_StateLabel(new QLabel),
    m_FileReportButton(new QPushButton("File report...")),
    m_ShowInFolderButton(new QPushButton("Show in folder")),
    m_ShowLogButton(new QPushButton("Show log")),
    m_CopyPathButton(new QPushButton("Copy path")),
    m_DeleteButton(new QPushButton("Delete")),
    m_DeleteAllButton(new QPushButton("Delete all"))
{
  this->setObjectName("QmitkCrashDumpManagerDialog");
  this->setWindowTitle("Diagnostic Data");
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->setModal(false);

  m_FileReportButton->setObjectName("fileReportButton");
  m_ShowInFolderButton->setObjectName("showInFolderButton");
  m_ShowLogButton->setObjectName("showLogButton");
  m_CopyPathButton->setObjectName("copyPathButton");
  m_DeleteButton->setObjectName("deleteButton");
  m_DeleteAllButton->setObjectName("deleteAllButton");

  auto* header = QmitkCrashDumpUi::CreateHeader("Diagnostic data",
    "Crash dumps and diagnostic snapshots saved on this computer.");

  auto* introLabel = new QLabel(QmitkCrashDumpUi::Paragraph(
    "Crash dumps are written when MITK closes unexpectedly or is terminated while unresponsive; "
    "snapshots are captured on request. Hand them in with a problem report to help us find the "
    "cause.") + QmitkCrashDumpUi::PrivacyNote(false));
  introLabel->setWordWrap(true);

  m_StateLabel->setWordWrap(true);

  auto* refreshButton = new QPushButton("Refresh");
  refreshButton->setObjectName("refreshButton");

  auto* actionLayout = new QHBoxLayout;
  actionLayout->addWidget(m_FileReportButton);
  actionLayout->addWidget(m_ShowInFolderButton);
  actionLayout->addWidget(m_ShowLogButton);
  actionLayout->addWidget(m_CopyPathButton);
  actionLayout->addStretch();
  actionLayout->addWidget(m_DeleteButton);
  actionLayout->addWidget(m_DeleteAllButton);
  actionLayout->addWidget(refreshButton);

  auto* closeButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);

  connect(m_List, &QmitkCrashDumpListWidget::SelectionChanged, this, &QmitkCrashDumpManagerDialog::OnSelectionChanged);
  connect(m_FileReportButton, &QPushButton::clicked, this, &QmitkCrashDumpManagerDialog::OnFileReport);
  connect(m_ShowInFolderButton, &QPushButton::clicked, this, &QmitkCrashDumpManagerDialog::OnShowInFolder);
  connect(m_ShowLogButton, &QPushButton::clicked, this, &QmitkCrashDumpManagerDialog::OnShowLog);
  connect(m_CopyPathButton, &QPushButton::clicked, this, &QmitkCrashDumpManagerDialog::OnCopyPath);
  connect(m_DeleteButton, &QPushButton::clicked, this, &QmitkCrashDumpManagerDialog::OnDelete);
  connect(m_DeleteAllButton, &QPushButton::clicked, this, &QmitkCrashDumpManagerDialog::OnDeleteAll);
  connect(refreshButton, &QPushButton::clicked, this, &QmitkCrashDumpManagerDialog::Refresh);
  connect(closeButtonBox, &QDialogButtonBox::rejected, this, &QDialog::close);

  auto* content = new QVBoxLayout;
  content->setContentsMargins(16, 12, 16, 14);
  content->addWidget(introLabel);
  content->addSpacing(4);
  content->addWidget(m_StateLabel);
  content->addWidget(m_List);
  content->addLayout(actionLayout);
  content->addWidget(closeButtonBox);

  // The header band runs edge to edge; only the content below it is inset.
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(header);
  layout->addLayout(content);

  this->resize(760, 420);
  this->Refresh();
}

void QmitkCrashDumpManagerDialog::Refresh()
{
  // The service may be registered or unregistered while the manager is open.
  m_FileReportButton->setVisible(mitk::IsCrashReportServiceAvailable());

  if (mitk::CrashDumpFacility::GetDatabaseDirectory().empty())
  {
    m_StateLabel->setText(QmitkCrashDumpUi::Warning(
      "Crash dumps are not available: the folder for diagnostic data could not be determined."));
    m_StateLabel->setVisible(true);
  }
  else if (!mitk::CrashDumpFacility::IsActive())
  {
    m_StateLabel->setText(QString("<span style=\"color: %1; font-weight: bold;\">No new crash dumps are being "
      "recorded in this session.</span> Existing ones can still be handled here.")
      .arg(QmitkCrashDumpUi::AccentColor().name()));
    m_StateLabel->setVisible(true);
  }
  else
  {
    m_StateLabel->setVisible(false);
  }

  m_List->SetDumps(mitk::CrashDumpFacility::ListAllDumps());
  m_DeleteAllButton->setEnabled(!m_List->GetDumps().empty());
}

void QmitkCrashDumpManagerDialog::OnSelectionChanged()
{
  const auto selected = m_List->GetSelectedDumps();
  const bool hasSelection = !selected.empty();

  const bool anyLogAvailable = std::any_of(selected.begin(), selected.end(),
    [](const mitk::CrashDumpInfo& dump) { return !dump.SessionLog.empty(); });

  m_FileReportButton->setEnabled(hasSelection);
  m_ShowInFolderButton->setEnabled(hasSelection);
  m_ShowLogButton->setEnabled(anyLogAvailable);
  m_CopyPathButton->setEnabled(hasSelection);
  m_DeleteButton->setEnabled(hasSelection);
}

void QmitkCrashDumpManagerDialog::OnFileReport()
{
  mitk::FileCrashReport(m_List->GetSelectedDumps(), this);
}

void QmitkCrashDumpManagerDialog::OnShowInFolder()
{
  std::vector<std::filesystem::path> files;
  for (const auto& dump : m_List->GetSelectedDumps())
    files.push_back(dump.Path);

  QmitkCrashDumpUi::ShowInFolders(files);
}

void QmitkCrashDumpManagerDialog::OnShowLog()
{
  for (const auto& dump : m_List->GetSelectedDumps())
  {
    std::error_code error;
    if (!dump.SessionLog.empty() && std::filesystem::exists(dump.SessionLog, error))
      QDesktopServices::openUrl(QUrl::fromLocalFile(QmitkCrashDumpUi::ToQString(dump.SessionLog)));
  }
}

void QmitkCrashDumpManagerDialog::OnCopyPath()
{
  QStringList paths;
  for (const auto& dump : m_List->GetSelectedDumps())
    paths << QmitkCrashDumpUi::ToQString(dump.Path);

  QApplication::clipboard()->setText(paths.join('\n'));
}

void QmitkCrashDumpManagerDialog::OnDelete()
{
  const auto selected = m_List->GetSelectedDumps();
  this->DeleteDumps(selected, QString("Delete the %1 selected crash dump(s)? This cannot be undone.").arg(selected.size()));
}

void QmitkCrashDumpManagerDialog::OnDeleteAll()
{
  const auto all = m_List->GetDumps();
  this->DeleteDumps(all, QString("Delete all %1 crash dump(s)? This cannot be undone.").arg(all.size()));
}

void QmitkCrashDumpManagerDialog::DeleteDumps(const std::vector<mitk::CrashDumpInfo>& dumps, const QString& question)
{
  if (dumps.empty())
    return;

  if (QMessageBox::question(this, "Delete Diagnostic Data", question) != QMessageBox::Yes)
    return;

  QStringList failed;

  for (const auto& dump : dumps)
  {
    // The folder is shared with other running instances, which may have
    // removed the dump in the meantime.
    std::error_code error;
    if (!std::filesystem::exists(dump.Path, error))
      continue;

    if (!mitk::CrashDumpFacility::DeleteDump(dump.Path))
    {
      MITK_WARN << "Could not delete crash dump '" << dump.Path.string() << "'.";
      failed << QmitkCrashDumpUi::ToQString(dump.Path);
    }
  }

  this->Refresh();

  if (!failed.isEmpty())
    QMessageBox::warning(this, "Delete Diagnostic Data", "Could not delete:\n" + failed.join('\n'));
}
