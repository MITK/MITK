/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkRestServer.h>
#include <mitkStandaloneDataStorage.h>

#include <httplib.h>

#include <chrono>
#include <iostream>

// Platform socket headers for PortOccupier
#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#endif

/**
 * @brief RAII helper that binds and listens on a TCP port to block other processes.
 *
 * On Windows, SO_EXCLUSIVEADDRUSE is set so that even sockets with SO_REUSEADDR
 * (as used by httplib internally) cannot bind to the same port.
 * On POSIX, omitting SO_REUSEADDR is sufficient to block re-binding while a
 * socket is actively listening.
 *
 * Winsock is assumed to be already initialised by httplib (via its static
 * WSAStartup helper) before any test runs.
 */
class PortOccupier
{
public:
  explicit PortOccupier(int port)
  {
#ifdef _WIN32
    m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket == INVALID_SOCKET)
      return;

    // Prevent httplib's SO_REUSEADDR socket from binding to the same address:port.
    BOOL excl = TRUE;
    ::setsockopt(m_Socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
                 reinterpret_cast<const char*>(&excl), sizeof(excl));
#else
    m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_Socket < 0)
    {
      m_Socket = kInvalid;
      return;
    }
    // Intentionally do NOT set SO_REUSEADDR: an active listening socket
    // blocks further bind() calls from other sockets on POSIX.
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (::bind(m_Socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0 ||
        ::listen(m_Socket, 1) != 0)
    {
      this->Close();
    }
  }

  ~PortOccupier() { this->Close(); }

  PortOccupier(const PortOccupier&) = delete;
  PortOccupier& operator=(const PortOccupier&) = delete;

  bool IsOccupied() const { return m_Socket != kInvalid; }

  void Close()
  {
    if (m_Socket == kInvalid)
      return;
#ifdef _WIN32
    ::closesocket(m_Socket);
    m_Socket = INVALID_SOCKET;
#else
    ::close(m_Socket);
    m_Socket = kInvalid;
#endif
  }

private:
#ifdef _WIN32
  static constexpr SOCKET kInvalid = INVALID_SOCKET;
  SOCKET m_Socket = INVALID_SOCKET;
#else
  static constexpr int kInvalid = -1;
  int m_Socket = -1;
#endif
};

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
  // Port conflict tests
  MITK_TEST(PortAlreadyInUseReturnsFalse);
  MITK_TEST(CanRestartOnDifferentPortAfterPortConflict);
  // Request logging tests
  MITK_TEST(LogLimitDefaultsToUnlimited);
  MITK_TEST(LogLimitCanBeSet);
  MITK_TEST(ClearRequestLogWorks);
  // Log version tests
  MITK_TEST(LogVersionStartsAtZero);
  MITK_TEST(LogVersionIncrementsOnClearLog);
  MITK_TEST(LogVersionIncrementsOnStartAndStop);
  MITK_TEST(LogVersionIncrementsOnRequest);
  MITK_TEST(LogVersionIncrementsOnSetLogLimitTrim);
  CPPUNIT_TEST_SUITE_END();

private:
  std::unique_ptr<mitk::RestServer> m_Server;
  std::chrono::steady_clock::time_point m_TestStart;
  const char* m_CurrentTest = nullptr;

  void T(const char* label, std::chrono::steady_clock::time_point since)
  {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - since).count();
    std::cerr << "[TIMING] " << (m_CurrentTest ? m_CurrentTest : "?")
              << " " << label << ": " << ms << "ms" << std::endl;
  }

