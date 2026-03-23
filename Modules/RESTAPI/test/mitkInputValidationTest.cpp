/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include "mitkDataStorageController.h"
#include "mitkDataStorageBridge.h"
#include <mitkStandaloneDataStorage.h>

#include <nlohmann/json.hpp>

class mitkInputValidationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkInputValidationTestSuite);
  // UID validation tests
  MITK_TEST(ValidUidAccepted);
  MITK_TEST(ValidUidWithUnderscoresAccepted);
  MITK_TEST(InvalidUidWithSpecialCharsRejected);
  MITK_TEST(EmptyUidRejected);
  MITK_TEST(TooLongUidRejected);

  // Property key validation tests
  MITK_TEST(InvalidPropertyKeyWithControlCharsRejected);
  MITK_TEST(PropertyKeyWithRestapiPrefixRejected);
  MITK_TEST(TooLongPropertyKeyRejected);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;
  std::unique_ptr<mitk::DataStorageController> m_Controller;

  // Helper to create a request with path params
  httplib::Request CreateRequest(const std::string& path = "",
                                  const std::string& body = "",
                                  const std::unordered_map<std::string, std::string>& pathParams = {},
                                  const std::multimap<std::string, std::string>& queryParams = {},
                                  const std::string& contentType = "")
  {
    httplib::Request req;
    req.path = path;
    req.body = body;
    req.path_params = pathParams;
    req.params = queryParams;
    if (!contentType.empty())
    {
      req.set_header("Content-Type", contentType);
    }
    return req;
  }

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Bridge = std::make_unique<mitk::DataStorageBridge>();
    m_Bridge->SetDataStorage(m_DataStorage);
    m_Controller = std::make_unique<mitk::DataStorageController>(*m_Bridge);
  }

  void tearDown() override
  {
    m_Controller.reset();
    m_Bridge->SetDataStorage(nullptr);
    m_Bridge.reset();
    m_DataStorage = nullptr;
  }

  // ===== UID validation tests =====

  // Valid UID (alphanumeric + hyphens + underscores) should not return 400.
  // Since the node does not exist, we expect 404 (not found) instead.
  void ValidUidAccepted()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/abc-123-def", "", {{"uid", "abc-123-def"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    // UID validation passes, so we get 404 (node not found) not 400 (invalid UID)
    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  // UIDs with underscores (e.g., "node_1") are valid — this is the format
  // used by NodeUidMapper::GenerateUid().
  void ValidUidWithUnderscoresAccepted()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/node_42", "", {{"uid", "node_42"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    // UID validation passes, so we get 404 (node not found) not 400 (invalid UID)
    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  // UID with path traversal characters should be rejected with 400.
  // ValidateUid only allows alphanumeric + hyphens + underscores, so slashes and dots are invalid.
  void InvalidUidWithSpecialCharsRejected()
  {
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/abc/../etc/passwd", "", {{"uid", "abc/../etc/passwd"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // Empty UID should be rejected with 400.
  void EmptyUidRejected()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/", "", {{"uid", ""}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // UID longer than 64 characters should be rejected with 400.
  void TooLongUidRejected()
  {
    const std::string longUid(65, 'a');
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + longUid, "", {{"uid", longUid}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // ===== Property key validation tests =====
  // In the controller, property key validation runs AFTER UID validation but
  // BEFORE the node existence check. So we use a valid UID format (that does
  // not exist) and verify 400 is returned for invalid keys, not 404.

  // Property key with control characters (newline) should be rejected with 400.
  void InvalidPropertyKeyWithControlCharsRejected()
  {
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/test-uid/properties/bad\nkey",
      R"({"value": "test"})",
      {{"uid", "test-uid"}, {"property_key", "bad\nkey"}},
      {},
      "application/json");
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_properties_key(req, res);

    // Key validation happens after UID validation but before node lookup,
    // so we expect 400 for invalid key
    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // Property key with "restapi." prefix should be rejected with 400.
  void PropertyKeyWithRestapiPrefixRejected()
  {
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/test-uid/properties/restapi.internal",
      R"({"value": "test"})",
      {{"uid", "test-uid"}, {"property_key", "restapi.internal"}},
      {},
      "application/json");
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // Property key longer than 256 characters should be rejected with 400.
  void TooLongPropertyKeyRejected()
  {
    const std::string longKey(257, 'k');
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/test-uid/properties/" + longKey,
      R"({"value": "test"})",
      {{"uid", "test-uid"}, {"property_key", longKey}},
      {},
      "application/json");
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkInputValidation)
