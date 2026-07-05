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
  MITK_TEST(StallFiresStallCallback);
  MITK_TEST(RecoveryFiresWhenBeatingResumes);
  MITK_TEST(CapturesAreBoundedPerEpisode);
  CPPUNIT_TEST_SUITE_END();

  std::atomic<int> m_StallCount{ 0 };
  std::atomic<int> m_RecoveryCount{ 0 };

  mitk::HeartbeatMonitor::Config m_Config;

public:
  void setUp() override
  {
    m_StallCount = 0;
    m_RecoveryCount = 0;

    // Short, test-friendly timings. Deliberately well above scheduler jitter.
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
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Beat well within the timeout for several timeout windows.
    for (int i = 0; i < 40; ++i)
    {
      monitor.Beat();
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    monitor.Stop();

    CPPUNIT_ASSERT_EQUAL(0, m_StallCount.load());
  }

  void StallFiresStallCallback()
  {
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Never beat; wait past the timeout and let a capture or two happen.
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    monitor.Stop();

    CPPUNIT_ASSERT(m_StallCount.load() >= 1);
    CPPUNIT_ASSERT_EQUAL(0, m_RecoveryCount.load());
  }

  void RecoveryFiresWhenBeatingResumes()
  {
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Stall first...
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    CPPUNIT_ASSERT(m_StallCount.load() >= 1);

    // ...then resume beating and let the recovery fire.
    for (int i = 0; i < 10; ++i)
    {
      monitor.Beat();
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    monitor.Stop();

    CPPUNIT_ASSERT_EQUAL(1, m_RecoveryCount.load());
  }

  void CapturesAreBoundedPerEpisode()
  {
    auto monitor = this->MakeMonitor();
    monitor.Start();

    // Stall for far longer than MaxCapturesPerEpisode * CaptureInterval.
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    monitor.Stop();

    CPPUNIT_ASSERT(m_StallCount.load() >= 1);
    CPPUNIT_ASSERT(m_StallCount.load() <= m_Config.MaxCapturesPerEpisode);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkHeartbeatMonitor)
