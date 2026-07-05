/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkUiFreezeWatchdog.h>

#include <mitkCrashDumpFacility.h>
#include <mitkHeartbeatMonitor.h>
#include <mitkLog.h>

#include <QTimer>

QmitkUiFreezeWatchdog::QmitkUiFreezeWatchdog(std::chrono::seconds timeout, QObject *parent)
  : QObject(parent),
    m_Timer(new QTimer(this))
{
  mitk::HeartbeatMonitor::Config config;
  config.Timeout = timeout;

  // The callbacks run on the monitor's worker thread. CaptureSnapshot and
  // PurgeProvisionalSnapshots are thread-safe, and a snapshot dumps every
  // thread, so the wedged UI thread's stack is captured even though this
  // does not run on it.
  m_Monitor = std::make_unique<mitk::HeartbeatMonitor>(config,
    []
    {
      MITK_WARN << "UI thread appears frozen; capturing a provisional diagnostic snapshot.";
      mitk::CrashDumpFacility::CaptureSnapshot(mitk::SnapshotKind::WatchdogProvisional);
    },
    []
    {
      mitk::CrashDumpFacility::PurgeProvisionalSnapshots();
    });

  connect(m_Timer, &QTimer::timeout, this, [this] { m_Monitor->Beat(); });
  m_Timer->start(1000);

  m_Monitor->Start();
}

QmitkUiFreezeWatchdog::~QmitkUiFreezeWatchdog()
{
  // Stop beating before the monitor is torn down; the unique_ptr's destructor
  // then joins the worker thread.
  m_Timer->stop();
}