public:
  void setUp() override
  {
    m_TestStart = std::chrono::steady_clock::now();
    auto t0 = m_TestStart;
    m_Server = std::make_unique<mitk::RestServer>();
    T("setUp", t0);
  }

  void tearDown() override
  {
    auto t0 = std::chrono::steady_clock::now();
    if (m_Server)
    {
      m_Server->Stop();
    }
    m_Server.reset();
    T("tearDown", t0);
    T("TOTAL", m_TestStart);
    std::cerr << std::endl;
  }

  void StartsAndStops()
  {
    m_CurrentTest = "StartsAndStops";

    // Configure to use a test port
    mitk::RestServerConfig config;
    config.port = 18080;  // Use non-standard port for testing
    config.enabled = true;
    m_Server->SetConfig(config);

    CPPUNIT_ASSERT(!m_Server->IsRunning());

    auto t0 = std::chrono::steady_clock::now();
    bool started = m_Server->Start();
    T("Start()", t0);
    CPPUNIT_ASSERT(started);
    CPPUNIT_ASSERT(m_Server->IsRunning());

    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);

    CPPUNIT_ASSERT(!m_Server->IsRunning());
  }

  void ReportsRunningState()
  {
    m_CurrentTest = "ReportsRunningState";
    CPPUNIT_ASSERT(!m_Server->IsRunning());

    mitk::RestServerConfig config;
    config.port = 18081;
    config.enabled = true;
    m_Server->SetConfig(config);

    auto t0 = std::chrono::steady_clock::now();
    m_Server->Start();
    T("Start()", t0);
    CPPUNIT_ASSERT(m_Server->IsRunning());

    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);

    CPPUNIT_ASSERT(!m_Server->IsRunning());
  }

  void ReportsServerUrl()
  {
    m_CurrentTest = "ReportsServerUrl";
    // Not running - should return nullopt
    CPPUNIT_ASSERT(!m_Server->GetServerUrl().has_value());

    mitk::RestServerConfig config;
    config.host = "127.0.0.1";
    config.port = 18082;
    config.enabled = true;
    m_Server->SetConfig(config);

    auto t0 = std::chrono::steady_clock::now();
    m_Server->Start();
    T("Start()", t0);

    auto url = m_Server->GetServerUrl();
    CPPUNIT_ASSERT(url.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("http://127.0.0.1:18082"), url.value());

    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);
  }

  void ConfigurationPersists()
  {
    m_CurrentTest = "ConfigurationPersists";
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
    m_CurrentTest = "PendingVsRunningConfig";

    // Set up initial config
    mitk::RestServerConfig config;
    config.host = "127.0.0.1";
    config.port = 18083;
    config.enabled = true;
    m_Server->SetConfig(config);

    auto pending = m_Server->GetPendingConfig();
    CPPUNIT_ASSERT_EQUAL(18083, pending.port);
    CPPUNIT_ASSERT(!m_Server->GetRunningConfig().has_value());

    // Start server
    auto t0 = std::chrono::steady_clock::now();
    m_Server->Start();
    T("Start()", t0);

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
    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);

    // After stop: running config should be nullopt
    CPPUNIT_ASSERT(!m_Server->GetRunningConfig().has_value());
    // Pending config should still have new value
    pending = m_Server->GetPendingConfig();
    CPPUNIT_ASSERT_EQUAL(18084, pending.port);
  }

  void HandlesDataStorageConnection()
  {
    m_CurrentTest = "HandlesDataStorageConnection";
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
    m_CurrentTest = "DisabledConfigPreventsStart";
    mitk::RestServerConfig config;
    config.enabled = false;
    m_Server->SetConfig(config);

    bool started = m_Server->Start();
    CPPUNIT_ASSERT(!started);
    CPPUNIT_ASSERT(!m_Server->IsRunning());
    CPPUNIT_ASSERT(m_Server->GetLastError().has_value());
  }

  // ===== Port conflict tests =====

  void PortAlreadyInUseReturnsFalse()
  {
    m_CurrentTest = "PortAlreadyInUseReturnsFalse";

    // Occupy port 18100 with a raw socket. SO_EXCLUSIVEADDRUSE (Windows) prevents
    // httplib's SO_REUSEADDR socket from binding to the same address:port.
    PortOccupier occupier(18100);
    CPPUNIT_ASSERT_MESSAGE("PortOccupier must successfully bind port 18100", occupier.IsOccupied());

    mitk::RestServerConfig config;
    config.port = 18100;
    config.enabled = true;
    m_Server->SetConfig(config);

    auto t0 = std::chrono::steady_clock::now();
    const bool started = m_Server->Start();
    T("Start() [expect fail]", t0);

    CPPUNIT_ASSERT_MESSAGE("Start() must return false when port is already in use", !started);
    CPPUNIT_ASSERT_MESSAGE("IsRunning() must be false after failed start", !m_Server->IsRunning());
    CPPUNIT_ASSERT_MESSAGE("GetLastError() must be set after failed start", m_Server->GetLastError().has_value());
  }

  void CanRestartOnDifferentPortAfterPortConflict()
  {
    m_CurrentTest = "CanRestartOnDifferentPortAfterPortConflict";

    // Occupy port 18101 with a raw socket.
    {
      PortOccupier occupier(18101);
      CPPUNIT_ASSERT_MESSAGE("PortOccupier must successfully bind port 18101", occupier.IsOccupied());

      mitk::RestServerConfig config;
      config.port = 18101;
      config.enabled = true;
      m_Server->SetConfig(config);

      auto t0 = std::chrono::steady_clock::now();
      CPPUNIT_ASSERT(!m_Server->Start());
      T("Start() [expect fail]", t0);
      // occupier goes out of scope here, releasing the port
    }

    // Retry on a now-free port. Must not crash and must succeed.
    mitk::RestServerConfig config;
    config.port = 18102;
    config.enabled = true;
    m_Server->SetConfig(config);

    auto t0 = std::chrono::steady_clock::now();
    const bool started = m_Server->Start();
    T("Start()", t0);

    CPPUNIT_ASSERT_MESSAGE("Start() must succeed on a free port after a previous port conflict", started);
    CPPUNIT_ASSERT(m_Server->IsRunning());

    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);
    CPPUNIT_ASSERT(!m_Server->IsRunning());
  }

  // ===== Request logging tests =====

  void LogLimitDefaultsToUnlimited()
  {
    m_CurrentTest = "LogLimitDefaultsToUnlimited";
    // Default log limit should be nullopt (unlimited)
    CPPUNIT_ASSERT(!m_Server->GetLogLimit().has_value());

    // Request log should be empty initially
    auto log = m_Server->GetRequestLog();
    CPPUNIT_ASSERT(log.empty());
  }

  void LogLimitCanBeSet()
  {
    m_CurrentTest = "LogLimitCanBeSet";
    // Set a limit
    m_Server->SetLogLimit(100);
    auto limit = m_Server->GetLogLimit();
    CPPUNIT_ASSERT(limit.has_value());
    CPPUNIT_ASSERT_EQUAL(100u, limit.value());

    // Set to unlimited
    m_Server->SetLogLimit(std::nullopt);
    CPPUNIT_ASSERT(!m_Server->GetLogLimit().has_value());

    // Set limit again
    m_Server->SetLogLimit(50);
    limit = m_Server->GetLogLimit();
    CPPUNIT_ASSERT(limit.has_value());
    CPPUNIT_ASSERT_EQUAL(50u, limit.value());
  }

  void ClearRequestLogWorks()
  {
    m_CurrentTest = "ClearRequestLogWorks";
    // The log is managed internally, but we can test the clear interface
    // Note: Actual request recording happens during HTTP requests, which
    // we can't easily simulate in this unit test. We test the clear interface.
    m_Server->ClearRequestLog();
    auto log = m_Server->GetRequestLog();
    CPPUNIT_ASSERT(log.empty());
  }

  // ===== Log version tests =====

  void LogVersionStartsAtZero()
  {
    m_CurrentTest = "LogVersionStartsAtZero";
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), m_Server->GetRequestLogVersion());
  }

  void LogVersionIncrementsOnClearLog()
  {
    m_CurrentTest = "LogVersionIncrementsOnClearLog";
    const auto versionBefore = m_Server->GetRequestLogVersion();
    m_Server->ClearRequestLog();
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBefore);
  }

  void LogVersionIncrementsOnStartAndStop()
  {
    m_CurrentTest = "LogVersionIncrementsOnStartAndStop";

    mitk::RestServerConfig config;
    config.port = 18090;
    config.enabled = true;
    m_Server->SetConfig(config);

    const auto versionBeforeStart = m_Server->GetRequestLogVersion();
    auto t0 = std::chrono::steady_clock::now();
    m_Server->Start();
    T("Start()", t0);
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBeforeStart);

    const auto versionBeforeStop = m_Server->GetRequestLogVersion();
    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBeforeStop);
  }

  void LogVersionIncrementsOnRequest()
  {
    m_CurrentTest = "LogVersionIncrementsOnRequest";

    mitk::RestServerConfig config;
    config.port = 18091;
    config.enabled = true;
    m_Server->SetConfig(config);

    auto t0 = std::chrono::steady_clock::now();
    m_Server->Start();
    T("Start()", t0);

    const auto versionBefore = m_Server->GetRequestLogVersion();

    t0 = std::chrono::steady_clock::now();
    httplib::Client client("127.0.0.1", 18091);
    auto result = client.Get("/api/v1/health");
    T("HTTP GET", t0);
    CPPUNIT_ASSERT(result != nullptr);

    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBefore);

    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);
  }

  void LogVersionIncrementsOnSetLogLimitTrim()
  {
    m_CurrentTest = "LogVersionIncrementsOnSetLogLimitTrim";

    mitk::RestServerConfig config;
    config.port = 18092;
    config.enabled = true;
    m_Server->SetConfig(config);

    auto t0 = std::chrono::steady_clock::now();
    m_Server->Start();
    T("Start()", t0);

    // Populate the log with several requests
    t0 = std::chrono::steady_clock::now();
    httplib::Client client("127.0.0.1", 18092);
    client.Get("/api/v1/health");
    client.Get("/api/v1/health");
    client.Get("/api/v1/health");
    T("HTTP GET x3", t0);

    const auto versionBeforeTrim = m_Server->GetRequestLogVersion();

    // Set limit lower than current log size - should trim and increment
    m_Server->SetLogLimit(1);
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBeforeTrim);

    // Set limit higher than current size - no trimming, should NOT increment
    const auto versionAfterTrim = m_Server->GetRequestLogVersion();
    m_Server->SetLogLimit(100);
    CPPUNIT_ASSERT_EQUAL(versionAfterTrim, m_Server->GetRequestLogVersion());

    t0 = std::chrono::steady_clock::now();
    m_Server->Stop();
    T("Stop()", t0);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestServer)
