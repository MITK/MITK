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
#include <mitkStringProperty.h>
#include <mitkProperties.h>

#include <nlohmann/json.hpp>

class mitkDataStorageControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDataStorageControllerTestSuite);
  // DataStorage availability tests
  MITK_TEST(GetNodesWithoutDataStorage);
  MITK_TEST(GetNodeWithoutDataStorage);

  // GET /nodes tests
  MITK_TEST(GetNodesEmpty);
  MITK_TEST(GetNodesReturnsNodes);
  MITK_TEST(GetNodesWithPagination);
  MITK_TEST(GetNodesResponseFormat);

  // GET /nodes/:uid tests
  MITK_TEST(GetNodeByUid);
  MITK_TEST(GetNodeNotFound);

  // POST /nodes tests
  MITK_TEST(CreateNode);
  MITK_TEST(CreateNodeWithInvalidJson);

  // DELETE /nodes/:uid tests
  MITK_TEST(DeleteNode);
  MITK_TEST(DeleteNodeNotFound);
  MITK_TEST(DeleteNodeWithChildrenNonRecursive);
  MITK_TEST(DeleteNodeWithChildrenRecursive);

  // GET /nodes/:uid/children tests
  MITK_TEST(GetChildren);
  MITK_TEST(GetChildrenParentNotFound);

  // POST /nodes/:uid/children tests
  MITK_TEST(CreateChildNode);

  // Property tests
  MITK_TEST(GetNodeProperties);
  MITK_TEST(GetNodeProperty);
  MITK_TEST(GetNodePropertyNotFound);
  MITK_TEST(SetNodeProperty);
  MITK_TEST(DeleteNodeProperty);
  MITK_TEST(DeleteProtectedProperty);
  MITK_TEST(PatchNodeProperties);
  MITK_TEST(PutNodeProperties);
  // Special character tests (percent-encoding)
  MITK_TEST(PropertyKeyWithSpace);
  MITK_TEST(PropertyKeyWithDot);
  MITK_TEST(PathFilterWithSpace);

  // PATCH /nodes/:uid tests
  MITK_TEST(PatchNodeReparent);
  MITK_TEST(PatchNodeReparentToNull);
  MITK_TEST(PatchNodeNotFound);

  // Fields parameter tests
  MITK_TEST(GetNodesWithFieldsSelection);
  MITK_TEST(GetNodesWithFieldsSelectionPartialMatch);

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

  // Helper to get a node UID from the data storage
  std::string GetFirstNodeUid()
  {
    mitk::NodeQueryParams params;
    auto result = m_Bridge->GetNodes(params);
    if (!result.nodes.empty())
    {
      return result.nodes[0]["uid"].get<std::string>();
    }
    return "";
  }

  // Helper to find a node's UID by querying with a name filter
  std::string FindUidByName(const std::string& name)
  {
    mitk::NodeQueryParams params;
    params.propertyFilters.push_back({"name", name, false});
    auto result = m_Bridge->GetNodes(params);
    if (result.nodes.empty())
    {
      throw std::runtime_error("Node not found: " + name);
    }
    return result.nodes[0]["uid"].get<std::string>();
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

  // ===== DataStorage availability tests =====

  void GetNodesWithoutDataStorage()
  {
    m_Bridge->SetDataStorage(nullptr);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("DATASTORAGE_NOT_AVAILABLE"), json["error"]["code"].get<std::string>());
  }

  void GetNodeWithoutDataStorage()
  {
    m_Bridge->SetDataStorage(nullptr);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/abc-123", "", {{"uid", "abc-123"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(503, res.status);
  }

  // ===== GET /nodes tests =====

  void GetNodesEmpty()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].is_array());
    CPPUNIT_ASSERT(json["data"].empty());
    CPPUNIT_ASSERT_EQUAL(0, json["meta"]["total_count"].get<int>());
  }

  void GetNodesReturnsNodes()
  {
    // Add some nodes
    auto node1 = mitk::DataNode::New();
    node1->SetName("Node1");
    m_DataStorage->Add(node1);

    auto node2 = mitk::DataNode::New();
    node2->SetName("Node2");
    m_DataStorage->Add(node2);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"].size());
    CPPUNIT_ASSERT_EQUAL(2, json["meta"]["total_count"].get<int>());
  }

  void GetNodesWithPagination()
  {
    // Add 5 nodes
    for (int i = 0; i < 5; ++i)
    {
      auto node = mitk::DataNode::New();
      node->SetName("Node" + std::to_string(i));
      m_DataStorage->Add(node);
    }

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "", {}, {{"limit", "2"}, {"offset", "1"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"].size());
    CPPUNIT_ASSERT_EQUAL(5, json["meta"]["total_count"].get<int>());
    CPPUNIT_ASSERT_EQUAL(2, json["meta"]["limit"].get<int>());
    CPPUNIT_ASSERT_EQUAL(1, json["meta"]["offset"].get<int>());
  }

  void GetNodesResponseFormat()
  {
    auto node = mitk::DataNode::New();
    node->SetName("TestNode");
    m_DataStorage->Add(node);

    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    auto nodeJson = json["data"][0];

    // Verify required fields
    CPPUNIT_ASSERT(nodeJson.contains("uid"));
    CPPUNIT_ASSERT(nodeJson.contains("name"));
    CPPUNIT_ASSERT(nodeJson.contains("path"));
    CPPUNIT_ASSERT(nodeJson.contains("children_count"));
    CPPUNIT_ASSERT_EQUAL(std::string("TestNode"), nodeJson["name"].get<std::string>());

    // Verify meta fields
    CPPUNIT_ASSERT(json["meta"].contains("total_count"));
    CPPUNIT_ASSERT(json["meta"].contains("limit"));
    CPPUNIT_ASSERT(json["meta"].contains("offset"));
    CPPUNIT_ASSERT(json["meta"].contains("returned_count"));
  }

  // ===== GET /nodes/:uid tests =====

  void GetNodeByUid()
  {
    auto node = mitk::DataNode::New();
    node->SetName("SpecificNode");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();
    CPPUNIT_ASSERT(!uid.empty());

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid, "", {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("SpecificNode"), json["data"]["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(uid, json["data"]["uid"].get<std::string>());
  }

  void GetNodeNotFound()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent", "", {{"uid", "nonexistent"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  // ===== POST /nodes tests =====

  void CreateNode()
  {
    nlohmann::json body;
    body["name"] = "NewNode";

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", body.dump(), {}, {}, "application/json");
    httplib::Response res;

    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(201, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("uid"));
    CPPUNIT_ASSERT_EQUAL(std::string("NewNode"), json["data"]["name"].get<std::string>());
    CPPUNIT_ASSERT(json["meta"].contains("location"));
  }

  void CreateNodeWithInvalidJson()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "not valid json", {}, {}, "application/json");
    httplib::Response res;

    m_Controller->HandlePOST_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), json["error"]["code"].get<std::string>());
  }

  // ===== DELETE /nodes/:uid tests =====

  void DeleteNode()
  {
    auto node = mitk::DataNode::New();
    node->SetName("ToDelete");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid, "", {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandleDELETE_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(uid, json["data"]["deleted_uid"].get<std::string>());

    // Verify node is gone
    mitk::NodeQueryParams params;
    auto nodes = m_Bridge->GetNodes(params);
    CPPUNIT_ASSERT(nodes.nodes.empty());
  }

  void DeleteNodeNotFound()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent", "", {{"uid", "nonexistent"}});
    httplib::Response res;

    m_Controller->HandleDELETE_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
  }

  void DeleteNodeWithChildrenNonRecursive()
  {
    auto parent = mitk::DataNode::New();
    parent->SetName("Parent");
    m_DataStorage->Add(parent);

    auto child = mitk::DataNode::New();
    child->SetName("Child");
    m_DataStorage->Add(child, parent.GetPointer());

    auto parentUid = this->FindUidByName("Parent");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + parentUid, "",
                                   {{"uid", parentUid}});
    httplib::Response res;

    m_Controller->HandleDELETE_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(409, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_HAS_CHILDREN"), json["error"]["code"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(1, json["error"]["children_count"].get<int>());
  }

  void DeleteNodeWithChildrenRecursive()
  {
    auto parent = mitk::DataNode::New();
    parent->SetName("ParentRecursive");
    m_DataStorage->Add(parent);

    auto child = mitk::DataNode::New();
    child->SetName("ChildRecursive");
    m_DataStorage->Add(child, parent);

    auto parentUid = this->FindUidByName("ParentRecursive");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + parentUid, "",
                                   {{"uid", parentUid}},
                                   {{"recursive", "true"}});
    httplib::Response res;

    m_Controller->HandleDELETE_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(parentUid, json["data"]["deleted_uid"].get<std::string>());
    CPPUNIT_ASSERT(json["data"].contains("deleted_children"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), json["data"]["deleted_children"].size());
  }

  // ===== GET /nodes/:uid/children tests =====

  void GetChildren()
  {
    auto parent = mitk::DataNode::New();
    parent->SetName("ParentForChildren");
    m_DataStorage->Add(parent);

    auto child1 = mitk::DataNode::New();
    child1->SetName("ChildA");
    m_DataStorage->Add(child1, parent.GetPointer());

    auto child2 = mitk::DataNode::New();
    child2->SetName("ChildB");
    m_DataStorage->Add(child2, parent);

    auto parentUid = this->FindUidByName("ParentForChildren");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + parentUid + "/children", "",
                                   {{"uid", parentUid}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_children(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"].size());
    CPPUNIT_ASSERT_EQUAL(2, json["meta"]["total_count"].get<int>());
    CPPUNIT_ASSERT_EQUAL(parentUid, json["meta"]["parent_uid"].get<std::string>());
  }

  void GetChildrenParentNotFound()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent/children", "",
                                   {{"uid", "nonexistent"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_children(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
  }

  // ===== POST /nodes/:uid/children tests =====

  void CreateChildNode()
  {
    auto parent = mitk::DataNode::New();
    parent->SetName("ParentForNewChild");
    m_DataStorage->Add(parent);

    auto parentUid = this->FindUidByName("ParentForNewChild");

    nlohmann::json body;
    body["name"] = "NewChild";

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + parentUid + "/children",
                                   body.dump(),
                                   {{"uid", parentUid}}, {}, "application/json");
    httplib::Response res;

    m_Controller->HandlePOST_nodes_uid_children(req, res);

    CPPUNIT_ASSERT_EQUAL(201, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NewChild"), json["data"]["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(parentUid, json["data"]["parent_uid"].get<std::string>());
  }

  // ===== Property tests =====

  void GetNodeProperties()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    node->SetBoolProperty("visible", true);
    node->SetFloatProperty("opacity", 0.5f);
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties", "",
                                   {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_properties(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("properties"));
    CPPUNIT_ASSERT(json["data"]["properties"].contains("visible"));
    CPPUNIT_ASSERT(json["data"]["properties"].contains("opacity"));
    CPPUNIT_ASSERT(json["meta"].contains("count"));
    CPPUNIT_ASSERT(json["meta"].contains("property_scope"));
  }

  void GetNodeProperty()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    node->SetFloatProperty("opacity", 0.75f);
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties/opacity", "",
                                   {{"uid", uid}, {"key", "opacity"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Data contains property in standard JSON serialization format: {key: value}
    CPPUNIT_ASSERT(json["data"].contains("opacity"));
    // Simple properties are just the value directly
    CPPUNIT_ASSERT(json["data"]["opacity"].is_number());

    // Verify meta contains scope and context info
    CPPUNIT_ASSERT(json.contains("meta"));
    CPPUNIT_ASSERT_EQUAL(uid, json["meta"]["node_uid"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("opacity"), json["meta"]["property_key"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("all"), json["meta"]["property_scope"].get<std::string>());
    CPPUNIT_ASSERT(json["meta"]["context"].is_null());
  }

  void GetNodePropertyNotFound()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties/nonexistent", "",
                                   {{"uid", uid}, {"key", "nonexistent"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("PROPERTY_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  void SetNodeProperty()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    nlohmann::json body;
    body["value"] = 0.5;

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties/opacity",
                                   body.dump(),
                                   {{"uid", uid}, {"key", "opacity"}});
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Data contains property in standard JSON format: {key: value}
    CPPUNIT_ASSERT(json["data"].contains("opacity"));

    // Meta contains operation info
    CPPUNIT_ASSERT(json.contains("meta"));
    CPPUNIT_ASSERT_EQUAL(uid, json["meta"]["node_uid"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("opacity"), json["meta"]["property_key"].get<std::string>());
    CPPUNIT_ASSERT(json["meta"].contains("replaced"));
  }

  void DeleteNodeProperty()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    node->SetBoolProperty("customProp", true);
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties/customProp", "",
                                   {{"uid", uid}, {"key", "customProp"}});
    httplib::Response res;

    m_Controller->HandleDELETE_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("customProp"), json["data"]["deleted"].get<std::string>());
  }

  void DeleteProtectedProperty()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties/name", "",
                                   {{"uid", uid}, {"key", "name"}});
    httplib::Response res;

    m_Controller->HandleDELETE_nodes_uid_properties_key(req, res);

    CPPUNIT_ASSERT_EQUAL(400, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("PROPERTY_PROTECTED"), json["error"]["code"].get<std::string>());
  }

  void PatchNodeProperties()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    nlohmann::json body;
    body["visible"] = true;
    body["opacity"] = 0.8;

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties",
                                   body.dump(),
                                   {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandlePATCH_nodes_uid_properties(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("updated"));
    CPPUNIT_ASSERT(json["data"].contains("failed"));
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), json["data"]["updated"].size());
  }

  void PutNodeProperties()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PropNode");
    node->SetBoolProperty("oldProp", true);
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();

    nlohmann::json body;
    body["newProp1"] = true;
    body["newProp2"] = 42;

    auto req = this->CreateRequest("/api/v1/datastorage/nodes/" + uid + "/properties",
                                   body.dump(),
                                   {{"uid", uid}});
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_properties(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"].contains("replaced"));
    CPPUNIT_ASSERT(json["meta"].contains("node_uid"));
    CPPUNIT_ASSERT(json["meta"].contains("property_scope"));
  }

  // ===== Special character tests (percent-encoding) =====
  // These tests verify that property keys with special characters work correctly.
  // In real usage, httplib decodes percent-encoded URL paths before passing to handlers.
  // Here we simulate by passing the decoded key directly in path_params.

  void PropertyKeyWithSpace()
  {
    // Test property key containing a space: "my property"
    // URL would be: /api/v1/datastorage/nodes/{uid}/properties/my%20property
    auto node = mitk::DataNode::New();
    node->SetName("SpacePropNode");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();
    std::string keyWithSpace = "my property";

    // Set a property with space in the key
    nlohmann::json body;
    body["value"] = "test value";

    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + uid + "/properties/my%20property",
      body.dump(),
      {{"uid", uid}, {"key", keyWithSpace}});  // Decoded key
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_properties_key(req, res);
    CPPUNIT_ASSERT_EQUAL(200, res.status);

    // Verify we can get the property back
    auto getReq = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + uid + "/properties/my%20property", "",
      {{"uid", uid}, {"key", keyWithSpace}});
    httplib::Response getRes;

    m_Controller->HandleGET_nodes_uid_properties_key(getReq, getRes);
    CPPUNIT_ASSERT_EQUAL(200, getRes.status);

    auto json = nlohmann::json::parse(getRes.body);
    CPPUNIT_ASSERT(json["data"].contains(keyWithSpace));
    CPPUNIT_ASSERT_EQUAL(std::string("test value"), json["data"][keyWithSpace].get<std::string>());
  }

  void PropertyKeyWithDot()
  {
    // Test property key containing a dot: "my.property"
    // Dots are generally safe in URLs but good to test
    auto node = mitk::DataNode::New();
    node->SetName("DotPropNode");
    m_DataStorage->Add(node);

    std::string uid = this->GetFirstNodeUid();
    std::string keyWithDot = "my.property";

    // Set a property with dot in the key
    nlohmann::json body;
    body["value"] = 123;

    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + uid + "/properties/" + keyWithDot,
      body.dump(),
      {{"uid", uid}, {"key", keyWithDot}});
    httplib::Response res;

    m_Controller->HandlePUT_nodes_uid_properties_key(req, res);
    CPPUNIT_ASSERT_EQUAL(200, res.status);

    // Verify we can get the property back
    auto getReq = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + uid + "/properties/" + keyWithDot, "",
      {{"uid", uid}, {"key", keyWithDot}});
    httplib::Response getRes;

    m_Controller->HandleGET_nodes_uid_properties_key(getReq, getRes);
    CPPUNIT_ASSERT_EQUAL(200, getRes.status);

    auto json = nlohmann::json::parse(getRes.body);
    CPPUNIT_ASSERT(json["data"].contains(keyWithDot));
  }

  void PathFilterWithSpace()
  {
    // Test path filter with space in node name
    // URL would be: /api/v1/datastorage/nodes?path=/My%20Node
    auto node = mitk::DataNode::New();
    node->SetName("My Node");  // Name with space
    m_DataStorage->Add(node);

    // Query with path filter containing space (decoded in query params)
    // Note: "path" is a system filter, not a property filter (not "filter.path")
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes", "",
      {},
      {{"path", "/My Node"}});  // Decoded path value
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);
    CPPUNIT_ASSERT_EQUAL(200, res.status);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(1, json["meta"]["total_count"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("My Node"), json["data"][0]["name"].get<std::string>());
  }

  // ===== PATCH /nodes/:uid tests =====

  void PatchNodeReparent()
  {
    // Create parent node
    auto parent = mitk::DataNode::New();
    parent->SetName("NewParent");
    m_DataStorage->Add(parent);

    // Create child node (initially top-level)
    auto child = mitk::DataNode::New();
    child->SetName("ChildToMove");
    m_DataStorage->Add(child);

    auto parentUid = this->FindUidByName("NewParent");
    auto childUid = this->FindUidByName("ChildToMove");

    // Reparent child under parent
    nlohmann::json body;
    body["parent_uid"] = parentUid;

    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + childUid,
      body.dump(),
      {{"uid", childUid}}, {}, "application/json");
    httplib::Response res;

    m_Controller->HandlePATCH_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(childUid, json["data"]["uid"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(parentUid, json["data"]["parent_uid"].get<std::string>());

    // Verify parent now has the child
    auto parentNode = m_Bridge->GetNode(parentUid);
    CPPUNIT_ASSERT(parentNode.has_value());
    CPPUNIT_ASSERT_EQUAL(1, parentNode.value()["children_count"].get<int>());
  }

  void PatchNodeReparentToNull()
  {
    // Create parent and child
    auto parent = mitk::DataNode::New();
    parent->SetName("ParentToLeave");
    m_DataStorage->Add(parent);

    auto child = mitk::DataNode::New();
    child->SetName("ChildToMakeTopLevel");
    m_DataStorage->Add(child, parent);

    auto childUid = this->FindUidByName("ChildToMakeTopLevel");

    // Move child to top-level (parent_uid = null)
    nlohmann::json body;
    body["parent_uid"] = nullptr;

    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/" + childUid,
      body.dump(),
      {{"uid", childUid}}, {}, "application/json");
    httplib::Response res;

    m_Controller->HandlePATCH_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT(json["data"]["parent_uid"].is_null());
  }

  void PatchNodeNotFound()
  {
    nlohmann::json body;
    body["parent_uid"] = "some-parent";

    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes/nonexistent",
      body.dump(),
      {{"uid", "nonexistent"}}, {}, "application/json");
    httplib::Response res;

    m_Controller->HandlePATCH_nodes_uid(req, res);

    CPPUNIT_ASSERT_EQUAL(404, res.status);
    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), json["error"]["code"].get<std::string>());
  }

  // ===== Fields parameter tests =====

  void GetNodesWithFieldsSelection()
  {
    auto node = mitk::DataNode::New();
    node->SetName("FieldsTestNode");
    m_DataStorage->Add(node);

    // Request only uid and name fields
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes", "",
      {},
      {{"fields", "uid,name"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Verify only requested fields are present
    auto nodeJson = json["data"][0];
    CPPUNIT_ASSERT(nodeJson.contains("uid"));
    CPPUNIT_ASSERT(nodeJson.contains("name"));
    CPPUNIT_ASSERT(!nodeJson.contains("path"));
    CPPUNIT_ASSERT(!nodeJson.contains("parent_uid"));
    CPPUNIT_ASSERT(!nodeJson.contains("children_count"));
    CPPUNIT_ASSERT(!nodeJson.contains("data_type"));

    // Verify fields are in meta
    CPPUNIT_ASSERT(json["meta"].contains("fields"));
    auto fields = json["meta"]["fields"];
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), fields.size());
  }

  void GetNodesWithFieldsSelectionPartialMatch()
  {
    auto node = mitk::DataNode::New();
    node->SetName("PartialFieldsNode");
    m_DataStorage->Add(node);

    // Request fields including one that doesn't exist
    auto req = this->CreateRequest(
      "/api/v1/datastorage/nodes", "",
      {},
      {{"fields", "uid,name,nonexistent_field"}});
    httplib::Response res;

    m_Controller->HandleGET_nodes(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Verify only existing requested fields are present
    auto nodeJson = json["data"][0];
    CPPUNIT_ASSERT(nodeJson.contains("uid"));
    CPPUNIT_ASSERT(nodeJson.contains("name"));
    CPPUNIT_ASSERT(!nodeJson.contains("nonexistent_field"));
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkDataStorageController)
