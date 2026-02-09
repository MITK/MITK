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
#include "mitkHealthController.h"
#include "mitkDataStorageBridge.h"
#include "mitkErrorResponse.h"
#include <mitkStandaloneDataStorage.h>
#include <mitkStringProperty.h>
#include <mitkProperties.h>

#include <nlohmann/json.hpp>

/**
 * @brief API Conformance Tests - verify the OpenAPI specification matches the implementation.
 *
 * These tests ensure that the REST API implementation conforms to its OpenAPI 3.0 specification.
 * They are designed to catch drift in either direction:
 * - Implementation changes without spec update
 * - Spec changes without implementation update
 *
 * Test categories:
 * 1. Structural Conformance: All endpoints exist
 * 2. Response Schema Conformance: Response structures match spec
 * 3. Query Parameter Conformance: Parameters work as documented
 * 4. Status Code Conformance: Correct HTTP status codes
 * 5. Content-Type Conformance: Correct Content-Type headers
 * 6. Bidirectional Validation: Error codes documented and implemented
 */
class mitkApiConformanceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkApiConformanceTestSuite);

  // Category 1: Structural Conformance
  MITK_TEST(ImplementationEndpointsMatchSpec);

  // Category 2: Response Schema Conformance
  MITK_TEST(SuccessListResponseHasDataEnvelope);
  MITK_TEST(SuccessListResponseHasMetaEnvelope);
  MITK_TEST(NodeObjectHasAllRequiredFields);
  MITK_TEST(NodeFieldTypesCorrect);
  MITK_TEST(ErrorResponseMatchesRFC7807);
  MITK_TEST(ErrorResponseHasAllRequiredFields);
  MITK_TEST(PaginationResponseHasRequiredFields);
  MITK_TEST(HealthResponseStructure);
  MITK_TEST(InfoResponseStructure);

  // Category 3: Query Parameter Conformance
  MITK_TEST(PaginationDefaultLimit);
  MITK_TEST(PaginationMaxLimitEnforced);
  MITK_TEST(PaginationOffsetWorks);
  MITK_TEST(PaginationNoOverlap);
  MITK_TEST(FilterExactMatch);
  MITK_TEST(FilterWildcardPrefix);
  MITK_TEST(FilterWildcardSuffix);
  MITK_TEST(FilterNegation);
  MITK_TEST(HierarchyFilterToplevel);
  MITK_TEST(FieldsSelection);
  MITK_TEST(SortAscending);
  MITK_TEST(SortDescending);
  MITK_TEST(PropertyScopeDefaultForGet);

  // Category 4: Status Code Conformance
  MITK_TEST(ListNodesReturns200);
  MITK_TEST(GetNodeReturns200);
  MITK_TEST(CreateNodeReturns201);
  MITK_TEST(DeleteNodeReturns200WithBody);
  MITK_TEST(MalformedJsonReturns400);
  MITK_TEST(NodeNotFoundReturns404);
  MITK_TEST(PropertyNotFoundReturns404);
  MITK_TEST(NodeHasChildrenReturns409);
  MITK_TEST(UnsupportedContentTypeReturns415);
  MITK_TEST(NoDataStorageReturns503);

  // Category 5: Content-Type Conformance
  MITK_TEST(JsonEndpointsReturnApplicationJson);
  MITK_TEST(HealthEndpointReturnsJson);

  // Category 6: Bidirectional Validation
  MITK_TEST(AllErrorCodesHaveFactoryMethods);
  MITK_TEST(ErrorResponseContainsInstancePath);
  MITK_TEST(CreateNodeResponseHasLocationMeta);
  MITK_TEST(ChildrenEndpointIncludesParentUid);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;
  std::unique_ptr<mitk::DataStorageController> m_Controller;
  std::unique_ptr<mitk::HealthController> m_HealthController;

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

  /// Create a node via the controller and return its UID.
  std::string CreateTestNode(const std::string& name)
  {
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes",
      nlohmann::json{{"name", name}}.dump(),
      {},
      {},
      "application/json");
    httplib::Response res;

    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("CreateTestNode failed for: " + name, 201, res.status);
    auto json = nlohmann::json::parse(res.body);
    return json["data"]["uid"].get<std::string>();
  }

  /// Create a child node under parentUid and return its UID.
  std::string CreateTestChildNode(const std::string& name, const std::string& parentUid)
  {
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + parentUid + "/children",
      nlohmann::json{{"name", name}}.dump(),
      {{"uid", parentUid}},
      {},
      "application/json");
    httplib::Response res;

    m_Controller->HandlePOST_nodes_uid_children(req, res);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("CreateTestChildNode failed for: " + name, 201, res.status);
    auto json = nlohmann::json::parse(res.body);
    return json["data"]["uid"].get<std::string>();
  }

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Bridge = std::make_unique<mitk::DataStorageBridge>();
    m_Bridge->SetDataStorage(m_DataStorage);
    m_Controller = std::make_unique<mitk::DataStorageController>(*m_Bridge);
    m_HealthController = std::make_unique<mitk::HealthController>(*m_Bridge);
  }

  void tearDown() override
  {
    m_HealthController.reset();
    m_Controller.reset();
    m_Bridge->SetDataStorage(nullptr);
    m_Bridge.reset();
    m_DataStorage = nullptr;
  }

  // ==========================================
  // Category 1: Structural Conformance
  // ==========================================

  void ImplementationEndpointsMatchSpec()
  {
    // Per OpenAPI spec, the following 18 endpoint+method combinations exist.
    // This test verifies each handler can be called without crashing and
    // returns a meaningful response (not a generic 500).

    // Discovery endpoints
    {
      httplib::Request req;
      httplib::Response res;
      m_HealthController->HandleGET_health(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /health should respond", res.status == 200);
    }
    {
      httplib::Request req;
      httplib::Response res;
      m_HealthController->HandleGET_info(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /info should respond", res.status == 200);
    }

    // GET /nodes
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes");
      httplib::Response res;
      m_Controller->HandleGET_nodes(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /nodes should respond", res.status == 200);
    }

    // POST /nodes
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes",
        R"({"name":"StructTest"})", {}, {}, "application/json");
      httplib::Response res;
      m_Controller->HandlePOST_nodes(req, res);
      CPPUNIT_ASSERT_MESSAGE("POST /nodes should respond with 201", res.status == 201);
    }

    // GET /nodes/:uid (use a nonexistent UID to verify we get 404 not crash)
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid", "",
        {{"uid", "test-uid"}});
      httplib::Response res;
      m_Controller->HandleGET_nodes_uid(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /nodes/:uid should respond (404 ok)", res.status == 404);
    }

    // PATCH /nodes/:uid
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid",
        R"({"parent_uid": null})", {{"uid", "test-uid"}}, {}, "application/json");
      httplib::Response res;
      m_Controller->HandlePATCH_nodes_uid(req, res);
      CPPUNIT_ASSERT_MESSAGE("PATCH /nodes/:uid should respond (404 ok)", res.status == 404);
    }

    // DELETE /nodes/:uid
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid", "",
        {{"uid", "test-uid"}});
      httplib::Response res;
      m_Controller->HandleDELETE_nodes_uid(req, res);
      CPPUNIT_ASSERT_MESSAGE("DELETE /nodes/:uid should respond (404 ok)", res.status == 404);
    }

    // GET /nodes/:uid/children
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/children", "",
        {{"uid", "test-uid"}});
      httplib::Response res;
      m_Controller->HandleGET_nodes_uid_children(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /nodes/:uid/children should respond (404 ok)", res.status == 404);
    }

    // POST /nodes/:uid/children
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/children",
        R"({"name":"Child"})", {{"uid", "test-uid"}}, {}, "application/json");
      httplib::Response res;
      m_Controller->HandlePOST_nodes_uid_children(req, res);
      CPPUNIT_ASSERT_MESSAGE("POST /nodes/:uid/children should respond (404 ok)", res.status == 404);
    }

    // GET /nodes/:uid/data
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/data", "",
        {{"uid", "test-uid"}});
      httplib::Response res;
      m_Controller->HandleGET_nodes_uid_data(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /nodes/:uid/data should respond (404 ok)", res.status == 404);
    }

    // PUT /nodes/:uid/data
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/data",
        "binary", {{"uid", "test-uid"}}, {}, "application/octet-stream");
      httplib::Response res;
      m_Controller->HandlePUT_nodes_uid_data(req, res);
      CPPUNIT_ASSERT_MESSAGE("PUT /nodes/:uid/data should respond (404 ok)", res.status == 404);
    }

    // GET /nodes/:uid/properties
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/properties", "",
        {{"uid", "test-uid"}});
      httplib::Response res;
      m_Controller->HandleGET_nodes_uid_properties(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /nodes/:uid/properties should respond (404 ok)", res.status == 404);
    }

    // GET /nodes/:uid/properties/:key
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/properties/name", "",
        {{"uid", "test-uid"}, {"key", "name"}});
      httplib::Response res;
      m_Controller->HandleGET_nodes_uid_properties_key(req, res);
      CPPUNIT_ASSERT_MESSAGE("GET /nodes/:uid/properties/:key should respond (404 ok)", res.status == 404);
    }

    // PUT /nodes/:uid/properties/:key
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/properties/visible",
        "true", {{"uid", "test-uid"}, {"key", "visible"}}, {}, "application/json");
      httplib::Response res;
      m_Controller->HandlePUT_nodes_uid_properties_key(req, res);
      CPPUNIT_ASSERT_MESSAGE("PUT /nodes/:uid/properties/:key should respond (404 ok)", res.status == 404);
    }

    // DELETE /nodes/:uid/properties/:key
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/properties/custom", "",
        {{"uid", "test-uid"}, {"key", "custom"}});
      httplib::Response res;
      m_Controller->HandleDELETE_nodes_uid_properties_key(req, res);
      CPPUNIT_ASSERT_MESSAGE("DELETE /nodes/:uid/properties/:key should respond (404 ok)", res.status == 404);
    }

    // PUT /nodes/:uid/properties
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/properties",
        R"({"visible": true})", {{"uid", "test-uid"}}, {}, "application/json");
      httplib::Response res;
      m_Controller->HandlePUT_nodes_uid_properties(req, res);
      CPPUNIT_ASSERT_MESSAGE("PUT /nodes/:uid/properties should respond (404 ok)", res.status == 404);
    }

    // PATCH /nodes/:uid/properties
    {
      auto req = this->CreateRequest("/api/v1/datastorage/nodes/test-uid/properties",
        R"({"visible": true})", {{"uid", "test-uid"}}, {}, "application/json");
      httplib::Response res;
      m_Controller->HandlePATCH_nodes_uid_properties(req, res);
      CPPUNIT_ASSERT_MESSAGE("PATCH /nodes/:uid/properties should respond (404 ok)", res.status == 404);
    }
  }

  // ==========================================
  // Category 2: Response Schema Conformance
  // ==========================================

  void SuccessListResponseHasDataEnvelope()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    CPPUNIT_ASSERT_MESSAGE("List response must have 'data' key", json.contains("data"));
    CPPUNIT_ASSERT_MESSAGE("'data' must be an array", json["data"].is_array());
  }

  void SuccessListResponseHasMetaEnvelope()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);

    CPPUNIT_ASSERT_MESSAGE("List response must have 'meta' key", json.contains("meta"));
    CPPUNIT_ASSERT_MESSAGE("'meta' must be an object", json["meta"].is_object());
  }

  void NodeObjectHasAllRequiredFields()
  {
    const std::string uid = this->CreateTestNode("ConformanceNode");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid, "",
      {{"uid", uid}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    const auto& nodeData = json["data"];

    // Per OpenAPI spec: Node schema has these required fields
    CPPUNIT_ASSERT_MESSAGE("Node must have 'uid'", nodeData.contains("uid"));
    CPPUNIT_ASSERT_MESSAGE("Node must have 'name'", nodeData.contains("name"));
    CPPUNIT_ASSERT_MESSAGE("Node must have 'path'", nodeData.contains("path"));
    CPPUNIT_ASSERT_MESSAGE("Node must have 'parent_uid'", nodeData.contains("parent_uid"));
    CPPUNIT_ASSERT_MESSAGE("Node must have 'data_type'", nodeData.contains("data_type"));
    CPPUNIT_ASSERT_MESSAGE("Node must have 'children_count'", nodeData.contains("children_count"));
    CPPUNIT_ASSERT_MESSAGE("Node must have 'timestamp'", nodeData.contains("timestamp"));
  }

  void NodeFieldTypesCorrect()
  {
    const std::string uid = this->CreateTestNode("TypeCheckNode");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid, "",
      {{"uid", uid}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    auto json = nlohmann::json::parse(res.body);
    const auto& nodeData = json["data"];

    // Per OpenAPI spec: type constraints
    CPPUNIT_ASSERT_MESSAGE("uid must be string", nodeData["uid"].is_string());
    CPPUNIT_ASSERT_MESSAGE("name must be string", nodeData["name"].is_string());
    CPPUNIT_ASSERT_MESSAGE("path must be string", nodeData["path"].is_string());
    CPPUNIT_ASSERT_MESSAGE("parent_uid must be string or null",
      nodeData["parent_uid"].is_string() || nodeData["parent_uid"].is_null());
    CPPUNIT_ASSERT_MESSAGE("data_type must be string or null",
      nodeData["data_type"].is_string() || nodeData["data_type"].is_null());
    CPPUNIT_ASSERT_MESSAGE("children_count must be integer", nodeData["children_count"].is_number_integer());
    CPPUNIT_ASSERT_MESSAGE("timestamp must be integer", nodeData["timestamp"].is_number_integer());

    // Value checks
    CPPUNIT_ASSERT_EQUAL(std::string("TypeCheckNode"), nodeData["name"].get<std::string>());
    CPPUNIT_ASSERT_MESSAGE("Top-level node has null parent_uid", nodeData["parent_uid"].is_null());
    CPPUNIT_ASSERT_MESSAGE("Node without data has null data_type", nodeData["data_type"].is_null());
    CPPUNIT_ASSERT_MESSAGE("children_count >= 0", nodeData["children_count"].get<int>() >= 0);
  }

  void ErrorResponseMatchesRFC7807()
  {
    // Trigger 404 error
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent", "",
      {{"uid", "nonexistent"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Per OpenAPI spec: Error schema wraps in "error" object
    CPPUNIT_ASSERT_MESSAGE("Error response must have 'error' key", json.contains("error"));

    const auto& error = json["error"];

    // Per RFC 7807 / OpenAPI spec: required fields
    CPPUNIT_ASSERT_MESSAGE("error must have 'type'", error.contains("type"));
    CPPUNIT_ASSERT_MESSAGE("error must have 'code'", error.contains("code"));
    CPPUNIT_ASSERT_MESSAGE("error must have 'title'", error.contains("title"));
    CPPUNIT_ASSERT_MESSAGE("error must have 'message'", error.contains("message"));
    CPPUNIT_ASSERT_MESSAGE("error must have 'status'", error.contains("status"));
  }

  void ErrorResponseHasAllRequiredFields()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent", "",
      {{"uid", "nonexistent"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    auto json = nlohmann::json::parse(res.body);
    const auto& error = json["error"];

    // Type checks per OpenAPI spec
    CPPUNIT_ASSERT_MESSAGE("type must be string (URI)", error["type"].is_string());
    CPPUNIT_ASSERT_MESSAGE("code must be string", error["code"].is_string());
    CPPUNIT_ASSERT_MESSAGE("title must be string", error["title"].is_string());
    CPPUNIT_ASSERT_MESSAGE("message must be string", error["message"].is_string());
    CPPUNIT_ASSERT_MESSAGE("status must be integer", error["status"].is_number_integer());

    // Value checks
    CPPUNIT_ASSERT_EQUAL(404, error["status"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), error["code"].get<std::string>());

    // Type URI should follow pattern
    const std::string typeUri = error["type"].get<std::string>();
    CPPUNIT_ASSERT_MESSAGE("type URI must contain error code",
      typeUri.find("NODE_NOT_FOUND") != std::string::npos);
  }

  void PaginationResponseHasRequiredFields()
  {
    // Create enough nodes to have pagination metadata
    for (int i = 0; i < 5; ++i)
    {
      this->CreateTestNode("PagNode" + std::to_string(i));
    }

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"limit", "2"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    const auto& meta = json["meta"];

    // Per OpenAPI spec: ListMeta schema
    CPPUNIT_ASSERT_MESSAGE("meta must have total_count", meta.contains("total_count"));
    CPPUNIT_ASSERT_MESSAGE("meta must have limit", meta.contains("limit"));
    CPPUNIT_ASSERT_MESSAGE("meta must have offset", meta.contains("offset"));
    CPPUNIT_ASSERT_MESSAGE("meta must have returned_count", meta.contains("returned_count"));

    // When there are more pages, links should have "next"
    CPPUNIT_ASSERT_MESSAGE("meta must have links when paginated", meta.contains("links"));
    CPPUNIT_ASSERT_MESSAGE("links must have 'next' when more pages",
      meta["links"].contains("next"));
  }

  void HealthResponseStructure()
  {
    httplib::Request req;
    httplib::Response res;
    m_HealthController->HandleGET_health(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Per OpenAPI spec: HealthStatus schema
    CPPUNIT_ASSERT_MESSAGE("Health must have data.status", json["data"].contains("status"));
    CPPUNIT_ASSERT_MESSAGE("Health must have data.checks", json["data"].contains("checks"));
    CPPUNIT_ASSERT_MESSAGE("checks must have datastorage", json["data"]["checks"].contains("datastorage"));

    CPPUNIT_ASSERT_EQUAL(std::string("healthy"), json["data"]["status"].get<std::string>());
  }

  void InfoResponseStructure()
  {
    httplib::Request req;
    httplib::Response res;
    m_HealthController->HandleGET_info(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Per OpenAPI spec: ApiInfo schema
    CPPUNIT_ASSERT_MESSAGE("Info must have data.name", json["data"].contains("name"));
    CPPUNIT_ASSERT_MESSAGE("Info must have data.version", json["data"].contains("version"));
    CPPUNIT_ASSERT_MESSAGE("Info must have data.api_version", json["data"].contains("api_version"));
    CPPUNIT_ASSERT_MESSAGE("Info must have data.mitk_version", json["data"].contains("mitk_version"));
    CPPUNIT_ASSERT_MESSAGE("Info must have data.documentation_url", json["data"].contains("documentation_url"));
    CPPUNIT_ASSERT_MESSAGE("Info must have data.capabilities", json["data"].contains("capabilities"));
    CPPUNIT_ASSERT_MESSAGE("capabilities must have transfer_modes",
      json["data"]["capabilities"].contains("transfer_modes"));
  }

  // ==========================================
  // Category 3: Query Parameter Conformance
  // ==========================================

  void PaginationDefaultLimit()
  {
    // Per OpenAPI spec: default limit is 50
    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(50, json["meta"]["limit"].get<int>());
    CPPUNIT_ASSERT_EQUAL(0, json["meta"]["offset"].get<int>());
  }

  void PaginationMaxLimitEnforced()
  {
    // Per OpenAPI spec: maximum limit is 1000
    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"limit", "5000"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_MESSAGE("limit must be capped at 1000",
      json["meta"]["limit"].get<int>() <= 1000);
  }

  void PaginationOffsetWorks()
  {
    for (int i = 0; i < 5; ++i)
    {
      this->CreateTestNode("OffNode" + std::to_string(i));
    }

    // Get with offset
    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"limit", "2"}, {"offset", "2"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(2, json["meta"]["offset"].get<int>());
    CPPUNIT_ASSERT_EQUAL(2, json["meta"]["returned_count"].get<int>());
    CPPUNIT_ASSERT_EQUAL(5, json["meta"]["total_count"].get<int>());
  }

  void PaginationNoOverlap()
  {
    for (int i = 0; i < 6; ++i)
    {
      this->CreateTestNode("OvlNode" + std::to_string(i));
    }

    // Page 1
    auto req1 = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"limit", "3"}, {"offset", "0"}});
    httplib::Response res1;
    m_Controller->HandleGET_nodes(req1, res1);
    auto page1 = nlohmann::json::parse(res1.body);

    // Page 2
    auto req2 = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"limit", "3"}, {"offset", "3"}});
    httplib::Response res2;
    m_Controller->HandleGET_nodes(req2, res2);
    auto page2 = nlohmann::json::parse(res2.body);

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), page1["data"].size());
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), page2["data"].size());

    // Verify no overlap in UIDs
    for (const auto& n1 : page1["data"])
    {
      for (const auto& n2 : page2["data"])
      {
        CPPUNIT_ASSERT_MESSAGE("Pages must not overlap",
          n1["uid"].get<std::string>() != n2["uid"].get<std::string>());
      }
    }
  }

  void FilterExactMatch()
  {
    this->CreateTestNode("FilterExact");
    this->CreateTestNode("FilterOther");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"filter.name", "FilterExact"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), json["data"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("FilterExact"),
      json["data"][0]["name"].get<std::string>());
  }

  void FilterWildcardPrefix()
  {
    this->CreateTestNode("CT_Scan_001");
    this->CreateTestNode("CT_Scan_002");
    this->CreateTestNode("MRI_Scan_001");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"filter.name", "CT*"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"].size());
  }

  void FilterWildcardSuffix()
  {
    this->CreateTestNode("Scan_A");
    this->CreateTestNode("Scan_B");
    this->CreateTestNode("Image_A");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"filter.name", "*_A"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"].size());
  }

  void FilterNegation()
  {
    this->CreateTestNode("KeepMe");
    this->CreateTestNode("FilterOut");

    // Negation filter: filter.name!=FilterOut (URL-encoded as filter.name!=FilterOut)
    // httplib may parse the ! as part of the key or value
    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"filter.name!", "FilterOut"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    // Should only return KeepMe
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), json["data"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("KeepMe"),
      json["data"][0]["name"].get<std::string>());
  }

  void HierarchyFilterToplevel()
  {
    const std::string parentUid = this->CreateTestNode("TopLevel");
    this->CreateTestChildNode("ChildLevel", parentUid);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"hierarchy", "toplevel"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    // Only top-level node should be returned
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), json["data"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("TopLevel"),
      json["data"][0]["name"].get<std::string>());
  }

  void FieldsSelection()
  {
    this->CreateTestNode("FieldsTestNode");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"fields", "uid,name"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(!json["data"].empty());

    const auto& node = json["data"][0];
    // Only requested fields should be present
    CPPUNIT_ASSERT_MESSAGE("uid field must be present", node.contains("uid"));
    CPPUNIT_ASSERT_MESSAGE("name field must be present", node.contains("name"));
    CPPUNIT_ASSERT_MESSAGE("path field must not be present", !node.contains("path"));
    CPPUNIT_ASSERT_MESSAGE("data_type field must not be present", !node.contains("data_type"));
  }

  void SortAscending()
  {
    this->CreateTestNode("ZZZ_Sort");
    this->CreateTestNode("AAA_Sort");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"sort", "name"}, {"filter.name", "*_Sort"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].size() >= 2);

    const std::string first = json["data"][0]["name"].get<std::string>();
    const std::string second = json["data"][1]["name"].get<std::string>();
    CPPUNIT_ASSERT_MESSAGE("Ascending sort: AAA should come before ZZZ", first < second);
  }

  void SortDescending()
  {
    this->CreateTestNode("ZZZ_Desc");
    this->CreateTestNode("AAA_Desc");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"sort", "-name"}, {"filter.name", "*_Desc"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].size() >= 2);

    const std::string first = json["data"][0]["name"].get<std::string>();
    const std::string second = json["data"][1]["name"].get<std::string>();
    CPPUNIT_ASSERT_MESSAGE("Descending sort: ZZZ should come before AAA", first > second);
  }

  void PropertyScopeDefaultForGet()
  {
    const std::string uid = this->CreateTestNode("ScopeTestNode");

    // Per OpenAPI spec: GET /properties defaults to property_scope=all
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties", "",
      {{"uid", uid}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid_properties(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(200, res.status);

    // Meta should report scope as "all" (the default for GET)
    CPPUNIT_ASSERT_EQUAL(std::string("all"),
      json["meta"]["property_scope"].get<std::string>());
  }

  // ==========================================
  // Category 4: Status Code Conformance
  // ==========================================

  void ListNodesReturns200()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
  }

  void GetNodeReturns200()
  {
    const std::string uid = this->CreateTestNode("Status200Node");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid, "",
      {{"uid", uid}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
  }

  void CreateNodeReturns201()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes",
      R"({"name":"Created201"})", {}, {}, "application/json");
    httplib::Response res;
    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(201, res.status);
  }

  void DeleteNodeReturns200WithBody()
  {
    const std::string uid = this->CreateTestNode("ToDelete");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid, "",
      {{"uid", uid}});
    httplib::Response res;
    m_Controller->HandleDELETE_nodes_uid(req, res);

    // Per implementation: returns 200 with JSON body (not 204)
    CPPUNIT_ASSERT_EQUAL(200, res.status);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_MESSAGE("Delete response must have data.deleted_uid",
      json["data"].contains("deleted_uid"));
    CPPUNIT_ASSERT_EQUAL(uid, json["data"]["deleted_uid"].get<std::string>());
  }

  void MalformedJsonReturns400()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes",
      "not valid json{{{", {}, {}, "application/json");
    httplib::Response res;
    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"),
      json["error"]["code"].get<std::string>());
  }

  void NodeNotFoundReturns404()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent", "",
      {{"uid", "nonexistent"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"),
      json["error"]["code"].get<std::string>());
  }

  void PropertyNotFoundReturns404()
  {
    const std::string uid = this->CreateTestNode("PropNotFoundNode");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties/nonexistent", "",
      {{"uid", uid}, {"key", "nonexistent"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("PROPERTY_NOT_FOUND"),
      json["error"]["code"].get<std::string>());
  }

  void NodeHasChildrenReturns409()
  {
    const std::string parentUid = this->CreateTestNode("ParentNode409");
    this->CreateTestChildNode("Child409", parentUid);

    // Try to delete parent without recursive flag
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + parentUid, "",
      {{"uid", parentUid}});
    httplib::Response res;
    m_Controller->HandleDELETE_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(409, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_HAS_CHILDREN"),
      json["error"]["code"].get<std::string>());

    // Per spec: extended error should include children_count
    CPPUNIT_ASSERT_MESSAGE("NODE_HAS_CHILDREN error must include children_count",
      json["error"].contains("children_count"));
    CPPUNIT_ASSERT_MESSAGE("children_count must be > 0",
      json["error"]["children_count"].get<int>() > 0);
  }

  void UnsupportedContentTypeReturns415()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes",
      R"({"name":"Test"})", {}, {}, "text/plain");
    httplib::Response res;
    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(415, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("UNSUPPORTED_FORMAT"),
      json["error"]["code"].get<std::string>());
  }

  void NoDataStorageReturns503()
  {
    m_Bridge->SetDataStorage(nullptr);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"),
      json["error"]["code"].get<std::string>());
  }

  // ==========================================
  // Category 5: Content-Type Conformance
  // ==========================================

  void JsonEndpointsReturnApplicationJson()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    // Verify the response body is valid JSON
    CPPUNIT_ASSERT_NO_THROW(auto result = nlohmann::json::parse(res.body));

    // Verify Content-Type header is application/json
    bool hasJsonContentType = false;
    for (const auto& header : res.headers)
    {
      if (header.first == "Content-Type" &&
          header.second.find("application/json") != std::string::npos)
      {
        hasJsonContentType = true;
        break;
      }
    }
    CPPUNIT_ASSERT_MESSAGE("JSON endpoint must set Content-Type: application/json",
      hasJsonContentType);
  }

  void HealthEndpointReturnsJson()
  {
    httplib::Request req;
    httplib::Response res;
    m_HealthController->HandleGET_health(req, res);

    CPPUNIT_ASSERT_NO_THROW(auto result = nlohmann::json::parse(res.body));
  }

  // ==========================================
  // Category 6: Bidirectional Validation
  // ==========================================

  void AllErrorCodesHaveFactoryMethods()
  {
    // Verify that all error codes defined in ErrorResponse have corresponding
    // factory methods by checking the constant definitions exist.
    // This ensures the spec can document all codes that exist in code.

    // These are the error code constants from mitkErrorResponse.h
    CPPUNIT_ASSERT_MESSAGE("INVALID_REQUEST code must be defined",
      std::string(mitk::ErrorResponse::CODE_INVALID_REQUEST) == "INVALID_REQUEST");
    CPPUNIT_ASSERT_MESSAGE("NODE_NOT_FOUND code must be defined",
      std::string(mitk::ErrorResponse::CODE_NODE_NOT_FOUND) == "NODE_NOT_FOUND");
    CPPUNIT_ASSERT_MESSAGE("PROPERTY_NOT_FOUND code must be defined",
      std::string(mitk::ErrorResponse::CODE_PROPERTY_NOT_FOUND) == "PROPERTY_NOT_FOUND");
    CPPUNIT_ASSERT_MESSAGE("DATASTORAGE_NOT_AVAILABLE code must be defined",
      std::string(mitk::ErrorResponse::CODE_DATASTORAGE_NOT_AVAILABLE) == "DATASTORAGE_NOT_AVAILABLE");
    CPPUNIT_ASSERT_MESSAGE("INTERNAL_ERROR code must be defined",
      std::string(mitk::ErrorResponse::CODE_INTERNAL_ERROR) == "INTERNAL_ERROR");
    CPPUNIT_ASSERT_MESSAGE("PROPERTY_PROTECTED code must be defined",
      std::string(mitk::ErrorResponse::CODE_PROPERTY_PROTECTED) == "PROPERTY_PROTECTED");
    CPPUNIT_ASSERT_MESSAGE("NOT_IMPLEMENTED code must be defined",
      std::string(mitk::ErrorResponse::CODE_NOT_IMPLEMENTED) == "NOT_IMPLEMENTED");
    CPPUNIT_ASSERT_MESSAGE("NODE_HAS_CHILDREN code must be defined",
      std::string(mitk::ErrorResponse::CODE_NODE_HAS_CHILDREN) == "NODE_HAS_CHILDREN");
    CPPUNIT_ASSERT_MESSAGE("NO_DATA code must be defined",
      std::string(mitk::ErrorResponse::CODE_NO_DATA) == "NO_DATA");
    CPPUNIT_ASSERT_MESSAGE("UNSUPPORTED_FORMAT code must be defined",
      std::string(mitk::ErrorResponse::CODE_UNSUPPORTED_FORMAT) == "UNSUPPORTED_FORMAT");
    CPPUNIT_ASSERT_MESSAGE("SERIALIZATION_ERROR code must be defined",
      std::string(mitk::ErrorResponse::CODE_SERIALIZATION_ERROR) == "SERIALIZATION_ERROR");
    CPPUNIT_ASSERT_MESSAGE("FILE_NOT_FOUND code must be defined",
      std::string(mitk::ErrorResponse::CODE_FILE_NOT_FOUND) == "FILE_NOT_FOUND");
    CPPUNIT_ASSERT_MESSAGE("FILE_READ_ERROR code must be defined",
      std::string(mitk::ErrorResponse::CODE_FILE_READ_ERROR) == "FILE_READ_ERROR");
    CPPUNIT_ASSERT_MESSAGE("TRANSFER_MODE_NOT_AVAILABLE code must be defined",
      std::string(mitk::ErrorResponse::CODE_TRANSFER_MODE_NOT_AVAILABLE) == "TRANSFER_MODE_NOT_AVAILABLE");

    // Verify each factory method produces correct structure
    auto nodeError = mitk::ErrorResponse::NodeNotFound("test-uid", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), nodeError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(404, nodeError["error"]["status"].get<int>());

    auto propError = mitk::ErrorResponse::PropertyNotFound("key", "uid", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("PROPERTY_NOT_FOUND"), propError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(404, propError["error"]["status"].get<int>());

    auto dsError = mitk::ErrorResponse::DataStorageNotAvailable("/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"), dsError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(503, dsError["error"]["status"].get<int>());

    auto invalidError = mitk::ErrorResponse::InvalidRequest("detail", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), invalidError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(400, invalidError["error"]["status"].get<int>());

    auto internalError = mitk::ErrorResponse::InternalError("detail", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("INTERNAL_ERROR"), internalError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(500, internalError["error"]["status"].get<int>());

    auto protectedError = mitk::ErrorResponse::PropertyProtected("name", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("PROPERTY_PROTECTED"), protectedError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(400, protectedError["error"]["status"].get<int>());

    auto childrenError = mitk::ErrorResponse::NodeHasChildren(3, "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_HAS_CHILDREN"), childrenError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(409, childrenError["error"]["status"].get<int>());

    auto noDataError = mitk::ErrorResponse::NoData("uid", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("NO_DATA"), noDataError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(404, noDataError["error"]["status"].get<int>());

    auto formatError = mitk::ErrorResponse::UnsupportedFormat("detail", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("UNSUPPORTED_FORMAT"), formatError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(415, formatError["error"]["status"].get<int>());

    auto serError = mitk::ErrorResponse::SerializationError("detail", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("SERIALIZATION_ERROR"), serError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(500, serError["error"]["status"].get<int>());

    auto fileError = mitk::ErrorResponse::FileNotFound("/some/path", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("FILE_NOT_FOUND"), fileError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(422, fileError["error"]["status"].get<int>());

    auto readError = mitk::ErrorResponse::FileReadError("/some/path", "detail", "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("FILE_READ_ERROR"), readError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(422, readError["error"]["status"].get<int>());

    auto transferError = mitk::ErrorResponse::TransferModeNotAvailable("invalid", {"direct", "file-reference"}, "/test/path");
    CPPUNIT_ASSERT_EQUAL(std::string("TRANSFER_MODE_NOT_AVAILABLE"), transferError["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(406, transferError["error"]["status"].get<int>());
  }

  void ErrorResponseContainsInstancePath()
  {
    // Per OpenAPI spec: "instance" field should contain the request path
    const std::string requestPath = "/api/v1/datastorage/nodes/test-uid";

    auto req = this->CreateRequest(requestPath, "",
      {{"uid", "test-uid"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);

    CPPUNIT_ASSERT_MESSAGE("Error must contain 'instance'",
      json["error"].contains("instance"));
    CPPUNIT_ASSERT_EQUAL(requestPath,
      json["error"]["instance"].get<std::string>());
  }

  void CreateNodeResponseHasLocationMeta()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes",
      R"({"name":"LocationTest"})", {}, {}, "application/json");
    httplib::Response res;
    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(201, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Per OpenAPI spec: POST response meta should contain location
    CPPUNIT_ASSERT_MESSAGE("Create response must have meta.location",
      json["meta"].contains("location"));

    const std::string location = json["meta"]["location"].get<std::string>();
    const std::string uid = json["data"]["uid"].get<std::string>();
    CPPUNIT_ASSERT_MESSAGE("Location must contain the node UID",
      location.find(uid) != std::string::npos);
  }

  void ChildrenEndpointIncludesParentUid()
  {
    const std::string parentUid = this->CreateTestNode("ParentForChildren");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + parentUid + "/children", "",
      {{"uid", parentUid}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid_children(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Per OpenAPI spec: children endpoint meta should include parent_uid
    CPPUNIT_ASSERT_MESSAGE("Children response meta must have parent_uid",
      json["meta"].contains("parent_uid"));
    CPPUNIT_ASSERT_EQUAL(parentUid,
      json["meta"]["parent_uid"].get<std::string>());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkApiConformance)
