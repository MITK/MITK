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
#include <mitkIOUtil.h>

#include <httplib.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <utility>
#include <vector>

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

/**
 * @brief RAII override of the temp-directory environment so that
 *        std::filesystem::temp_directory_path() resolves to the caller-supplied
 *        path (the test passes the path of a regular file).
 *
 * Forces RestServer::SetupTempDirectory() to fail. SetupTempDirectory() runs
 * before bind_to_port(), so this exercises the pre-bind failure path: Start()
 * returns before any socket is bound.
 * Sets every variable the standard library consults on the respective platform
 * and restores the originals on destruction (or on an explicit Restore()).
 */
class TempEnvOverride
{
public:
  explicit TempEnvOverride(const std::string& tempRootPath)
  {
#ifdef _WIN32
    // GetTempPath2W/GetTempPathW consults TMP, then TEMP (then USERPROFILE);
    // setting the first two non-empty is enough (it returns the first non-empty
    // value verbatim and only falls back to the system root when they are unset).
    m_Vars = {"TMP", "TEMP"};
#else
    // glibc/libc++ consult TMPDIR, TMP, TEMP, TEMPDIR (first non-empty wins).
    m_Vars = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};
#endif
    for (const auto& name : m_Vars)
    {
      const char* old = std::getenv(name.c_str());
      m_Saved.emplace_back(name, old ? std::optional<std::string>(old) : std::nullopt);
      SetEnv(name, tempRootPath);
    }
  }

  ~TempEnvOverride() { this->Restore(); }

  TempEnvOverride(const TempEnvOverride&) = delete;
  TempEnvOverride& operator=(const TempEnvOverride&) = delete;

  void Restore()
  {
    for (const auto& [name, value] : m_Saved)
    {
      if (value.has_value())
        SetEnv(name, *value);
      else
        UnsetEnv(name);
    }
    m_Saved.clear();
  }

