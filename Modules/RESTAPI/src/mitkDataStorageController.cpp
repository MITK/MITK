/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataStorageController.h"
#include "mitkErrorResponse.h"
#include "mitkNodeQueryParams.h"

#include <algorithm>
#include <sstream>

namespace mitk
{

namespace
{
  /**
   * @brief Parse a comma-separated string into a vector of trimmed values.
   *
   * @param input The comma-separated string.
   * @return Vector of trimmed non-empty values.
   */
  std::vector<std::string> ParseCommaSeparatedList(const std::string& input)
  {
    std::vector<std::string> result;
    std::istringstream stream(input);
    std::string item;

    while (std::getline(stream, item, ','))
    {
      // Trim leading whitespace
      auto start = item.find_first_not_of(" \t");
      if (start == std::string::npos)
      {
        continue;  // Skip empty or whitespace-only items
      }

      // Trim trailing whitespace
      auto end = item.find_last_not_of(" \t");
      item = item.substr(start, end - start + 1);

      if (!item.empty())
      {
        result.push_back(item);
      }
    }

    return result;
  }

  /**
   * @brief Build pagination links from request and query result.
   *
   * @param req The original request.
   * @param limit The pagination limit.
   * @param offset The current offset.
   * @param totalCount Total number of items.
   * @param returnedCount Number of items returned in this response.
   * @return JSON object with "prev" and "next" links (if applicable).
   */
  nlohmann::json BuildPaginationLinks(
    const httplib::Request& req,
    int limit,
    int offset,
    int totalCount,
    int returnedCount)
  {
    nlohmann::json links = nlohmann::json::object();

    // Build base URL from request path
    std::string basePath = req.path;

    // Collect query parameters, excluding offset (we'll set it ourselves)
    std::vector<std::pair<std::string, std::string>> preservedParams;
    for (const auto& param : req.params)
    {
      if (param.first != "offset")
      {
        preservedParams.push_back(param);
      }
    }

    // Build query string helper
    auto buildUrl = [&basePath, &preservedParams](int newOffset) -> std::string {
      std::ostringstream url;
      url << basePath << "?";

      bool first = true;
      for (const auto& param : preservedParams)
      {
        if (!first) url << "&";
        url << param.first << "=" << param.second;
        first = false;
      }

      if (!first) url << "&";
      url << "offset=" << newOffset;

      return url.str();
    };

    // Previous page link (only if not at the beginning)
    if (offset > 0)
    {
      int prevOffset = std::max(0, offset - limit);
      links["prev"] = buildUrl(prevOffset);
    }

    // Next page link (only if there are more items)
    if (offset + returnedCount < totalCount)
    {
      int nextOffset = offset + limit;
      links["next"] = buildUrl(nextOffset);
    }

    return links;
  }
}  // anonymous namespace

DataStorageController::DataStorageController(DataStorageBridge& bridge)
  : m_Bridge(bridge)
{
}

NodeQueryParams DataStorageController::ParseNodeQueryParams(const httplib::Request& req) const
{
  NodeQueryParams params;

  // Pagination
  if (req.has_param("limit"))
  {
    try
    {
      params.limit = std::stoi(req.get_param_value("limit"));
      params.limit = std::clamp(params.limit, 1, 1000);
    }
    catch (const std::exception&) {}
  }

  if (req.has_param("offset"))
  {
    try
    {
      params.offset = std::max(0, std::stoi(req.get_param_value("offset")));
    }
    catch (const std::exception&) {}
  }

  // Hierarchy (per API spec: "all" or "toplevel")
  if (req.has_param("hierarchy"))
  {
    std::string hierarchyStr = req.get_param_value("hierarchy");
    if (hierarchyStr == "toplevel")
    {
      params.hierarchy = Hierarchy::Toplevel;
    }
    // Default is "all"
  }

  // Path filter
  if (req.has_param("path"))
  {
    params.path = req.get_param_value("path");
  }

  // Data type filter
  if (req.has_param("data_type"))
  {
    params.dataType = req.get_param_value("data_type");
  }

  // Parent UID filter
  if (req.has_param("parent_uid"))
  {
    params.parentUid = req.get_param_value("parent_uid");
  }

  // Property scope for filtering (per API spec: "all", "node", "data")
  if (req.has_param("property_scope"))
  {
    std::string scopeStr = req.get_param_value("property_scope");
    if (scopeStr == "node")
    {
      params.propertyScope = PropertyScope::Node;
    }
    else if (scopeStr == "data")
    {
      params.propertyScope = PropertyScope::Data;
    }
    // Default is "all"
  }

  // Context (for renderer-specific properties)
  if (req.has_param("context"))
  {
    params.context = req.get_param_value("context");
  }

  // Property filters (per API spec: filter.{property}=value, filter.{property}!=value)
  // Supports: exact match, negation (!=), prefix wildcard (CT*), suffix wildcard (*Scan)
  for (const auto& param : req.params)
  {
    const std::string& paramKey = param.first;
    std::string paramValue = param.second;

    // Check for filter.X patterns
    if (paramKey.rfind("filter.", 0) == 0)
    {
      std::string propertyKey = paramKey.substr(7);  // After "filter."
      PropertyFilter propFilter;
      propFilter.negated = false;

      // Handle negation operator (filter.property!=value)
      // URL parsing may result in either:
      // 1. key="filter.visible!" value="true" (if != is parsed as part of key)
      // 2. key="filter.visible" value="!true" (if ! is parsed as part of value)
      if (!propertyKey.empty() && propertyKey.back() == '!')
      {
        propertyKey = propertyKey.substr(0, propertyKey.length() - 1);
        propFilter.negated = true;
      }
      else if (!paramValue.empty() && paramValue.front() == '!')
      {
        paramValue = paramValue.substr(1);
        propFilter.negated = true;
      }

      propFilter.key = propertyKey;
      propFilter.value = paramValue;

      params.propertyFilters.push_back(propFilter);
    }
  }

  // Fields selection (comma-separated list)
  if (req.has_param("fields"))
  {
    params.fields = ParseCommaSeparatedList(req.get_param_value("fields"));
  }

  // Sort (per API spec: sort=name for ascending, sort=-timestamp for descending)
  if (req.has_param("sort"))
  {
    std::string sortStr = req.get_param_value("sort");
    SortSpec sortSpec;

    if (!sortStr.empty() && sortStr[0] == '-')
    {
      sortSpec.ascending = false;
      sortSpec.field = sortStr.substr(1);
    }
    else
    {
      sortSpec.ascending = true;
      sortSpec.field = sortStr;
    }

    params.sort = sortSpec;
  }

  return params;
}

PropertyQueryParams DataStorageController::ParsePropertyQueryParams(const httplib::Request& req) const
{
  PropertyQueryParams params;

  // Property scope (per API spec: "all", "node", "data")
  if (req.has_param("property_scope"))
  {
    std::string scopeStr = req.get_param_value("property_scope");
    if (scopeStr == "node")
    {
      params.scope = PropertyScope::Node;
    }
    else if (scopeStr == "data")
    {
      params.scope = PropertyScope::Data;
    }
    // Default is "all"
  }

  // Context (for renderer-specific properties)
  if (req.has_param("context"))
  {
    params.context = req.get_param_value("context");
  }

  // Content parameter (per API spec: content=true/false to include values)
  if (req.has_param("content"))
  {
    std::string contentStr = req.get_param_value("content");
    params.includeContent = (contentStr != "false" && contentStr != "0");
  }

  // Names - comma-separated list of specific property names to return
  if (req.has_param("names"))
  {
    params.names = ParseCommaSeparatedList(req.get_param_value("names"));
  }

  return params;
}

void DataStorageController::SendJsonResponse(httplib::Response& res, int status, const nlohmann::json& body)
{
  res.status = status;
  res.set_content(body.dump(), "application/json");
}

void DataStorageController::SendErrorResponse(httplib::Response& res, int status, const nlohmann::json& error)
{
  res.status = status;
  res.set_content(error.dump(), "application/json");
}

void DataStorageController::HandleGET_nodes(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  // Parse query parameters
  auto params = this->ParseNodeQueryParams(req);

  auto queryResult = m_Bridge.GetNodes(params);

  int returnedCount = static_cast<int>(queryResult.nodes.size());

  nlohmann::json response;
  response["data"] = queryResult.nodes;
  response["meta"]["total_count"] = queryResult.totalCount;
  response["meta"]["limit"] = queryResult.limit;
  response["meta"]["offset"] = queryResult.offset;
  response["meta"]["returned_count"] = returnedCount;

  // Include pagination links if applicable
  auto links = BuildPaginationLinks(req, queryResult.limit, queryResult.offset, queryResult.totalCount, returnedCount);
  if (!links.empty())
  {
    response["meta"]["links"] = links;
  }

  // Include query info: context, filters, sorting, field limitations
  response["meta"]["context"] = params.context.has_value() ? nlohmann::json(params.context.value()) : nlohmann::json(nullptr);

  // Property scope
  std::string scopeStr = "all";
  if (params.propertyScope == PropertyScope::Node)
  {
    scopeStr = "node";
  }
  else if (params.propertyScope == PropertyScope::Data)
  {
    scopeStr = "data";
  }
  response["meta"]["property_scope"] = scopeStr;

  // Filters applied
  nlohmann::json filtersApplied = nlohmann::json::object();
  if (params.hierarchy == Hierarchy::Toplevel)
  {
    filtersApplied["hierarchy"] = "toplevel";
  }
  if (params.path.has_value())
  {
    filtersApplied["path"] = params.path.value();
  }
  if (params.dataType.has_value())
  {
    filtersApplied["data_type"] = params.dataType.value();
  }
  if (params.parentUid.has_value())
  {
    filtersApplied["parent_uid"] = params.parentUid.value();
  }
  if (!params.propertyFilters.empty())
  {
    nlohmann::json propFilters = nlohmann::json::array();
    for (const auto& pf : params.propertyFilters)
    {
      nlohmann::json filter;
      filter["key"] = pf.key;
      filter["value"] = pf.value;
      filter["negated"] = pf.negated;
      propFilters.push_back(filter);
    }
    filtersApplied["property_filters"] = propFilters;
  }
  if (!filtersApplied.empty())
  {
    response["meta"]["filters"] = filtersApplied;
  }

  // Sorting applied
  if (params.sort.has_value())
  {
    nlohmann::json sortInfo;
    sortInfo["field"] = params.sort.value().field;
    sortInfo["ascending"] = params.sort.value().ascending;
    response["meta"]["sort"] = sortInfo;
  }

  // Field limitations
  if (!params.fields.empty())
  {
    response["meta"]["fields"] = params.fields;
  }

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandlePOST_nodes(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  // Parse request body
  nlohmann::json nodeData;
  try
  {
    if (!req.body.empty())
    {
      nodeData = nlohmann::json::parse(req.body);
    }
  }
  catch (const nlohmann::json::parse_error& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Invalid JSON: " + std::string(e.what()), req.path));
    return;
  }

  auto createResult = m_Bridge.CreateNode(nodeData);
  if (!createResult.success)
  {
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to create node", req.path));
    return;
  }

  // Get the created node details
  auto node = m_Bridge.GetNode(createResult.uid);

  nlohmann::json response;
  response["data"] = node.value();
  response["meta"]["location"] = "/api/v1/datastorage/nodes/" + createResult.uid;

  // Report any properties that failed to deserialize
  if (!createResult.failedProperties.empty())
  {
    response["meta"]["failed_properties"] = createResult.failedProperties;
  }

  this->SendJsonResponse(res, 201, response);
}

void DataStorageController::HandleGET_nodes_uid(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");

  auto node = m_Bridge.GetNode(uid);
  if (!node.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Get available contexts for this node
  auto availableContexts = m_Bridge.GetNodeAvailableContexts(uid);

  nlohmann::json response;
  response["data"] = node.value();
  if (availableContexts.has_value())
  {
    response["meta"]["available_contexts"] = availableContexts.value();
  }

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandlePATCH_nodes_uid(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");

  // Check if node exists
  auto existingNode = m_Bridge.GetNode(uid);
  if (!existingNode.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Parse request body
  nlohmann::json updates;
  try
  {
    updates = nlohmann::json::parse(req.body);
  }
  catch (const nlohmann::json::parse_error& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Invalid JSON: " + std::string(e.what()), req.path));
    return;
  }

  if (!m_Bridge.UpdateNode(uid, updates))
  {
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to update node", req.path));
    return;
  }

  // Return updated node
  auto updatedNode = m_Bridge.GetNode(uid);

  nlohmann::json response;
  response["data"] = updatedNode.value();

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandleDELETE_nodes_uid(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");

  // Parse recursive parameter (default: false)
  bool recursive = false;
  if (req.has_param("recursive"))
  {
    std::string recursiveStr = req.get_param_value("recursive");
    recursive = (recursiveStr == "true" || recursiveStr == "1");
  }

  // Check if node exists first
  auto existingNode = m_Bridge.GetNode(uid);
  if (!existingNode.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  auto deleteResult = m_Bridge.DeleteNode(uid, recursive);

  if (!deleteResult.success)
  {
    // Check if failure was due to having children
    if (deleteResult.childrenCount > 0)
    {
      this->SendErrorResponse(res, 409, ErrorResponse::NodeHasChildren(deleteResult.childrenCount, req.path));
      return;
    }
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to delete node", req.path));
    return;
  }

  nlohmann::json response;
  response["data"]["deleted_uid"] = uid;
  if (!deleteResult.deletedChildren.empty())
  {
    response["data"]["deleted_children"] = deleteResult.deletedChildren;
  }

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandleGET_nodes_uid_children(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string parentUid = req.path_params.at("uid");

  // Check if parent node exists
  auto parentNode = m_Bridge.GetNode(parentUid);
  if (!parentNode.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(parentUid, req.path));
    return;
  }

  // Parse query parameters and set parent filter
  auto params = this->ParseNodeQueryParams(req);
  params.parentUid = parentUid;

  // Use GetNodes with parent filter - eliminates need for separate GetChildren method
  auto queryResult = m_Bridge.GetNodes(params);

  int returnedCount = static_cast<int>(queryResult.nodes.size());

  nlohmann::json response;
  response["data"] = queryResult.nodes;
  response["meta"]["parent_uid"] = parentUid;
  response["meta"]["total_count"] = queryResult.totalCount;
  response["meta"]["limit"] = queryResult.limit;
  response["meta"]["offset"] = queryResult.offset;
  response["meta"]["returned_count"] = returnedCount;

  // Include pagination links if applicable
  auto links = BuildPaginationLinks(req, queryResult.limit, queryResult.offset, queryResult.totalCount, returnedCount);
  if (!links.empty())
  {
    response["meta"]["links"] = links;
  }

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandlePOST_nodes_uid_children(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string parentUid = req.path_params.at("uid");

  // Check if parent node exists
  auto parentNode = m_Bridge.GetNode(parentUid);
  if (!parentNode.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(parentUid, req.path));
    return;
  }

  // Parse request body
  nlohmann::json nodeData;
  try
  {
    if (!req.body.empty())
    {
      nodeData = nlohmann::json::parse(req.body);
    }
  }
  catch (const nlohmann::json::parse_error& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Invalid JSON: " + std::string(e.what()), req.path));
    return;
  }

  // Use CreateNode with parent UID - eliminates need for separate CreateChildNode method
  auto createResult = m_Bridge.CreateNode(nodeData, parentUid);
  if (!createResult.success)
  {
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to create child node", req.path));
    return;
  }

  // Get the created node details
  auto node = m_Bridge.GetNode(createResult.uid);

  nlohmann::json response;
  response["data"] = node.value();
  response["meta"]["location"] = "/api/v1/datastorage/nodes/" + createResult.uid;

  // Report any properties that failed to deserialize
  if (!createResult.failedProperties.empty())
  {
    response["meta"]["failed_properties"] = createResult.failedProperties;
  }

  this->SendJsonResponse(res, 201, response);
}

void DataStorageController::HandleGET_nodes_uid_properties(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");

  // Parse property query parameters
  auto params = this->ParsePropertyQueryParams(req);

  auto properties = m_Bridge.GetNodeProperties(uid, params);
  if (!properties.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Get available contexts for this node
  auto availableContexts = m_Bridge.GetNodeAvailableContexts(uid);

  nlohmann::json response;
  response["data"]["properties"] = properties.value();
  response["meta"]["count"] = properties.value().size();

  // Report actual scope used (per API spec: "all", "node", "data")
  std::string scopeStr = "all";
  if (params.scope == PropertyScope::Node)
  {
    scopeStr = "node";
  }
  else if (params.scope == PropertyScope::Data)
  {
    scopeStr = "data";
  }
  response["meta"]["property_scope"] = scopeStr;
  response["meta"]["context"] = params.context.has_value() ? nlohmann::json(params.context.value()) : nlohmann::json(nullptr);
  if (availableContexts.has_value())
  {
    response["meta"]["available_contexts"] = availableContexts.value();
  }

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandleGET_nodes_uid_properties_key(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");
  std::string key = req.path_params.at("key");

  // First check if node exists
  auto nodeCheck = m_Bridge.GetNode(uid);
  if (!nodeCheck.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Parse property query parameters for context and scope
  auto params = this->ParsePropertyQueryParams(req);

  auto property = m_Bridge.GetNodeProperty(uid, key, params);
  if (!property.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::PropertyNotFound(key, uid, req.path));
    return;
  }

  // Build scope string for response
  std::string scopeStr = "all";
  if (params.scope == PropertyScope::Node)
  {
    scopeStr = "node";
  }
  else if (params.scope == PropertyScope::Data)
  {
    scopeStr = "data";
  }

  nlohmann::json response;
  response["data"] = property.value();
  response["meta"]["node_uid"] = uid;
  response["meta"]["property_key"] = key;
  response["meta"]["property_scope"] = scopeStr;
  response["meta"]["context"] = params.context.has_value() ? nlohmann::json(params.context.value()) : nlohmann::json(nullptr);

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandlePUT_nodes_uid_properties_key(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");
  std::string key = req.path_params.at("key");

  // Check if node exists
  auto nodeCheck = m_Bridge.GetNode(uid);
  if (!nodeCheck.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Parse property query parameters for context and scope
  auto params = this->ParsePropertyQueryParams(req);

  // Parse request body
  nlohmann::json value;
  try
  {
    value = nlohmann::json::parse(req.body);
  }
  catch (const nlohmann::json::parse_error& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Invalid JSON: " + std::string(e.what()), req.path));
    return;
  }

  // Get previous value if exists
  auto previousProperty = m_Bridge.GetNodeProperty(uid, key, params);

  if (!m_Bridge.SetNodeProperty(uid, key, value, params))
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Failed to set property value", req.path));
    return;
  }

  auto updatedProperty = m_Bridge.GetNodeProperty(uid, key, params);

  nlohmann::json response;
  if (updatedProperty.has_value())
  {
    response["data"] = updatedProperty.value();
  }
  response["meta"]["node_uid"] = uid;
  response["meta"]["property_key"] = key;
  response["meta"]["replaced"] = previousProperty.has_value();

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandleDELETE_nodes_uid_properties_key(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");
  std::string key = req.path_params.at("key");

  // Check if node exists
  auto nodeCheck = m_Bridge.GetNode(uid);
  if (!nodeCheck.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Parse property query parameters for context and scope
  auto params = this->ParsePropertyQueryParams(req);

  // Check for protected properties (like "name") - only protected in node scope
  if (key == "name" && params.scope != PropertyScope::Data)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::PropertyProtected(key, req.path));
    return;
  }

  if (!m_Bridge.DeleteNodeProperty(uid, key, params))
  {
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to delete property", req.path));
    return;
  }

  nlohmann::json response;
  response["data"]["deleted"] = key;

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandlePUT_nodes_uid_properties(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");

  // Check if node exists
  auto nodeCheck = m_Bridge.GetNode(uid);
  if (!nodeCheck.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Parse property query parameters for context and scope
  auto params = this->ParsePropertyQueryParams(req);

  // Parse request body
  nlohmann::json properties;
  try
  {
    properties = nlohmann::json::parse(req.body);
  }
  catch (const nlohmann::json::parse_error& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Invalid JSON: " + std::string(e.what()), req.path));
    return;
  }

  if (!properties.is_object())
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Request body must be a JSON object", req.path));
    return;
  }

  auto replaceResult = m_Bridge.ReplaceNodeProperties(uid, properties, params);
  if (!replaceResult.has_value())
  {
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to replace properties", req.path));
    return;
  }

  // Build scope string for response
  std::string scopeStr = "all";
  if (params.scope == PropertyScope::Node)
  {
    scopeStr = "node";
  }
  else if (params.scope == PropertyScope::Data)
  {
    scopeStr = "data";
  }

  nlohmann::json response;
  response["data"] = replaceResult.value();
  response["meta"]["node_uid"] = uid;
  response["meta"]["property_scope"] = scopeStr;
  response["meta"]["context"] = params.context.has_value() ? nlohmann::json(params.context.value()) : nlohmann::json(nullptr);

  this->SendJsonResponse(res, 200, response);
}

void DataStorageController::HandlePATCH_nodes_uid_properties(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  std::string uid = req.path_params.at("uid");

  // Check if node exists
  auto nodeCheck = m_Bridge.GetNode(uid);
  if (!nodeCheck.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Parse property query parameters for context and scope
  auto params = this->ParsePropertyQueryParams(req);

  // Parse request body
  nlohmann::json properties;
  try
  {
    properties = nlohmann::json::parse(req.body);
  }
  catch (const nlohmann::json::parse_error& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Invalid JSON: " + std::string(e.what()), req.path));
    return;
  }

  if (!properties.is_object())
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest("Request body must be a JSON object", req.path));
    return;
  }

  std::vector<std::string> updated;
  std::vector<nlohmann::json> failed;

  for (auto& [key, value] : properties.items())
  {
    if (m_Bridge.SetNodeProperty(uid, key, value, params))
    {
      updated.push_back(key);
    }
    else
    {
      nlohmann::json failure;
      failure["property"] = key;
      failure["error"] = "Failed to set property value";
      failed.push_back(failure);
    }
  }

  // Build scope string for response
  std::string scopeStr = "all";
  if (params.scope == PropertyScope::Node)
  {
    scopeStr = "node";
  }
  else if (params.scope == PropertyScope::Data)
  {
    scopeStr = "data";
  }

  nlohmann::json response;
  response["data"]["updated"] = updated;
  response["data"]["failed"] = failed;
  response["meta"]["node_uid"] = uid;
  response["meta"]["property_scope"] = scopeStr;
  response["meta"]["context"] = params.context.has_value() ? nlohmann::json(params.context.value()) : nlohmann::json(nullptr);

  this->SendJsonResponse(res, 200, response);
}

}
