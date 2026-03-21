/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkErrorResponse.h>
#include <mitkRestServer.h>
#include <mitkRestServerConfig.h>
#include "mitkHealthController.h"
#include <mitkDataStorageBridge.h>
#include <mitkStandaloneDataStorage.h>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include <httplib.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <thread>

namespace
{
  /** Port used for security integration tests. Chosen to be unlikely to conflict. */
  constexpr int kTestPort = 59001;

  /** Wait until the server is accepting connections or the timeout expires. */
  bool WaitForServer(int port, int timeoutMs = 2000)
  {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline)
    {
      httplib::Client probe("127.0.0.1", port);
      probe.set_connection_timeout(0, 50000); // 50 ms
      if (const auto res = probe.Get("/api/v1/health"))
      {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return false;
  }

  /** Build a minimal enabled RestServerConfig for integration tests. */
  mitk::RestServerConfig MakeTestConfig()
  {
    mitk::RestServerConfig cfg;
    cfg.enabled = true;
    cfg.host = "127.0.0.1";
    cfg.port = kTestPort;
    cfg.rateLimitEnabled = false;
    cfg.requireAuth = false;
    cfg.clientAccessMode = mitk::ClientAccessMode::AllowAll;
    return cfg;
  }
}

class mitkSecurityMiddlewareTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSecurityMiddlewareTestSuite);
  MITK_TEST(ErrorResponseAccessDenied);
  MITK_TEST(ErrorResponseUnauthorized);
  MITK_TEST(ErrorResponseRateLimitExceeded);
  MITK_TEST(ErrorResponseFileAccessDenied);
  MITK_TEST(ConfigDefaultsAreSecure);
  MITK_TEST(ConfigSecurityFieldsPersist);
  MITK_TEST(HealthControllerFileAccessConfigUnrestricted);
  MITK_TEST(HealthControllerFileAccessConfigRestricted);
  MITK_TEST(CheckAuthentication_MissingHeader_Returns401);
  MITK_TEST(CheckAuthentication_InvalidToken_Returns401);
  MITK_TEST(CheckAuthentication_ValidToken_AllowsRequest);
  MITK_TEST(CheckAuthentication_ExemptPath_NoTokenRequired);
  MITK_TEST(CheckRateLimit_ExceedsLimit_Returns429);
  MITK_TEST(CheckRateLimit_Disabled_AllowsAll);
  MITK_TEST(CheckClientAccess_AllowAll_Permits);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override {}
  void tearDown() override {}

  void ErrorResponseAccessDenied()
  {
    const auto error = mitk::ErrorResponse::AccessDenied("Client IP 10.0.0.1 is not allowed", "/api/v1/nodes");

    CPPUNIT_ASSERT_EQUAL(403, error["error"]["status"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("ACCESS_DENIED"), error["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("/api/v1/nodes"), error["error"]["instance"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Access Denied"), error["error"]["title"].get<std::string>());
    CPPUNIT_ASSERT(error["error"]["message"].get<std::string>().find("10.0.0.1") != std::string::npos);
  }

  void ErrorResponseUnauthorized()
  {
    const auto error = mitk::ErrorResponse::Unauthorized("Invalid API token", "/api/v1/nodes");

    CPPUNIT_ASSERT_EQUAL(401, error["error"]["status"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("UNAUTHORIZED"), error["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Unauthorized"), error["error"]["title"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("/api/v1/nodes"), error["error"]["instance"].get<std::string>());
  }

  void ErrorResponseRateLimitExceeded()
  {
    const auto error = mitk::ErrorResponse::RateLimitExceeded(30, "/api/v1/nodes");

    CPPUNIT_ASSERT_EQUAL(429, error["error"]["status"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("RATE_LIMIT_EXCEEDED"), error["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(30, error["error"]["retry_after"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("/api/v1/nodes"), error["error"]["instance"].get<std::string>());
  }

  void ErrorResponseFileAccessDenied()
  {
    const auto error =
      mitk::ErrorResponse::FileAccessDenied("Path outside allowed directories", "/api/v1/nodes/123/data");

    CPPUNIT_ASSERT_EQUAL(403, error["error"]["status"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("FILE_ACCESS_DENIED"), error["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("/api/v1/nodes/123/data"), error["error"]["instance"].get<std::string>());
  }

  void ConfigDefaultsAreSecure()
  {
    const mitk::RestServerConfig config;

    CPPUNIT_ASSERT(config.clientAccessMode == mitk::ClientAccessMode::LocalhostOnly);
    CPPUNIT_ASSERT_EQUAL(false, config.requireAuth);
    CPPUNIT_ASSERT_EQUAL(false, config.rateLimitEnabled);
    CPPUNIT_ASSERT_EQUAL(120, config.rateLimitPerMinute);
    CPPUNIT_ASSERT_EQUAL(512, config.maxPayloadSizeMB);
    CPPUNIT_ASSERT(config.fileAccessMode == mitk::FileAccessMode::Unrestricted);
    CPPUNIT_ASSERT_EQUAL(false, config.httpsEnabled);
    CPPUNIT_ASSERT(config.allowedClientIPs.empty());
    CPPUNIT_ASSERT(config.allowedFileDirectories.empty());
    CPPUNIT_ASSERT(config.apiToken.empty());
  }

  void ConfigSecurityFieldsPersist()
  {
    mitk::RestServer server;

    mitk::RestServerConfig config;
    config.clientAccessMode = mitk::ClientAccessMode::Whitelist;
    config.allowedClientIPs = {"192.168.1.1", "10.0.0.1"};
    config.requireAuth = true;
    config.apiToken = "test-token-12345";
    config.rateLimitEnabled = true;
    config.rateLimitPerMinute = 60;
    config.maxPayloadSizeMB = 256;
    config.fileAccessMode = mitk::FileAccessMode::AllowedDirectories;
    config.allowedFileDirectories = {"/tmp/allowed"};
    config.httpsEnabled = false;

    server.SetConfig(config);

    const auto retrieved = server.GetPendingConfig();
    CPPUNIT_ASSERT(retrieved.clientAccessMode == mitk::ClientAccessMode::Whitelist);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), retrieved.allowedClientIPs.size());
    CPPUNIT_ASSERT_EQUAL(std::string("192.168.1.1"), retrieved.allowedClientIPs[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("10.0.0.1"), retrieved.allowedClientIPs[1]);
    CPPUNIT_ASSERT_EQUAL(true, retrieved.requireAuth);
    CPPUNIT_ASSERT_EQUAL(std::string("test-token-12345"), retrieved.apiToken);
    CPPUNIT_ASSERT_EQUAL(true, retrieved.rateLimitEnabled);
    CPPUNIT_ASSERT_EQUAL(60, retrieved.rateLimitPerMinute);
    CPPUNIT_ASSERT_EQUAL(256, retrieved.maxPayloadSizeMB);
    CPPUNIT_ASSERT(retrieved.fileAccessMode == mitk::FileAccessMode::AllowedDirectories);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), retrieved.allowedFileDirectories.size());
    CPPUNIT_ASSERT_EQUAL(std::string("/tmp/allowed"), retrieved.allowedFileDirectories[0]);
  }

  void HealthControllerFileAccessConfigUnrestricted()
  {
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    mitk::DataStorageBridge bridge;
    bridge.SetDataStorage(ds);
    mitk::HealthController controller(bridge);

    httplib::Request req;
    httplib::Response res;
    controller.HandleGET_config_file_access(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("unrestricted"), json["data"]["mode"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(false, json["data"]["restrictions_active"].get<bool>());
    CPPUNIT_ASSERT(!json["data"].contains("allowed_paths"));
    CPPUNIT_ASSERT(json["data"].contains("max_active_temp_dirs_per_ip"));
    CPPUNIT_ASSERT_EQUAL(5, json["data"]["max_active_temp_dirs_per_ip"].get<int>());
  }

  void HealthControllerFileAccessConfigRestricted()
  {
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    mitk::DataStorageBridge bridge;
    bridge.SetDataStorage(ds);
    mitk::HealthController controller(bridge);

    controller.SetFileAccessConfig(mitk::FileAccessMode::AllowedDirectories, {"/data", "/images"});
    controller.SetMaxActiveTempDirsPerIp(3);

    httplib::Request req;
    httplib::Response res;
    controller.HandleGET_config_file_access(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("allowed-directories"), json["data"]["mode"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(true, json["data"]["restrictions_active"].get<bool>());
    CPPUNIT_ASSERT(json["data"].contains("allowed_paths"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"]["allowed_paths"].size());
    CPPUNIT_ASSERT_EQUAL(3, json["data"]["max_active_temp_dirs_per_ip"].get<int>());
  }

  // ==========================================
  // Authentication middleware integration tests
  // ==========================================

  void CheckAuthentication_MissingHeader_Returns401()
  {
    mitk::RestServer server;
    auto cfg = MakeTestConfig();
    cfg.requireAuth = true;
    cfg.apiToken = "super-secret-token";
    server.SetConfig(cfg);

    CPPUNIT_ASSERT_MESSAGE("Server failed to start", server.Start());
    CPPUNIT_ASSERT_MESSAGE("Server did not become ready", WaitForServer(kTestPort));

    httplib::Client client("127.0.0.1", kTestPort);
    const auto res = client.Get("/api/v1/datastorage/nodes");
    server.Stop();

    CPPUNIT_ASSERT_MESSAGE("Expected a response", res != nullptr);
    CPPUNIT_ASSERT_EQUAL(401, res->status);
    const auto body = nlohmann::json::parse(res->body);
    CPPUNIT_ASSERT_EQUAL(std::string("UNAUTHORIZED"), body["error"]["code"].get<std::string>());
  }

  void CheckAuthentication_InvalidToken_Returns401()
  {
    mitk::RestServer server;
    auto cfg = MakeTestConfig();
    cfg.requireAuth = true;
    cfg.apiToken = "super-secret-token";
    server.SetConfig(cfg);

    CPPUNIT_ASSERT_MESSAGE("Server failed to start", server.Start());
    CPPUNIT_ASSERT_MESSAGE("Server did not become ready", WaitForServer(kTestPort));

    httplib::Client client("127.0.0.1", kTestPort);
    httplib::Headers headers = {{"Authorization", "Bearer wrong-token"}};
    const auto res = client.Get("/api/v1/datastorage/nodes", headers);
    server.Stop();

    CPPUNIT_ASSERT_MESSAGE("Expected a response", res != nullptr);
    CPPUNIT_ASSERT_EQUAL(401, res->status);
  }

  void CheckAuthentication_ValidToken_AllowsRequest()
  {
    mitk::RestServer server;
    auto cfg = MakeTestConfig();
    cfg.requireAuth = true;
    cfg.apiToken = "super-secret-token";
    server.SetConfig(cfg);

    CPPUNIT_ASSERT_MESSAGE("Server failed to start", server.Start());
    CPPUNIT_ASSERT_MESSAGE("Server did not become ready", WaitForServer(kTestPort));

    httplib::Client client("127.0.0.1", kTestPort);
    httplib::Headers headers = {{"Authorization", "Bearer super-secret-token"}};
    const auto res = client.Get("/api/v1/datastorage/nodes", headers);
    server.Stop();

    CPPUNIT_ASSERT_MESSAGE("Expected a response", res != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Valid token should not return 401", res->status != 401);
  }

  void CheckAuthentication_ExemptPath_NoTokenRequired()
  {
    mitk::RestServer server;
    auto cfg = MakeTestConfig();
    cfg.requireAuth = true;
    cfg.apiToken = "super-secret-token";
    server.SetConfig(cfg);

    CPPUNIT_ASSERT_MESSAGE("Server failed to start", server.Start());
    CPPUNIT_ASSERT_MESSAGE("Server did not become ready", WaitForServer(kTestPort));

    httplib::Client client("127.0.0.1", kTestPort);
    // /health and /info are exempt from auth
    const auto healthRes = client.Get("/api/v1/health");
    const auto infoRes = client.Get("/api/v1/info");
    server.Stop();

    CPPUNIT_ASSERT_MESSAGE("Expected health response", healthRes != nullptr);
    CPPUNIT_ASSERT_MESSAGE("/health must not require auth", healthRes->status != 401);

    CPPUNIT_ASSERT_MESSAGE("Expected info response", infoRes != nullptr);
    CPPUNIT_ASSERT_MESSAGE("/info must not require auth", infoRes->status != 401);
  }

  // ==========================================
  // Rate limiting middleware integration tests
  // ==========================================

  void CheckRateLimit_ExceedsLimit_Returns429()
  {
    mitk::RestServer server;
    auto cfg = MakeTestConfig();
    cfg.rateLimitEnabled = true;
    cfg.rateLimitPerMinute = 3; // very low limit for testing
    server.SetConfig(cfg);

    CPPUNIT_ASSERT_MESSAGE("Server failed to start", server.Start());
    CPPUNIT_ASSERT_MESSAGE("Server did not become ready", WaitForServer(kTestPort));

    httplib::Client client("127.0.0.1", kTestPort);

    // First 3 requests should succeed (health is exempt, use a rate-limited path)
    bool gotRateLimited = false;
    for (int i = 0; i < 10; ++i)
    {
      const auto res = client.Get("/api/v1/datastorage/nodes");
      if (res != nullptr && res->status == 429)
      {
        gotRateLimited = true;
        CPPUNIT_ASSERT_MESSAGE("Retry-After header must be present", res->has_header("Retry-After"));
        const auto body = nlohmann::json::parse(res->body);
        CPPUNIT_ASSERT_EQUAL(std::string("RATE_LIMIT_EXCEEDED"), body["error"]["code"].get<std::string>());
        break;
      }
    }
    server.Stop();

    CPPUNIT_ASSERT_MESSAGE("Rate limit was never triggered", gotRateLimited);
  }

  void CheckRateLimit_Disabled_AllowsAll()
  {
    mitk::RestServer server;
    auto cfg = MakeTestConfig();
    cfg.rateLimitEnabled = false;
    server.SetConfig(cfg);

    CPPUNIT_ASSERT_MESSAGE("Server failed to start", server.Start());
    CPPUNIT_ASSERT_MESSAGE("Server did not become ready", WaitForServer(kTestPort));

    httplib::Client client("127.0.0.1", kTestPort);

    // Many requests should all succeed when rate limiting is disabled
    for (int i = 0; i < 20; ++i)
    {
      const auto res = client.Get("/api/v1/datastorage/nodes");
      CPPUNIT_ASSERT_MESSAGE("Expected a response", res != nullptr);
      CPPUNIT_ASSERT_MESSAGE("Rate limit must not trigger when disabled", res->status != 429);
    }
    server.Stop();
  }

  // ==========================================
  // Client access middleware integration tests
  // ==========================================

  void CheckClientAccess_AllowAll_Permits()
  {
    mitk::RestServer server;
    auto cfg = MakeTestConfig();
    cfg.clientAccessMode = mitk::ClientAccessMode::AllowAll;
    server.SetConfig(cfg);

    CPPUNIT_ASSERT_MESSAGE("Server failed to start", server.Start());
    CPPUNIT_ASSERT_MESSAGE("Server did not become ready", WaitForServer(kTestPort));

    httplib::Client client("127.0.0.1", kTestPort);
    const auto res = client.Get("/api/v1/health");
    server.Stop();

    CPPUNIT_ASSERT_MESSAGE("Expected a response", res != nullptr);
    CPPUNIT_ASSERT_MESSAGE("AllowAll mode must not block localhost", res->status != 403);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSecurityMiddleware)
