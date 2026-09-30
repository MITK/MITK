/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkHeartbeatMonitor.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <atomic>
#include <chrono>
#include <thread>

class mitkHeartbeatMonitorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkHeartbeatMonitorTestSuite);
  MITK_TEST(NoStallWhileBeating);
  MITK_TEST(NoStallBeforeFirstBeat);
  MITK_TEST(StallFiresStallCallback);
  MITK_TEST(RecoveryFiresWhenBeatingResumes);
  MITK_TEST(CapturesAreBoundedPerEpisode);
  CPPUNIT_TEST_SUITE_END();

  using Clock = std::chrono::steady_clock;

  std::atomic<int> m_StallCount{ 0 };
  std::atomic<int> m_RecoveryCount{ 0 };

  mitk::HeartbeatMonitor::Config m_Config;

  /** Polls the condition until it holds. The deadline lies far beyond any
   *  expected wait, so that a broken monitor still fails quickly. */
  template <typename Condition>
  static bool WaitUntil(Condition condition)
  {
    const auto deadline = Clock::now() + std::chrono::seconds(5);

    while (!condition())
    {
      if (Clock::now() >= deadline)
        return false;

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return true;
  }

public:
  void setUp() override
  {
    m_StallCount = 0;
    m_RecoveryCount = 0;

    // Short, test-friendly timings.
    m_Config = mitk::HeartbeatMonitor::Config{};
    m_Config.Timeout = std::chrono::milliseconds(150);
    m_Config.CaptureInterval = std::chrono::milliseconds(80);
    m_Config.PollInterval = std::chrono::milliseconds(20);
    m_Config.MaxCapturesPerEpisode = 3;
  }

  mitk::HeartbeatMonitor MakeMonitor()
  {
    return mitk::HeartbeatMonitor(m_Config,
      [this] { ++m_StallCount; },
      [this] { ++m_RecoveryCount; });
  }

  void NoStallWhileBeating()
  {
    // A loaded CI can starve the beat loop between iterations for far longer
    // than the 150 ms default timeout, which would trip a spurious stall.
    // Give the timeout a wide margin over the beat interval so only a
    // pathological (>1 s) starvation trips it, while still beating past a full
    // timeout window so the no-stall path is genuinely exercised.
    m_Config.Timeout = std::chrono::milliseconds(1000);

    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Bounded by the clock rather than by a number of beats, as a loaded CI
    // stretches every sleep.
    const auto end = Clock::now() + std::chrono::milliseconds(1200);

    while (Clock::now() < end)
    {
      monitor.Beat();
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    monitor.Stop();

    CPPUNIT_ASSERT_EQUAL(0, m_StallCount.load());
  }

  /** A thread that has never reported in has not stalled, it has not started
   *  yet. Without that rule the monitor trips during application startup,
   *  where plugin loading routinely outlasts the timeout before the event
   *  loop exists to beat for the first time. */
  void NoStallBeforeFirstBeat()
  {
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Two timeout windows without a single beat.
    std::this_thread::sleep_for(2 * m_Config.Timeout);
    monitor.Stop();

    CPPUNIT_ASSERT_EQUAL(0, m_StallCount.load());
    CPPUNIT_ASSERT_EQUAL(0, m_RecoveryCount.load());
  }

  void StallFiresStallCallback()
  {
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Report in once to arm stall detection, then go silent until the stall
    // fires.
    monitor.Beat();
    CPPUNIT_ASSERT(WaitUntil([this] { return m_StallCount.load() >= 1; }));
    monitor.Stop();

    CPPUNIT_ASSERT_EQUAL(0, m_RecoveryCount.load());
  }

  void RecoveryFiresWhenBeatingResumes()
  {
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Arm, then stall...
    monitor.Beat();
    CPPUNIT_ASSERT(WaitUntil([this] { return m_StallCount.load() >= 1; }));

    // ...then beat only until the recovery fires. Beating on would leave room
    // for a second stall episode, and its recovery, whenever a loaded CI
    // starves this thread for longer than the timeout between two beats.
    CPPUNIT_ASSERT(WaitUntil([&]
      {
        if (m_RecoveryCount.load() >= 1)
          return true;

        monitor.Beat();
        return false;
      }));
    monitor.Stop();

    CPPUNIT_ASSERT_EQUAL(1, m_RecoveryCount.load());
  }

  void CapturesAreBoundedPerEpisode()
  {
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Arm, then stall until the episode has taken all its captures...
    monitor.Beat();
    CPPUNIT_ASSERT(WaitUntil([this] { return m_StallCount.load() >= m_Config.MaxCapturesPerEpisode; }));

    // ...and on for two capture intervals, in which another one would be due.
    std::this_thread::sleep_for(2 * m_Config.CaptureInterval);
    monitor.Stop();

    CPPUNIT_ASSERT_EQUAL(m_Config.MaxCapturesPerEpisode, m_StallCount.load());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkHeartbeatMonitor)
