/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include "mitkDataStorageController.h"
#include "mitkHealthController.h"
#include "mitkRenderingController.h"
#include <mitkDataStorageBridge.h>
#include <mitkErrorResponse.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStringProperty.h>
#include <mitkProperties.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <utility>

/**
 * @brief API Conformance Tests - verify the OpenAPI specification matches the implementation.
 *
 * These tests ensure that the REST API implementation conforms to its OpenAPI 3.0 specification.
 * They are designed to catch drift in either direction:
 * - Implementation changes without spec update
 * - Spec changes without implementation update
 *
 * The tests load and parse the openapi.json spec at runtime, so expected values (field names,
 * types, defaults, error codes) come from the spec rather than being hardcoded.
 *
 * Test categories:
 * 1. Structural Conformance: All endpoints exist (spec <-> handler bidirectional)
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
  MITK_TEST(SpecEndpointsHaveHandlers);
  MITK_TEST(HandlersExistInSpec);

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
  MITK_TEST(FileAccessConfigResponseStructure);

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
  MITK_TEST(AllSpecErrorCodesExistInCode);
  MITK_TEST(AllCodeErrorCodesExistInSpec);
  MITK_TEST(ErrorResponseContainsInstancePath);
  MITK_TEST(CreateNodeResponseHasLocationMeta);
  MITK_TEST(ChildrenEndpointIncludesParentUid);

  CPPUNIT_TEST_SUITE_END();

private:
  using HandlerFunc = std::function<void(const httplib::Request&, httplib::Response&)>;
  using EndpointKey = std::pair<std::string, std::string>;

  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::DataStorageBridge> m_Bridge;
  std::unique_ptr<mitk::DataStorageController> m_Controller;
  std::unique_ptr<mitk::HealthController> m_HealthController;
  std::unique_ptr<mitk::RenderingController> m_RenderingController;

  nlohmann::json m_Spec;
  std::map<EndpointKey, HandlerFunc> m_EndpointRegistry;

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

  /// Build the endpoint registry mapping (OpenAPI path, HTTP method) -> handler function.
  void BuildEndpointRegistry()
  {
    m_EndpointRegistry.clear();

    // Discovery
    m_EndpointRegistry[{"/", "get"}] = [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_info(req, res);
    };
    m_EndpointRegistry[{"/health", "get"}] = [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_health(req, res);
    };
    m_EndpointRegistry[{"/info", "get"}] = [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_info(req, res);
    };
    m_EndpointRegistry[{"/config/file-access", "get"}] = [this](const httplib::Request& req, httplib::Response& res) {
      m_HealthController->HandleGET_config_file_access(req, res);
    };

    // Nodes
    m_EndpointRegistry[{"/datastorage/nodes", "get"}] = [this](const httplib::Request& req, httplib::Response& res) {
      m_Controller->HandleGET_nodes(req, res);
    };
    m_EndpointRegistry[{"/datastorage/nodes", "post"}] = [this](const httplib::Request& req, httplib::Response& res) {
      m_Controller->HandlePOST_nodes(req, res);
    };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}", "get"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandleGET_nodes_uid(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}", "patch"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandlePATCH_nodes_uid(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}", "delete"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandleDELETE_nodes_uid(req, res);
      };

    // Children
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/children", "get"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandleGET_nodes_uid_children(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/children", "post"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandlePOST_nodes_uid_children(req, res);
      };

    // Data
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/data", "get"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandleGET_nodes_uid_data(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/data", "put"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandlePUT_nodes_uid_data(req, res);
      };

    // Properties
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/properties", "get"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandleGET_nodes_uid_properties(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/properties", "put"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandlePUT_nodes_uid_properties(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/properties", "patch"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandlePATCH_nodes_uid_properties(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/properties/{key}", "get"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandleGET_nodes_uid_properties_key(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/properties/{key}", "put"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandlePUT_nodes_uid_properties_key(req, res);
      };
    m_EndpointRegistry[{"/datastorage/nodes/{uid}/properties/{key}", "delete"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_Controller->HandleDELETE_nodes_uid_properties_key(req, res);
      };

    // Rendering
    m_EndpointRegistry[{"/rendering/update", "post"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_RenderingController->HandlePOST_update(req, res);
      };
    m_EndpointRegistry[{"/rendering/reinit", "post"}] =
      [this](const httplib::Request& req, httplib::Response& res) {
        m_RenderingController->HandlePOST_reinit(req, res);
      };
  }

  /// Read required fields from components.schemas.<schemaName>.required.
  std::vector<std::string> GetRequiredFields(const std::string& schemaName) const
  {
    const auto& schema = m_Spec["components"]["schemas"][schemaName];
    if (!schema.contains("required"))
    {
      return {};
    }
    return schema["required"].get<std::vector<std::string>>();
  }

  /// Read field type from components.schemas.<schemaName>.properties.<field>.type.
  std::string GetFieldType(const std::string& schemaName, const std::string& field) const
  {
    const auto& props = m_Spec["components"]["schemas"][schemaName]["properties"];
    if (props.contains(field) && props[field].contains("type"))
    {
      return props[field]["type"].get<std::string>();
    }
    return "";
  }

  /// Check if field is nullable from components.schemas.<schemaName>.properties.<field>.nullable.
  bool IsFieldNullable(const std::string& schemaName, const std::string& field) const
  {
    const auto& props = m_Spec["components"]["schemas"][schemaName]["properties"];
    if (props.contains(field) && props[field].contains("nullable"))
    {
      return props[field]["nullable"].get<bool>();
    }
    return false;
  }

  /// Read default value for a component parameter.
  template <typename T>
  T GetParameterDefault(const std::string& paramName) const
  {
    return m_Spec["components"]["parameters"][paramName]["schema"]["default"].get<T>();
  }

  /// Read maximum value for a component parameter.
  template <typename T>
  T GetParameterMaximum(const std::string& paramName) const
  {
    return m_Spec["components"]["parameters"][paramName]["schema"]["maximum"].get<T>();
  }

  /// Collect all error codes from response examples throughout the spec.
  std::set<std::string> CollectSpecErrorCodes() const
  {
    std::set<std::string> codes;

    // Helper to extract error code from an example object
    auto extractCode = [&codes](const nlohmann::json& example) {
      if (example.contains("error") && example["error"].contains("code"))
      {
        codes.insert(example["error"]["code"].get<std::string>());
      }
    };

    // Walk all paths and their responses
    for (const auto& [path, pathItem] : m_Spec["paths"].items())
    {
      for (const auto& [method, operation] : pathItem.items())
      {
        if (!operation.contains("responses"))
        {
          continue;
        }

        for (const auto& [statusCode, response] : operation["responses"].items())
        {
          if (!response.contains("content"))
          {
            continue;
          }

          for (const auto& [mediaType, mediaObj] : response["content"].items())
          {
            // Single example
            if (mediaObj.contains("example"))
            {
              extractCode(mediaObj["example"]);
            }
            // Multiple examples
            if (mediaObj.contains("examples"))
            {
              for (const auto& [exName, exObj] : mediaObj["examples"].items())
              {
                if (exObj.contains("value"))
                {
                  extractCode(exObj["value"]);
                }
              }
            }
          }
        }
      }
    }

    // Walk shared responses in components.responses
    if (m_Spec.contains("components") && m_Spec["components"].contains("responses"))
    {
      for (const auto& [name, response] : m_Spec["components"]["responses"].items())
      {
        if (!response.contains("content"))
        {
          continue;
        }

        for (const auto& [mediaType, mediaObj] : response["content"].items())
        {
          if (mediaObj.contains("example"))
          {
            extractCode(mediaObj["example"]);
          }
          if (mediaObj.contains("examples"))
          {
            for (const auto& [exName, exObj] : mediaObj["examples"].items())
            {
              if (exObj.contains("value"))
              {
                extractCode(exObj["value"]);
              }
            }
          }
        }
      }
    }

    return codes;
  }

  /// Get all error code constants defined in ErrorResponse.
  static std::set<std::string> GetAllCodeErrorCodes()
  {
    return {
      mitk::ErrorResponse::CODE_INVALID_REQUEST,
      mitk::ErrorResponse::CODE_NODE_NOT_FOUND,
      mitk::ErrorResponse::CODE_PROPERTY_NOT_FOUND,
      mitk::ErrorResponse::CODE_DATASTORAGE_NOT_AVAILABLE,
      mitk::ErrorResponse::CODE_INTERNAL_ERROR,
      mitk::ErrorResponse::CODE_PROPERTY_PROTECTED,
      mitk::ErrorResponse::CODE_NOT_IMPLEMENTED,
      mitk::ErrorResponse::CODE_NODE_HAS_CHILDREN,
      mitk::ErrorResponse::CODE_NO_DATA,
      mitk::ErrorResponse::CODE_NO_GEOMETRY,
      mitk::ErrorResponse::CODE_RENDERING_ERROR,
      mitk::ErrorResponse::CODE_CIRCULAR_HIERARCHY_REFERENCE,
      mitk::ErrorResponse::CODE_UNSUPPORTED_FORMAT,
      mitk::ErrorResponse::CODE_SERIALIZATION_ERROR,
      mitk::ErrorResponse::CODE_FILE_NOT_FOUND,
      mitk::ErrorResponse::CODE_FILE_READ_ERROR,
      mitk::ErrorResponse::CODE_TRANSFER_MODE_NOT_AVAILABLE,
      mitk::ErrorResponse::CODE_ACCESS_DENIED,
      mitk::ErrorResponse::CODE_UNAUTHORIZED,
      mitk::ErrorResponse::CODE_RATE_LIMIT_EXCEEDED,
      mitk::ErrorResponse::CODE_FILE_ACCESS_DENIED
    };
  }

  nlohmann::json LoadOpenAPISpec()
  {
    auto restModule = us::ModuleRegistry::GetModule("MitkRESTAPI");
    if (nullptr==restModule)
    {
      mitkThrow() << "MitkRESTAPI module could not be found.";
    }

    auto* context = restModule->GetModuleContext();
    if (context == nullptr)
    {
      mitkThrow() << "MitkRESTAPI module context could not be found.";
    }

    us::ModuleResource resource = context->GetModule()->GetResource("openapi.json");
    if (!resource.IsValid())
    {
      mitkThrow() << "openapi.json Resource not found in MitkRESTAPI module bundle";
    }

    us::ModuleResourceStream stream(resource, std::ios::binary);

    return nlohmann::json::parse(stream);
  }

public:
  void setUp() override
  {
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Bridge = std::make_unique<mitk::DataStorageBridge>();
    m_Bridge->SetDataStorage(m_DataStorage);
    m_Controller = std::make_unique<mitk::DataStorageController>(*m_Bridge);
    m_HealthController = std::make_unique<mitk::HealthController>(*m_Bridge);
    m_RenderingController = std::make_unique<mitk::RenderingController>(*m_Bridge);

    m_Spec = this->LoadOpenAPISpec();

    this->BuildEndpointRegistry();
  }

  void tearDown() override
  {
    m_EndpointRegistry.clear();
    m_Spec = nullptr;
    m_RenderingController.reset();
    m_HealthController.reset();
    m_Controller.reset();
    m_Bridge->SetDataStorage(nullptr);
    m_Bridge.reset();
    m_DataStorage = nullptr;
  }

  // ==========================================
  // Category 1: Structural Conformance
  // ==========================================

  void SpecEndpointsHaveHandlers()
  {
    // Every {path, method} in the spec must have a handler in the registry.
    for (const auto& [path, pathItem] : m_Spec["paths"].items())
    {
      for (const auto& [method, operation] : pathItem.items())
      {
        const EndpointKey key{path, method};
        CPPUNIT_ASSERT_MESSAGE(
          "Spec endpoint has no handler: " + method + " " + path,
          m_EndpointRegistry.count(key) > 0);
      }
    }
  }

  void HandlersExistInSpec()
  {
    // Every handler in the registry must have a corresponding {path, method} in the spec.
    const auto& paths = m_Spec["paths"];
    for (const auto& [key, handler] : m_EndpointRegistry)
    {
      const auto& [path, method] = key;
      CPPUNIT_ASSERT_MESSAGE(
        "Handler has no spec entry: " + method + " " + path,
        paths.contains(path) && paths[path].contains(method));
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

    // Read required fields from the spec
    const auto requiredFields = this->GetRequiredFields("Node");
    CPPUNIT_ASSERT_MESSAGE("Node schema must define required fields", !requiredFields.empty());

    for (const auto& field : requiredFields)
    {
      CPPUNIT_ASSERT_MESSAGE("Node must have '" + field + "'", nodeData.contains(field));
    }
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

    // Read required fields and verify each has the correct type from spec
    const auto requiredFields = this->GetRequiredFields("Node");
    for (const auto& field : requiredFields)
    {
      const std::string specType = this->GetFieldType("Node", field);
      const bool nullable = this->IsFieldNullable("Node", field);
      const auto& value = nodeData[field];

      if (nullable && value.is_null())
      {
        continue; // null is valid for nullable fields
      }

      if (specType == "string")
      {
        CPPUNIT_ASSERT_MESSAGE(field + " must be string (or null if nullable)",
          value.is_string());
      }
      else if (specType == "integer")
      {
        CPPUNIT_ASSERT_MESSAGE(field + " must be integer",
          value.is_number_integer());
      }
    }

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

    // Per spec: Error schema has required: ["error"]
    const auto errorRequired = this->GetRequiredFields("Error");
    CPPUNIT_ASSERT_MESSAGE("Error schema must require 'error'",
      std::find(errorRequired.begin(), errorRequired.end(), "error") != errorRequired.end());
    CPPUNIT_ASSERT_MESSAGE("Error response must have 'error' key", json.contains("error"));

    // Per spec: error object has required fields
    const auto& errorProps = m_Spec["components"]["schemas"]["Error"]["properties"]["error"];
    CPPUNIT_ASSERT_MESSAGE("Error.error must have required fields", errorProps.contains("required"));

    const auto innerRequired = errorProps["required"].get<std::vector<std::string>>();
    const auto& error = json["error"];
    for (const auto& field : innerRequired)
    {
      CPPUNIT_ASSERT_MESSAGE("error must have '" + field + "'", error.contains(field));
    }
  }

  void ErrorResponseHasAllRequiredFields()
  {
    auto req = this->CreateRequest("/api/v1/datastorage/nodes/nonexistent", "",
      {{"uid", "nonexistent"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes_uid(req, res);

    auto json = nlohmann::json::parse(res.body);
    const auto& error = json["error"];

    // Read inner required fields and their types from spec
    const auto& errorSchema = m_Spec["components"]["schemas"]["Error"]["properties"]["error"];
    const auto innerRequired = errorSchema["required"].get<std::vector<std::string>>();
    const auto& innerProps = errorSchema["properties"];

    for (const auto& field : innerRequired)
    {
      CPPUNIT_ASSERT_MESSAGE("error must have '" + field + "'", error.contains(field));

      if (innerProps.contains(field) && innerProps[field].contains("type"))
      {
        const std::string specType = innerProps[field]["type"].get<std::string>();
        if (specType == "string")
        {
          CPPUNIT_ASSERT_MESSAGE("error." + field + " must be string", error[field].is_string());
        }
        else if (specType == "integer")
        {
          CPPUNIT_ASSERT_MESSAGE("error." + field + " must be integer", error[field].is_number_integer());
        }
      }
    }

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

    // Read core ListMeta fields from spec
    const auto& listMetaProps = m_Spec["components"]["schemas"]["ListMeta"]["properties"];
    const std::vector<std::string> coreFields = {"total_count", "limit", "offset", "returned_count"};
    for (const auto& field : coreFields)
    {
      CPPUNIT_ASSERT_MESSAGE("ListMeta spec must define '" + field + "'", listMetaProps.contains(field));
      CPPUNIT_ASSERT_MESSAGE("meta must have '" + field + "'", meta.contains(field));
    }

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

    // Read required fields from HealthStatus schema in spec
    const auto requiredFields = this->GetRequiredFields("HealthStatus");
    CPPUNIT_ASSERT_MESSAGE("HealthStatus schema must define required fields", !requiredFields.empty());

    for (const auto& field : requiredFields)
    {
      CPPUNIT_ASSERT_MESSAGE("Health data must have '" + field + "'", json["data"].contains(field));
    }

    CPPUNIT_ASSERT_EQUAL(std::string("healthy"), json["data"]["status"].get<std::string>());
  }

  void InfoResponseStructure()
  {
    httplib::Request req;
    httplib::Response res;
    m_HealthController->HandleGET_info(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    // Read fields from ApiInfo schema in spec
    const auto& apiInfoProps = m_Spec["components"]["schemas"]["ApiInfo"]["properties"];
    for (const auto& [field, schema] : apiInfoProps.items())
    {
      CPPUNIT_ASSERT_MESSAGE("Info data must have '" + field + "'", json["data"].contains(field));
    }
  }

  void FileAccessConfigResponseStructure()
  {
    httplib::Request req;
    httplib::Response res;
    m_HealthController->HandleGET_config_file_access(req, res);

    CPPUNIT_ASSERT_EQUAL(200, res.status);
    auto json = nlohmann::json::parse(res.body);

    CPPUNIT_ASSERT_MESSAGE("Response must have 'data' key", json.contains("data"));

    // Read required fields from FileAccessConfig schema in spec
    const auto requiredFields = this->GetRequiredFields("FileAccessConfig");
    CPPUNIT_ASSERT_MESSAGE("FileAccessConfig schema must define required fields", !requiredFields.empty());

    for (const auto& field : requiredFields)
    {
      CPPUNIT_ASSERT_MESSAGE("File access config data must have '" + field + "'",
        json["data"].contains(field));
    }

    // Default mode is unrestricted
    CPPUNIT_ASSERT_EQUAL(std::string("unrestricted"), json["data"]["mode"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(false, json["data"]["restrictions_active"].get<bool>());

    // allowed_paths should not be present when unrestricted
    CPPUNIT_ASSERT_MESSAGE("allowed_paths must not be present when unrestricted",
      !json["data"].contains("allowed_paths"));
  }

  // ==========================================
  // Category 3: Query Parameter Conformance
  // ==========================================

  void PaginationDefaultLimit()
  {
    // Read defaults from spec
    const int specDefaultLimit = this->GetParameterDefault<int>("Limit");
    const int specDefaultOffset = this->GetParameterDefault<int>("Offset");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes");
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_EQUAL(specDefaultLimit, json["meta"]["limit"].get<int>());
    CPPUNIT_ASSERT_EQUAL(specDefaultOffset, json["meta"]["offset"].get<int>());
  }

  void PaginationMaxLimitEnforced()
  {
    // Read maximum from spec
    const int specMaxLimit = this->GetParameterMaximum<int>("Limit");

    auto req = this->CreateRequest("/api/v1/datastorage/nodes", "",
      {}, {{"limit", "5000"}});
    httplib::Response res;
    m_Controller->HandleGET_nodes(req, res);

    auto json = nlohmann::json::parse(res.body);
    CPPUNIT_ASSERT_MESSAGE("limit must be capped at " + std::to_string(specMaxLimit),
      json["meta"]["limit"].get<int>() <= specMaxLimit);
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
    CPPUNIT_ASSERT_NO_THROW([[maybe_unused]] auto result = nlohmann::json::parse(res.body));

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

    CPPUNIT_ASSERT_NO_THROW([[maybe_unused]] auto result = nlohmann::json::parse(res.body));
  }

  // ==========================================
  // Category 6: Bidirectional Validation
  // ==========================================

  void AllSpecErrorCodesExistInCode()
  {
    // Every error code in the spec examples must have a constant in ErrorResponse.
    const auto specCodes = this->CollectSpecErrorCodes();
    const auto codeCodes = GetAllCodeErrorCodes();

    for (const auto& code : specCodes)
    {
      CPPUNIT_ASSERT_MESSAGE(
        "Spec error code '" + code + "' has no ErrorResponse constant",
        codeCodes.count(code) > 0);
    }
  }

  void AllCodeErrorCodesExistInSpec()
  {
    // Every error code constant in ErrorResponse must appear in at least one spec example.
    // Exception: NOT_IMPLEMENTED is implementation-only (no endpoint triggers it in normal flow).
    const auto specCodes = this->CollectSpecErrorCodes();
    const auto codeCodes = GetAllCodeErrorCodes();

    // NOT_IMPLEMENTED is an internal sentinel code used for unimplemented features;
    // no endpoint exposes it as a normal response, so it has no spec example.
    const std::set<std::string> exemptions = {"NOT_IMPLEMENTED"};

    for (const auto& code : codeCodes)
    {
      if (exemptions.count(code) > 0)
      {
        continue;
      }
      CPPUNIT_ASSERT_MESSAGE(
        "ErrorResponse code '" + code + "' has no spec example",
        specCodes.count(code) > 0);
    }
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
