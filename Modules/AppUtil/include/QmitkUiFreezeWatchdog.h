/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkUiFreezeWatchdog_h
#define QmitkUiFreezeWatchdog_h

#include <MitkAppUtilExports.h>

#include <QObject>

#include <chrono>
#include <memory>

namespace mitk
{
  class HeartbeatMonitor;
}

class QTimer;

/**
 * \brief UI-thread shell around the crash-handling mitk::HeartbeatMonitor.
 *
 * A QTimer on the UI thread bumps the monitor's heartbeat once a second. If
 * the UI thread wedges, the timer stops firing; after the configured timeout
 * the monitor (on its own worker thread) captures provisional diagnostic
 * snapshots of the whole process - which include the stuck UI thread - and
 * purges them again if the UI recovers. Only a freeze that ends in a hard
 * kill leaves a dump behind. Opt-in; created by mitk::BaseApplication when a
 * timeout is configured.
 */
class MITKAPPUTIL_EXPORT QmitkUiFreezeWatchdog : public QObject
{
  Q_OBJECT

public:
  explicit QmitkUiFreezeWatchdog(std::chrono::seconds timeout, QObject *parent = nullptr);
  ~QmitkUiFreezeWatchdog() override;

private:
  std::unique_ptr<mitk::HeartbeatMonitor> m_Monitor;
  QTimer *m_Timer;
};

#endif