private:
  static void SetEnv(const std::string& name, const std::string& value)
  {
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
  }
  static void UnsetEnv(const std::string& name)
  {
#ifdef _WIN32
    _putenv_s(name.c_str(), "");
#else
    unsetenv(name.c_str());
#endif
  }

  std::vector<std::string> m_Vars;
  std::vector<std::pair<std::string, std::optional<std::string>>> m_Saved;
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
  MITK_TEST(FailedStartReleasesPort);
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

  // ===== Port conflict tests =====

  void PortAlreadyInUseReturnsFalse()
  {
    // Occupy port 18100 with a raw socket. SO_EXCLUSIVEADDRUSE (Windows) prevents
    // httplib's SO_REUSEADDR socket from binding to the same address:port.
    PortOccupier occupier(18100);
    CPPUNIT_ASSERT_MESSAGE("PortOccupier must successfully bind port 18100", occupier.IsOccupied());

    mitk::RestServerConfig config;
    // Bind on the same address family as PortOccupier (IPv4 loopback). The
    // default "::" is IPv6 wildcard, which on macOS and Windows defaults to
    // IPV6_V6ONLY=1 and therefore does not collide with an IPv4 listener.
    config.host = "127.0.0.1";
    config.port = 18100;
    config.enabled = true;
    m_Server->SetConfig(config);

    const bool started = m_Server->Start();

    CPPUNIT_ASSERT_MESSAGE("Start() must return false when port is already in use", !started);
    CPPUNIT_ASSERT_MESSAGE("IsRunning() must be false after failed start", !m_Server->IsRunning());
    CPPUNIT_ASSERT_MESSAGE("GetLastError() must be set after failed start", m_Server->GetLastError().has_value());
  }

  void CanRestartOnDifferentPortAfterPortConflict()
  {
    // Occupy port 18101 with a raw socket.
    {
      PortOccupier occupier(18101);
      CPPUNIT_ASSERT_MESSAGE("PortOccupier must successfully bind port 18101", occupier.IsOccupied());

      mitk::RestServerConfig config;
      // Bind on the same address family as PortOccupier (IPv4 loopback); see
      // PortAlreadyInUseReturnsFalse for the IPV6_V6ONLY rationale.
      config.host = "127.0.0.1";
      config.port = 18101;
      config.enabled = true;
      m_Server->SetConfig(config);
      CPPUNIT_ASSERT(!m_Server->Start());
      // occupier goes out of scope here, releasing the port
    }

    // Retry on a now-free port. Must not crash and must succeed.
    mitk::RestServerConfig config;
    config.host = "127.0.0.1";
    config.port = 18102;
    config.enabled = true;
    m_Server->SetConfig(config);

    const bool started = m_Server->Start();

    CPPUNIT_ASSERT_MESSAGE("Start() must succeed on a free port after a previous port conflict", started);
    CPPUNIT_ASSERT(m_Server->IsRunning());

    m_Server->Stop();
    CPPUNIT_ASSERT(!m_Server->IsRunning());
  }

  void FailedStartReleasesPort()
  {
    const int port = 18110;

    // Force Start() to fail during setup by pointing the temp-directory resolver at a
    // regular file: a directory cannot be created beneath a file, so SetupTempDirectory()
    // fails deterministically on every platform. SetupTempDirectory() runs before
    // bind_to_port(), so this is a pre-bind failure: no socket is bound, and the decisive
    // check below confirms the port stays re-bindable. That catches the regression of
    // reordering bind_to_port() ahead of temp-directory setup, which would strand a bound
    // socket (httplib cannot close a socket that was bound but never entered its listen
    // loop, so bind must remain the last fallible step in Start()).
    //
    // The post-bind cleanup branches (listen failure, catch block) are not covered here:
    // reaching them needs bind to succeed and a later step to fail, which a unit test
    // cannot inject without a test-only seam in Start().
    std::ofstream blockerStream;
    const std::string blockerFile =
      mitk::IOUtil::CreateTemporaryFile(blockerStream, "mitk-rest-blocker-XXXXXX");
    blockerStream.close();

    // Remove the blocker file on scope exit so a mid-test assertion failure does not
    // orphan it (it lives in the real temp dir). TempEnvOverride and the stream restore
    // themselves via their own destructors.
    struct BlockerFileGuard
    {
      std::string path;
      ~BlockerFileGuard() { std::remove(path.c_str()); }
    } blockerGuard{blockerFile};

    {
      TempEnvOverride tempGuard(blockerFile);  // temp resolver now points at a file

      mitk::RestServerConfig config;
      config.host = "127.0.0.1";
      config.port = port;
      config.enabled = true;
      m_Server->SetConfig(config);

      const bool started = m_Server->Start();

      CPPUNIT_ASSERT_MESSAGE("Start() must fail when the temp dir cannot be created", !started);
      CPPUNIT_ASSERT_MESSAGE("IsRunning() must be false after a failed start", !m_Server->IsRunning());
      CPPUNIT_ASSERT_MESSAGE("GetLastError() must be set after a failed start", m_Server->GetLastError().has_value());
    }  // temp env restored here (TempEnvOverride dtor), before the port probe

    // Decisive check: the port must be re-bindable. Use an external socket, NOT a second
    // Start() (which would reassign m_Server and could mask a leaked socket). PortOccupier
    // omits SO_REUSEADDR (POSIX) / sets SO_EXCLUSIVEADDRUSE (Windows), so it cannot bind
    // while any listening socket leaked by the failed Start() is still alive.
    PortOccupier occupier(port);
    CPPUNIT_ASSERT_MESSAGE(
      "Port must be re-bindable after a failed start (no leaked listening socket)",
      occupier.IsOccupied());
  }

  // ===== Request logging tests =====

  void LogLimitDefaultsToUnlimited()
  {
    // Default log limit should be nullopt (unlimited)
    CPPUNIT_ASSERT(!m_Server->GetLogLimit().has_value());

    // Request log should be empty initially
    auto log = m_Server->GetRequestLog();
    CPPUNIT_ASSERT(log.empty());
  }

  void LogLimitCanBeSet()
  {
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
    CPPUNIT_ASSERT_EQUAL(uint64_t(0), m_Server->GetRequestLogVersion());
  }

  void LogVersionIncrementsOnClearLog()
  {
    const auto versionBefore = m_Server->GetRequestLogVersion();
    m_Server->ClearRequestLog();
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBefore);
  }

  void LogVersionIncrementsOnStartAndStop()
  {
    mitk::RestServerConfig config;
    config.port = 18090;
    config.enabled = true;
    m_Server->SetConfig(config);

    const auto versionBeforeStart = m_Server->GetRequestLogVersion();
    m_Server->Start();
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBeforeStart);

    const auto versionBeforeStop = m_Server->GetRequestLogVersion();
    m_Server->Stop();
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBeforeStop);
  }

  void LogVersionIncrementsOnRequest()
  {
    mitk::RestServerConfig config;
    config.port = 18091;
    config.enabled = true;
    m_Server->SetConfig(config);

    m_Server->Start();

    const auto versionBefore = m_Server->GetRequestLogVersion();

    httplib::Client client("127.0.0.1", 18091);
    auto result = client.Get("/api/v1/health");
    CPPUNIT_ASSERT(result != nullptr);

    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBefore);

    m_Server->Stop();
  }

  void LogVersionIncrementsOnSetLogLimitTrim()
  {
    mitk::RestServerConfig config;
    config.port = 18092;
    config.enabled = true;
    m_Server->SetConfig(config);

    m_Server->Start();

    // Populate the log with several requests
    httplib::Client client("127.0.0.1", 18092);
    client.Get("/api/v1/health");
    client.Get("/api/v1/health");
    client.Get("/api/v1/health");

    const auto versionBeforeTrim = m_Server->GetRequestLogVersion();

    // Set limit lower than current log size - should trim and increment
    m_Server->SetLogLimit(1);
    CPPUNIT_ASSERT(m_Server->GetRequestLogVersion() > versionBeforeTrim);

    // Set limit higher than current size - no trimming, should NOT increment
    const auto versionAfterTrim = m_Server->GetRequestLogVersion();
    m_Server->SetLogLimit(100);
    CPPUNIT_ASSERT_EQUAL(versionAfterTrim, m_Server->GetRequestLogVersion());

    m_Server->Stop();
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestServer)
