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
   * A worker thread watches the heartbeat counter; once the first beat has
   * arrived (see Start()) and the counter then does not advance within
   * Config::Timeout, the monitor treats the thread as stalled and
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

    /** \param config stall timeout, capture pacing and the per-episode and
     *  per-session capture caps.
     *  \param onStall invoked (possibly repeatedly) while the monitored
     *  thread is stalled.
     *  \param onRecovery invoked once when it resumes.
     *
     *  Callbacks run on the monitor's worker thread. */
    HeartbeatMonitor(const Config& config, Callback onStall, Callback onRecovery);

    /** \brief Stops the worker thread (joins). */
    ~HeartbeatMonitor();

    HeartbeatMonitor(const HeartbeatMonitor&) = delete;
    HeartbeatMonitor& operator=(const HeartbeatMonitor&) = delete;

    /** \brief Starts the worker thread. Stall detection does not begin until
     *  the first Beat(): a monitored thread that has never reported in has
     *  not stalled, it has not started yet. Without that rule the monitor
     *  trips during application startup, before the event loop exists to beat
     *  for the first time - at the price of not detecting a genuine hang in
     *  that window, which is acceptable because there is no UI thread to
     *  freeze yet and startup crashes are the crash handler's business. A
     *  monitor that has already beaten stays armed across Stop()/Start(). */
    void Start();

    void Stop();

    /** \brief Record liveness. Called by the monitored thread; lock-free.
     *  The first call also arms stall detection. */
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
