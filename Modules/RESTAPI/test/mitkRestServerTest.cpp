/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include <mitkRestServer.h>
#include <mitkStandaloneDataStorage.h>

#include <thread>
#include <chrono>

class mitkRestServerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestServerTestSuite);
  MITK_TEST(StartsAndStops);
  MITK_TEST(ReportsRunningState);
  MITK_TEST(ReportsServerUrl);
  MITK_TEST(ConfigurationPersists);
  MITK_TEST(PendingVsRunningConfig);
  MITK_TEST(HandlesDataStorageConnection);
  MITK_TEST(DisabledConfigPreventsStart);
  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<mitk::RestServer> m_Server;

public:
  void setUp() override
  {
    m_Server = std::make_unique<mitk::RestServer>();
  }

  void tearDown() override
  {
    if (m_Server)
    {
      m_Server->Stop();
    }
    m_Server.reset();
  }

  void StartsAndStops()
  {
    // Configure to use a test port
    mitk::RestServerConfig config;
    config.port = 18080;  // Use non-standard port for testing
    config.enabled = true;
    m_Server->SetConfig(config);

    CPPUNIT_ASSERT(!m_Server->IsRunning());

    bool started = m_Server->Start();
    CPPUNIT_ASSERT(started);

    // Give the server thread time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CPPUNIT_ASSERT(m_Server->IsRunning());

    m_Server->Stop();

    CPPUNIT_ASSERT(!m_Server->IsRunning());
  }

  void ReportsRunningState()
  {
    CPPUNIT_ASSERT(!m_Server->IsRunning());

    mitk::RestServerConfig config;
    config.port = 18081;
    config.enabled = true;
    m_Server->SetConfig(config);

    m_Server->Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CPPUNIT_ASSERT(m_Server->IsRunning());

    m_Server->Stop();

    CPPUNIT_ASSERT(!m_Server->IsRunning());
  }

  void ReportsServerUrl()
  {
    // Not running - should return nullopt
    CPPUNIT_ASSERT(!m_Server->GetServerUrl().has_value());

    mitk::RestServerConfig config;
    config.host = "127.0.0.1";
    config.port = 18082;
    config.enabled = true;
    m_Server->SetConfig(config);

    m_Server->Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto url = m_Server->GetServerUrl();
    CPPUNIT_ASSERT(url.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("http://127.0.0.1:18082"), url.value());

    m_Server->Stop();
  }

  void ConfigurationPersists()
  {
    mitk::RestServerConfig config;
    config.host = "0.0.0.0";
    config.port = 9999;
    config.enabled = false;
    config.threadPoolSize = 8;
    config.readTimeoutSeconds = 60;
    config.writeTimeoutSeconds = 60;

    m_Server->SetConfig(config);

    auto retrievedConfig = m_Server->GetPendingConfig();
    CPPUNIT_ASSERT_EQUAL(std::string("0.0.0.0"), retrievedConfig.host);
    CPPUNIT_ASSERT_EQUAL(9999, retrievedConfig.port);
    CPPUNIT_ASSERT_EQUAL(false, retrievedConfig.enabled);
    CPPUNIT_ASSERT_EQUAL(8, retrievedConfig.threadPoolSize);
    CPPUNIT_ASSERT_EQUAL(60, retrievedConfig.readTimeoutSeconds);
    CPPUNIT_ASSERT_EQUAL(60, retrievedConfig.writeTimeoutSeconds);
  }

  void PendingVsRunningConfig()
  {
    // Set up initial config
    mitk::RestServerConfig config;
    config.host = "127.0.0.1";
    config.port = 18083;
    config.enabled = true;
    m_Server->SetConfig(config);

    // Before start: pending config exists, running config does not
    auto pending = m_Server->GetPendingConfig();
    CPPUNIT_ASSERT_EQUAL(18083, pending.port);
    CPPUNIT_ASSERT(!m_Server->GetRunningConfig().has_value());

    // Start server
    m_Server->Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // After start: both configs exist and should be the same
    pending = m_Server->GetPendingConfig();
    auto running = m_Server->GetRunningConfig();
    CPPUNIT_ASSERT(running.has_value());
    CPPUNIT_ASSERT_EQUAL(pending.port, running.value().port);

    // Change pending config while running
    mitk::RestServerConfig newConfig;
    newConfig.host = "127.0.0.1";
    newConfig.port = 18084;  // Different port
    newConfig.enabled = true;
    m_Server->SetConfig(newConfig);

    // Pending should have new value, running should have old value
    pending = m_Server->GetPendingConfig();
    running = m_Server->GetRunningConfig();
    CPPUNIT_ASSERT_EQUAL(18084, pending.port);
    CPPUNIT_ASSERT_EQUAL(18083, running.value().port);

    // Stop server
    m_Server->Stop();

    // After stop: running config should be nullopt
    CPPUNIT_ASSERT(!m_Server->GetRunningConfig().has_value());
    // Pending config should still have new value
    pending = m_Server->GetPendingConfig();
    CPPUNIT_ASSERT_EQUAL(18084, pending.port);
  }

  void HandlesDataStorageConnection()
  {
    // Initially no DataStorage
    CPPUNIT_ASSERT(m_Server->GetDataStorage() == nullptr);

    // Connect DataStorage
    auto ds = mitk::StandaloneDataStorage::New();
    m_Server->SetDataStorage(ds.GetPointer());

    CPPUNIT_ASSERT(m_Server->GetDataStorage() != nullptr);
    CPPUNIT_ASSERT(m_Server->GetDataStorage() == ds.GetPointer());

    // Disconnect
    m_Server->SetDataStorage(nullptr);
    CPPUNIT_ASSERT(m_Server->GetDataStorage() == nullptr);
  }

  void DisabledConfigPreventsStart()
  {
    mitk::RestServerConfig config;
    config.enabled = false;
    m_Server->SetConfig(config);

    bool started = m_Server->Start();
    CPPUNIT_ASSERT(!started);
    CPPUNIT_ASSERT(!m_Server->IsRunning());
    CPPUNIT_ASSERT(m_Server->GetLastError().has_value());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestServer)
