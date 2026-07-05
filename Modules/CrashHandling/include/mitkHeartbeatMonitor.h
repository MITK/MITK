/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkHeartbeatMonitor_h
#define mitkHeartbeatMonitor_h

#include <MitkCrashHandlingExports.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>

namespace mitk
{
  /**
   * \brief Qt-free detector for a wedged (frozen) monitored thread.
   *
   * The monitored thread (typically the UI thread) calls Beat() regularly.
   * A worker thread watches the heartbeat counter; if it does not advance
   * within Config::Timeout, the monitor treats the thread as stalled and
   * invokes the stall callback - up to Config::MaxCapturesPerEpisode times,
   * Config::CaptureInterval apart, so an analyst can tell a permanent wedge
   * from a slow-moving loop afterwards. When the heartbeat resumes, the
   * recovery callback fires once. A per-session episode cap keeps a
   * persistent wedge from churning.
   *
   * The monitor only counts heartbeats and fires callbacks; it holds no Qt
   * and no crash-dump logic, so it is unit-testable with short timeouts.
   */
  class MITKCRASHHANDLING_EXPORT HeartbeatMonitor
  {
  public:
    struct Config
    {
      std::chrono::milliseconds Timeout = std::chrono::minutes(3);
      std::chrono::milliseconds CaptureInterval = std::chrono::seconds(5);
      std::chrono::milliseconds PollInterval = std::chrono::seconds(1);
      int MaxCapturesPerEpisode = 3;
      int MaxEpisodesPerSession = 10;
    };

    using Callback = std::function<void()>;

    /** \param onStall invoked (possibly repeatedly) while the monitored
     *  thread is stalled; \param onRecovery invoked once when it resumes.
     *  Callbacks run on the monitor's worker thread. */
    HeartbeatMonitor(const Config& config, Callback onStall, Callback onRecovery);

    /** \brief Stops the worker thread (joins). */
    ~HeartbeatMonitor();

    HeartbeatMonitor(const HeartbeatMonitor&) = delete;
    HeartbeatMonitor& operator=(const HeartbeatMonitor&) = delete;

    void Start();
    void Stop();

    /** \brief Record liveness. Called by the monitored thread; lock-free. */
    void Beat();

  private:
    void Run();

    Config m_Config;
    Callback m_OnStall;
    Callback m_OnRecovery;

    std::atomic<std::uint64_t> m_Heartbeat{ 0 };

    std::mutex m_Mutex;
    std::condition_variable m_Wake;
    bool m_Stop = false;
    std::thread m_Thread;
  };
}

#endif
