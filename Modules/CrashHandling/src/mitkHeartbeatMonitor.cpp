/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkHeartbeatMonitor.h"

mitk::HeartbeatMonitor::HeartbeatMonitor(const Config& config, Callback onStall, Callback onRecovery)
  : m_Config(config),
    m_OnStall(std::move(onStall)),
    m_OnRecovery(std::move(onRecovery))
{
}

mitk::HeartbeatMonitor::~HeartbeatMonitor()
{
  this->Stop();
}

void mitk::HeartbeatMonitor::Start()
{
  if (m_Thread.joinable())
    return;

  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Stop = false;
  }

  m_Thread = std::thread(&HeartbeatMonitor::Run, this);
}

void mitk::HeartbeatMonitor::Stop()
{
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Stop = true;
  }
  m_Wake.notify_all();

  if (m_Thread.joinable())
    m_Thread.join();
}

void mitk::HeartbeatMonitor::Beat()
{
  m_Heartbeat.fetch_add(1, std::memory_order_relaxed);
}

void mitk::HeartbeatMonitor::Run()
{
  using clock = std::chrono::steady_clock;

  auto lastSeen = m_Heartbeat.load(std::memory_order_relaxed);
  auto lastChange = clock::now();

  bool stalled = false;
  int capturesThisEpisode = 0;
  int episodes = 0;
  clock::time_point nextCapture;

  std::unique_lock<std::mutex> lock(m_Mutex);
  while (!m_Stop)
  {
    m_Wake.wait_for(lock, m_Config.PollInterval, [this] { return m_Stop; });
    if (m_Stop)
      break;

    // Callbacks may block (a capture can take a moment); do not hold the lock
    // across them. The heartbeat counter is atomic and needs no lock.
    lock.unlock();

    const auto now = clock::now();
    const auto current = m_Heartbeat.load(std::memory_order_relaxed);

    if (current != lastSeen)
    {
      lastSeen = current;
      lastChange = now;

      if (stalled)
      {
        stalled = false;
        capturesThisEpisode = 0;
        if (m_OnRecovery)
          m_OnRecovery();
      }
    }
    else if (!stalled)
    {
      if (now - lastChange >= m_Config.Timeout && episodes < m_Config.MaxEpisodesPerSession)
      {
        stalled = true;
        ++episodes;
        capturesThisEpisode = 1;
        nextCapture = now + m_Config.CaptureInterval;
        if (m_OnStall)
          m_OnStall();
      }
    }
    else if (capturesThisEpisode < m_Config.MaxCapturesPerEpisode && now >= nextCapture)
    {
      ++capturesThisEpisode;
      nextCapture = now + m_Config.CaptureInterval;
      if (m_OnStall)
        m_OnStall();
    }

    lock.lock();
  }
}
