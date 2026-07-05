/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCrashTestView.h"
#include <ui_QmitkCrashTestViewControls.h>

#include <mitkExceptionMacro.h>

#ifdef MITK_HAS_CRASHHANDLING
#include <mitkCrashDumpFacility.h>

#include <QDateTime>
#include <QDesktopServices>
#include <QLocale>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QUrl>

#include <algorithm>
#include <filesystem>
#include <set>
#include <tuple>
#include <utility>
#include <vector>
#endif

#include <chrono>
#include <thread>

const std::string QmitkCrashTestView::VIEW_ID = "org.mitk.views.crashtest";

QmitkCrashTestView::QmitkCrashTestView() = default;

QmitkCrashTestView::~QmitkCrashTestView() = default;

void QmitkCrashTestView::CreateQtPartControl(QWidget* parent)
{
  m_Controls = std::make_unique<Ui::QmitkCrashTestViewControls>();
  m_Controls->setupUi(parent);

  connect(m_Controls->crashButton, &QPushButton::clicked,
          this, &QmitkCrashTestView::OnTriggerHardCrash);
  connect(m_Controls->exceptionButton, &QPushButton::clicked,
          this, &QmitkCrashTestView::OnThrowUnhandledException);
  connect(m_Controls->freezeButton, &QPushButton::clicked,
          this, &QmitkCrashTestView::OnFreezeUiThread);

#ifdef MITK_HAS_CRASHHANDLING
  connect(m_Controls->refreshButton, &QPushButton::clicked,
          this, &QmitkCrashTestView::OnRefreshDumpList);
  connect(m_Controls->openFolderButton, &QPushButton::clicked,
          this, &QmitkCrashTestView::OnOpenDumpFolder);

  this->OnRefreshDumpList();
#else
  m_Controls->dumpsGroupBox->setEnabled(false);
  m_Controls->dumpsGroupBox->setTitle(
    m_Controls->dumpsGroupBox->title() + " (crash-dump facility not built)");
#endif
}

void QmitkCrashTestView::SetFocus()
{
  m_Controls->crashButton->setFocus();
}

void QmitkCrashTestView::OnTriggerHardCrash()
{
  int* volatile nullPointer = nullptr;
  *nullPointer = 42;
}

void QmitkCrashTestView::OnThrowUnhandledException()
{
  mitkThrow() << "Deliberate unhandled exception from the Crash Test view";
}

void QmitkCrashTestView::OnFreezeUiThread()
{
  std::this_thread::sleep_for(std::chrono::seconds(m_Controls->freezeDurationSpinBox->value()));

#ifdef MITK_HAS_CRASHHANDLING
  // Show the dumps captured during the freeze right away. The watchdog purges
  // them only once the event loop resumes and the heartbeat catches up, so a
  // delayed second refresh makes the recovery purge observable as well.
  this->OnRefreshDumpList();
  QTimer::singleShot(3000, this, &QmitkCrashTestView::OnRefreshDumpList);
#endif
}

void QmitkCrashTestView::OnRefreshDumpList()
{
#ifdef MITK_HAS_CRASHHANDLING
  std::vector<std::pair<mitk::CrashDumpInfo, QString>> rows;
  std::set<std::filesystem::path> snapshotPaths;

  for (const auto& dump : mitk::CrashDumpFacility::ListSnapshots(mitk::SnapshotKind::OnDemand))
  {
    snapshotPaths.insert(dump.Path);
    rows.emplace_back(dump, QStringLiteral("On-demand snapshot"));
  }

  for (const auto& dump : mitk::CrashDumpFacility::ListSnapshots(mitk::SnapshotKind::WatchdogProvisional))
  {
    snapshotPaths.insert(dump.Path);
    rows.emplace_back(dump, QStringLiteral("Freeze snapshot (provisional)"));
  }

  // ListDumps() also reports provisional freeze snapshots (they are
  // surfacable); those are already listed with their more specific kind.
  for (const auto& dump : mitk::CrashDumpFacility::ListDumps())
  {
    if (snapshotPaths.find(dump.Path) == snapshotPaths.end())
      rows.emplace_back(dump, QStringLiteral("Crash dump"));
  }

  std::sort(rows.begin(), rows.end(), [](const auto& lhs, const auto& rhs) {
    return std::tie(rhs.first.LastWriteTime, rhs.first.Path) <
           std::tie(lhs.first.LastWriteTime, lhs.first.Path);
  });

  m_Controls->dumpTreeWidget->clear();

  for (const auto& [dump, kind] : rows)
  {
    const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(dump.LastWriteTime);
    const auto dateTime = QDateTime::fromSecsSinceEpoch(
      std::chrono::duration_cast<std::chrono::seconds>(systemTime.time_since_epoch()).count());

    auto* item = new QTreeWidgetItem(m_Controls->dumpTreeWidget);
    item->setText(0, kind);
    item->setText(1, QString::fromStdWString(dump.Path.filename().wstring()));
    item->setText(2, QLocale().toString(dateTime, QLocale::ShortFormat));
    item->setText(3, QLocale().formattedDataSize(static_cast<qint64>(dump.SizeInBytes)));
    item->setToolTip(1, QString::fromStdWString(dump.Path.wstring()));
  }

  for (int column = 0; column < m_Controls->dumpTreeWidget->columnCount(); ++column)
    m_Controls->dumpTreeWidget->resizeColumnToContents(column);
#endif
}

void QmitkCrashTestView::OnOpenDumpFolder()
{
#ifdef MITK_HAS_CRASHHANDLING
  const auto directory = mitk::CrashDumpFacility::GetDatabaseDirectory();

  if (!directory.empty())
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdWString(directory.wstring())));
#endif
}
