/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include "mitkHealthController.h"
#include <mitkDataStorageBridge.h>
#include <mitkRestServerConfig.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkVersion.h>

#include <nlohmann/json.hpp>

class mitkHealthControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkHealthControllerTestSuite);
  MITK_TEST(HealthWithDataStorage);
  MITK_TEST(HealthWithoutDataStorage);
  MITK_TEST(HealthContainsUptimeWhenCallbackSet);
  MITK_TEST(HealthOmitsUptimeWhenCallbackNotSet);
  MITK_TEST(InfoContainsMitkVersion);
  MITK_TEST(InfoContainsCapabilities);
  MITK_TEST(InfoContainsDocumentationUrl);
  MITK_TEST(DocumentationUrlUsesNightlyForDevBuild);
  MITK_TEST(DocumentationUrlUsesVersionForReleaseBuild);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;
  std::unique_ptr<mitk::HealthController> m_Controller;

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Bridge = std::make_unique<mitk::DataStorageBridge>();
    m_Controller = std::make_unique<mitk::HealthController>(*m_Bridge);
  }

  void tearDown() override
  {
    m_Controller.reset();
    m_Bridge.reset();
    m_DataStorage = nullptr;
  }

  void HealthWithDataStorage()
  {
    m_Bridge->SetDataStorage(m_DataStorage);

    httplib::Request req;
    httplib::Response res;

    m_Controller->HandleGET_health(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("healthy"), json["data"]["status"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("ok"), json["data"]["checks"]["datastorage"].get<std::string>());
  }

  void HealthWithoutDataStorage()
  {
    // Don't connect DataStorage

    httplib::Request req;
    httplib::Response res;

    m_Controller->HandleGET_health(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("healthy"), json["data"]["status"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("unavailable"), json["data"]["checks"]["datastorage"].get<std::string>());
  }

  void HealthContainsUptimeWhenCallbackSet()
  {
    // Set uptime callback that returns a fixed value
    m_Controller->SetUptimeCallback([]() -> std::optional<int64_t> {
      return 42;
    });

    httplib::Request req;
    httplib::Response res;

    m_Controller->HandleGET_health(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("uptime_seconds"));
    CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(42), json["data"]["uptime_seconds"].get<int64_t>());
  }

  void HealthOmitsUptimeWhenCallbackNotSet()
  {
    // No uptime callback set

    httplib::Request req;
    httplib::Response res;

    m_Controller->HandleGET_health(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(!json["data"].contains("uptime_seconds"));
  }

  void InfoContainsMitkVersion()
  {
    httplib::Request req;
    httplib::Response res;

    m_Controller->HandleGET_info(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("mitk_version"));
    CPPUNIT_ASSERT_EQUAL(std::string(MITK_VERSION_STRING), json["data"]["mitk_version"].get<std::string>());
  }

  void InfoContainsCapabilities()
  {
    httplib::Request req;
    httplib::Response res;

    m_Controller->HandleGET_info(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("capabilities"));
    CPPUNIT_ASSERT(json["data"]["capabilities"].contains("transfer_modes"));
    CPPUNIT_ASSERT(json["data"]["capabilities"].contains("authentication"));

    // Check transfer modes
    auto transferModes = json["data"]["capabilities"]["transfer_modes"];
    CPPUNIT_ASSERT(transferModes.is_array());
    CPPUNIT_ASSERT(std::find(transferModes.begin(), transferModes.end(), "direct") != transferModes.end());
    CPPUNIT_ASSERT(std::find(transferModes.begin(), transferModes.end(), "file-reference") != transferModes.end());
  }

  void InfoContainsDocumentationUrl()
  {
    httplib::Request req;
    httplib::Response res;

    m_Controller->HandleGET_info(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("documentation_url"));
    // Check the controller emits a well-formed docs URL without recomputing it
    // through GetRestApiDocumentationUrl (the handler's own source), which would
    // make the assertion self-referential. The exact version-to-path mapping is
    // pinned independently by the two dedicated tests below.
    const auto url = json["data"]["documentation_url"].get<std::string>();
    CPPUNIT_ASSERT(url.starts_with("https://docs.mitk.org/"));
    CPPUNIT_ASSERT(url.ends_with("/MITKRESTAPISpec.html"));
  }

  void DocumentationUrlUsesNightlyForDevBuild()
  {
    // patch == 99 is MITK's development-build sentinel and must map to /nightly/,
    // which is the only docs path that resolves for an unreleased version.
    CPPUNIT_ASSERT_EQUAL(
      std::string("https://docs.mitk.org/nightly/MITKRESTAPISpec.html"),
      mitk::GetRestApiDocumentationUrl(2025, 12, 99));
  }

  void DocumentationUrlUsesVersionForReleaseBuild()
  {
    // docs.mitk.org serves release docs under a zero-padded YYYY.MM path, so a
    // single-digit minor (June -> 6) must render as 06 or the URL 404s.
    CPPUNIT_ASSERT_EQUAL(
      std::string("https://docs.mitk.org/2024.06/MITKRESTAPISpec.html"),
      mitk::GetRestApiDocumentationUrl(2024, 6, 0));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkHealthController)
