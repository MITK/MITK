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

#include <QmitkCrashDumpListWidget.h>

#include <QDesktopServices>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <filesystem>
#include <set>
#include <tuple>
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
  m_DumpList = new QmitkCrashDumpListWidget(m_Controls->dumpsGroupBox);
  m_Controls->dumpsLayout->insertWidget(0, m_DumpList);

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
  // Unlike the manager, also show this session's provisional snapshots:
  // watching them appear during a freeze and vanish on recovery is part of
  // what this view is for.
  auto dumps = mitk::CrashDumpFacility::ListAllDumps();
  const auto provisional = mitk::CrashDumpFacility::ListProvisionalSnapshotsOfThisSession();

  std::set<std::filesystem::path> provisionalPaths;
  for (const auto& dump : provisional)
  {
    provisionalPaths.insert(dump.Path);
    dumps.push_back(dump);
  }

  std::sort(dumps.begin(), dumps.end(), [](const auto& lhs, const auto& rhs) {
    return std::tie(rhs.LastWriteTime, rhs.Path) < std::tie(lhs.LastWriteTime, lhs.Path);
  });

  m_DumpList->SetDumps(dumps, provisionalPaths);
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
