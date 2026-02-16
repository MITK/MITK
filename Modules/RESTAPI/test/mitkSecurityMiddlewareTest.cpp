/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkErrorResponse.h"
#include "mitkRestServer.h"
#include "mitkRestServerConfig.h"
#include "mitkHealthController.h"
#include "mitkDataStorageBridge.h"
#include <mitkStandaloneDataStorage.h>

#include <nlohmann/json.hpp>

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
  }

  void HealthControllerFileAccessConfigRestricted()
  {
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    mitk::DataStorageBridge bridge;
    bridge.SetDataStorage(ds);
    mitk::HealthController controller(bridge);

    controller.SetFileAccessConfig(mitk::FileAccessMode::AllowedDirectories, {"/data", "/images"});

    httplib::Request req;
    httplib::Response res;
    controller.HandleGET_config_file_access(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    const auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("allowed-directories"), json["data"]["mode"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(true, json["data"]["restrictions_active"].get<bool>());
    CPPUNIT_ASSERT(json["data"].contains("allowed_paths"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"]["allowed_paths"].size());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSecurityMiddleware)
