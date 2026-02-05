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

#include <mitkBaseDataSerializer.h>
#include <mitkFileSystem.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkPointSet.h>

#include <vtkPolyData.h>

#include <itkObjectFactoryBase.h>

#include <algorithm>
#include <fstream>
#include <regex>
#include <sstream>

namespace mitk
{

namespace
{
  // JSON key constants to avoid magic strings
  constexpr const char* JSON_KEY_TRANSFER = "transfer";
  constexpr const char* JSON_KEY_MODE = "mode";
  constexpr const char* JSON_KEY_FILE_PATH = "file_path";
  constexpr const char* JSON_KEY_DIRECTORY_PATH = "directory_path";
  constexpr const char* JSON_KEY_SIZE_BYTES = "size_bytes";
  constexpr const char* JSON_KEY_DATA_METADATA = "data_metadata";
  constexpr const char* JSON_KEY_DATA = "data";
  constexpr const char* JSON_KEY_META = "meta";
  constexpr const char* JSON_KEY_WARNING = "warning";
  constexpr const char* JSON_KEY_UID = "uid";
  constexpr const char* JSON_KEY_NAME = "name";
  constexpr const char* JSON_KEY_KEY = "key";
  constexpr const char* JSON_KEY_VALUE = "value";
  constexpr const char* JSON_KEY_NEGATED = "negated";
  constexpr const char* JSON_KEY_ERROR = "error";
  constexpr const char* JSON_KEY_PROPERTY = "property";

  // Transfer mode constants
  constexpr const char* TRANSFER_MODE_DIRECT = "direct";
  constexpr const char* TRANSFER_MODE_FILE_REFERENCE = "file-reference";

  // MIME type constants
  constexpr const char* MIME_APPLICATION_JSON = "application/json";
  constexpr const char* MIME_APPLICATION_OCTET_STREAM = "application/octet-stream";

  // Header constants
  constexpr const char* HEADER_TRANSFER_MODE = "X-MITK-Transfer-Mode";
  constexpr const char* HEADER_DATA_FORMAT = "X-MITK-Data-Format";
  constexpr const char* HEADER_ACCEPT = "Accept";
  constexpr const char* HEADER_CONTENT_TYPE = "Content-Type";
  constexpr const char* HEADER_CONTENT_DISPOSITION = "Content-Disposition";
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

void DataStorageController::SetTempDirectory(const std::string& tempDir)
{
  m_TempDirectory = tempDir;
}

std::string DataStorageController::DetermineTransferMode(const httplib::Request& req) const
{
  // Check X-MITK-Transfer-Mode header first
  if (req.has_header(HEADER_TRANSFER_MODE))
  {
    const std::string mode = req.get_header_value(HEADER_TRANSFER_MODE);
    if (mode == TRANSFER_MODE_FILE_REFERENCE || mode == TRANSFER_MODE_DIRECT)
    {
      return mode;
    }
  }

  // Fall back to Accept header
  if (req.has_header(HEADER_ACCEPT))
  {
    const std::string accept = req.get_header_value(HEADER_ACCEPT);
    if (accept.find(MIME_APPLICATION_JSON) != std::string::npos)
    {
      return TRANSFER_MODE_FILE_REFERENCE;
    }
    if (accept.find(MIME_APPLICATION_OCTET_STREAM) != std::string::npos)
    {
      return TRANSFER_MODE_DIRECT;
    }
  }

  // Default to direct mode
  return TRANSFER_MODE_DIRECT;
}

std::string DataStorageController::ExtractFilenameFromContentDisposition(const httplib::Request& req) const
{
  if (!req.has_header(HEADER_CONTENT_DISPOSITION))
  {
    return "";
  }

  const std::string disposition = req.get_header_value(HEADER_CONTENT_DISPOSITION);

  // Parse filename from Content-Disposition header
  // Format: attachment; filename="name.ext" or filename=name.ext
  const std::regex filenameRegex(R"(filename\s*=\s*(?:\"([^\"]+)\"|([^\s;]+)))");
  std::smatch match;
  if (std::regex_search(disposition, match, filenameRegex))
  {
    // match[1] is quoted filename, match[2] is unquoted
    return match[1].matched ? match[1].str() : match[2].str();
  }

  return "";
}

nlohmann::json DataStorageController::BuildDataMetadata(const mitk::BaseData* data) const
{
  nlohmann::json metadata;

  if (data == nullptr)
  {
    return metadata;
  }

  // Handle Image data
  if (auto* image = dynamic_cast<const mitk::Image*>(data))
  {
    // Dimensions
    nlohmann::json dimensions = nlohmann::json::array();
    for (unsigned int i = 0; i < image->GetDimension(); ++i)
    {
      dimensions.push_back(static_cast<int>(image->GetDimension(i)));
    }
    metadata["dimensions"] = dimensions;

    // Spacing
    auto geometry = image->GetGeometry();
    if (geometry != nullptr)
    {
      auto spacing = geometry->GetSpacing();
      metadata["spacing"] = {spacing[0], spacing[1], spacing[2]};

      auto origin = geometry->GetOrigin();
      metadata["origin"] = {origin[0], origin[1], origin[2]};
    }

    // Pixel type
    auto pixelType = image->GetPixelType();
    metadata["pixel_type"] = pixelType.GetComponentTypeAsString();
  }
  // Handle Surface data
  else if (auto* surface = dynamic_cast<const mitk::Surface*>(data))
  {
    auto* vtkPoly = surface->GetVtkPolyData();
    if (vtkPoly != nullptr)
    {
      metadata["points_count"] = static_cast<int>(vtkPoly->GetNumberOfPoints());
      metadata["cells_count"] = static_cast<int>(vtkPoly->GetNumberOfCells());

      double bounds[6];
      vtkPoly->GetBounds(bounds);
      metadata["bounds"] = {bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
    }
  }
  // Handle PointSet data
  else if (auto* pointSet = dynamic_cast<const mitk::PointSet*>(data))
  {
    metadata["points_count"] = static_cast<int>(pointSet->GetSize());

    auto geometry = pointSet->GetGeometry();
    if (geometry != nullptr)
    {
      auto bounds = geometry->GetBounds();
      metadata["bounds"] = {bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]};
    }
  }

  return metadata;
}

void DataStorageController::SendBinaryResponse(httplib::Response& res, const std::string& data,
                                               const std::string& filename)
{
  res.status = 200;
  res.set_content(data, MIME_APPLICATION_OCTET_STREAM);
  // Content-Disposition includes filename with extension; format can be derived from extension
  res.set_header(HEADER_CONTENT_DISPOSITION, "attachment; filename=\"" + filename + "\"");
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

PropertyQueryParams DataStorageController::ParsePropertyQueryParams(const httplib::Request& req, PropertyScope defaultScope) const
{
  PropertyQueryParams params;
  params.scope = defaultScope;

  // Property scope
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
    else if (scopeStr == "all")
    {
      params.scope = PropertyScope::All;
    }
    // If invalid value, keep the default
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

DataStorageController::ResolveDataPathResult DataStorageController::ResolveDataPath(
  const httplib::Request& req, const std::string& contentType)
{
  ResolveDataPathResult result;
  result.success = false;
  result.isTemporary = false;
  result.errorStatus = 0;

  const bool isFileReference = (contentType.find(MIME_APPLICATION_JSON) != std::string::npos);
  const bool isDirectTransfer = (contentType.find(MIME_APPLICATION_OCTET_STREAM) != std::string::npos);

  if (isFileReference)
  {
    // Parse JSON body to get file path
    nlohmann::json body;
    try
    {
      body = nlohmann::json::parse(req.body);
    }
    catch (const nlohmann::json::parse_error& e)
    {
      result.errorStatus = 400;
      result.errorResponse = ErrorResponse::InvalidRequest(
        "Invalid JSON: " + std::string(e.what()), req.path);
      return result;
    }

    // Validate transfer mode if specified
    if (body.contains(JSON_KEY_TRANSFER) && body[JSON_KEY_TRANSFER].contains(JSON_KEY_MODE))
    {
      const std::string specifiedMode = body[JSON_KEY_TRANSFER][JSON_KEY_MODE].get<std::string>();
      if (specifiedMode != TRANSFER_MODE_DIRECT && specifiedMode != TRANSFER_MODE_FILE_REFERENCE)
      {
        result.errorStatus = 406;
        result.errorResponse = ErrorResponse::TransferModeNotAvailable(
          specifiedMode, {TRANSFER_MODE_FILE_REFERENCE, TRANSFER_MODE_DIRECT}, req.path);
        return result;
      }
    }

    if (!body.contains(JSON_KEY_TRANSFER) || !body[JSON_KEY_TRANSFER].contains(JSON_KEY_FILE_PATH))
    {
      result.errorStatus = 400;
      result.errorResponse = ErrorResponse::InvalidRequest(
        "Missing transfer.file_path in request body", req.path);
      return result;
    }

    const fs::path filePath(body[JSON_KEY_TRANSFER][JSON_KEY_FILE_PATH].get<std::string>());

    if (!fs::exists(filePath))
    {
      result.errorStatus = 422;
      result.errorResponse = ErrorResponse::FileNotFound(filePath.string(), req.path);
      return result;
    }

    result.success = true;
    result.filePath = filePath.string();
    result.isTemporary = false;
  }
  else if (isDirectTransfer)
  {
    if (req.body.empty())
    {
      result.errorStatus = 400;
      result.errorResponse = ErrorResponse::InvalidRequest("Request body is empty", req.path);
      return result;
    }

    // Get file extension from Content-Disposition or X-MITK-Data-Format
    std::string extension;
    const std::string filename = this->ExtractFilenameFromContentDisposition(req);
    if (!filename.empty())
    {
      const fs::path filenamePath(filename);
      if (filenamePath.has_extension())
      {
        extension = filenamePath.extension().string();
      }
    }

    if (extension.empty() && req.has_header(HEADER_DATA_FORMAT))
    {
      extension = "." + req.get_header_value(HEADER_DATA_FORMAT);
    }

    if (extension.empty())
    {
      result.errorStatus = 400;
      result.errorResponse = ErrorResponse::InvalidRequest(
        "Cannot determine file format. Provide filename in Content-Disposition header "
        "or use X-MITK-Data-Format header.", req.path);
      return result;
    }

    // Create temp file
    fs::path tempFilePath;
    try
    {
      tempFilePath = fs::path(IOUtil::CreateTemporaryFile("upload_XXXXXX" + extension, m_TempDirectory));
    }
    catch (const std::exception& e)
    {
      result.errorStatus = 500;
      result.errorResponse = ErrorResponse::InternalError(
        "Failed to create temporary file: " + std::string(e.what()), req.path);
      return result;
    }

    // Write request body to temp file
    try
    {
      std::ofstream outFile(tempFilePath, std::ios::binary);
      if (!outFile.is_open())
      {
        result.errorStatus = 500;
        result.errorResponse = ErrorResponse::InternalError(
          "Failed to open temporary file for writing", req.path);
        return result;
      }
      outFile.write(req.body.c_str(), static_cast<std::streamsize>(req.body.size()));
      outFile.close();
    }
    catch (const std::exception& e)
    {
      fs::remove(tempFilePath);
      result.errorStatus = 500;
      result.errorResponse = ErrorResponse::InternalError(
        "Failed to write temporary file: " + std::string(e.what()), req.path);
      return result;
    }

    result.success = true;
    result.filePath = tempFilePath.string();
    result.isTemporary = true;
  }
  else
  {
    result.errorStatus = 415;
    result.errorResponse = ErrorResponse::UnsupportedFormat(
      "Unsupported Content-Type. Use 'application/json' for file-reference mode or "
      "'application/octet-stream' for direct transfer mode.", req.path);
  }

  return result;
}

DataStorageController::LoadDataResult DataStorageController::LoadDataFromFile(
  const std::string& filePath, const std::string& requestPath)
{
  LoadDataResult result;
  result.success = false;
  result.errorStatus = 0;

  try
  {
    result.data = IOUtil::Load(filePath);
  }
  catch (const std::exception& e)
  {
    result.errorStatus = 422;
    result.errorResponse = ErrorResponse::FileReadError(filePath, e.what(), requestPath);
    return result;
  }

  if (result.data.empty() || result.data[0].IsNull())
  {
    result.errorStatus = 415;
    result.errorResponse = ErrorResponse::UnsupportedFormat(
      "No data could be loaded from file", requestPath);
    return result;
  }

  result.success = true;
  return result;
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

  // Add path_query to meta when path filter is used (per API spec Section 6.4)
  if (params.path.has_value())
  {
    response["meta"]["path_query"] = params.path.value();
  }

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

void DataStorageController::HandlePOST_nodes_uid_generic(const httplib::Request& req, httplib::Response& res, const std::optional<std::string>& parentUID)
{
  // Determine transfer mode from Content-Type
  const std::string contentType = req.has_header(HEADER_CONTENT_TYPE)
    ? req.get_header_value(HEADER_CONTENT_TYPE)
    : "";

  const bool isJsonMode = (contentType.find(MIME_APPLICATION_JSON) != std::string::npos);
  const bool isDirectTransfer = (contentType.find(MIME_APPLICATION_OCTET_STREAM) != std::string::npos);

  nlohmann::json nodeData;
  bool hasDataTransfer = false;
  std::string dataFilePath;
  bool deleteTempFile = false;

  if (isJsonMode)
  {
    // JSON body mode
    try
    {
      if (!req.body.empty())
      {
        nodeData = nlohmann::json::parse(req.body);
      }
    }
    catch (const nlohmann::json::parse_error& e)
    {
      this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest(
        "Invalid JSON: " + std::string(e.what()), req.path));
      return;
    }

    // Validate transfer mode if specified
    if (nodeData.contains(JSON_KEY_TRANSFER) && nodeData[JSON_KEY_TRANSFER].contains(JSON_KEY_MODE))
    {
      const std::string specifiedMode = nodeData[JSON_KEY_TRANSFER][JSON_KEY_MODE].get<std::string>();
      if (specifiedMode != TRANSFER_MODE_DIRECT && specifiedMode != TRANSFER_MODE_FILE_REFERENCE)
      {
        this->SendErrorResponse(res, 406, ErrorResponse::TransferModeNotAvailable(
          specifiedMode, {TRANSFER_MODE_FILE_REFERENCE, TRANSFER_MODE_DIRECT}, req.path));
        return;
      }
    }

    // Check if there's a transfer section
    if (nodeData.contains(JSON_KEY_TRANSFER) && nodeData[JSON_KEY_TRANSFER].contains(JSON_KEY_FILE_PATH))
    {
      hasDataTransfer = true;
    }
  }
  else if (isDirectTransfer)
  {
    // Binary body mode: name from query parameter
    if (req.has_param(JSON_KEY_NAME))
    {
      nodeData[JSON_KEY_NAME] = req.get_param_value(JSON_KEY_NAME);
    }
    hasDataTransfer = true;
  }
  else if (!contentType.empty())
  {
    this->SendErrorResponse(res, 415, ErrorResponse::UnsupportedFormat(
      "Unsupported Content-Type. Use 'application/json' or 'application/octet-stream'.", req.path));
    return;
  }
  // If no Content-Type and no body, create node with defaults only

  // Resolve data path if transfer is requested
  if (hasDataTransfer)
  {
    auto pathResult = this->ResolveDataPath(req, contentType);
    if (!pathResult.success)
    {
      this->SendErrorResponse(res, pathResult.errorStatus, pathResult.errorResponse);
      return;
    }
    dataFilePath = pathResult.filePath;
    deleteTempFile = pathResult.isTemporary;
  }

  // Helper for cleanup
  auto cleanupTempFile = [&]() {
    if (deleteTempFile && !dataFilePath.empty())
    {
      try { fs::remove(dataFilePath); }
      catch (const std::exception&) { /* ignore */ }
    }
    };

  // Create the node (without data initially)
  nlohmann::json nodeDataForCreation = nodeData;
  if (nodeDataForCreation.is_object() && nodeDataForCreation.contains(JSON_KEY_TRANSFER))
  {
    nodeDataForCreation.erase(JSON_KEY_TRANSFER);
  }

  auto createResult = m_Bridge.CreateNode(nodeDataForCreation, parentUID);
  if (!createResult.success)
  {
    cleanupTempFile();
    if (parentUID.has_value())
    {
      this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to create child node", req.path));
    }
    else
    if (parentUID.has_value())
    {
      this->SendErrorResponse(res, 500, ErrorResponse::InternalError("Failed to create node", req.path));
    }
    return;
  }

  // Load and assign data if transfer was requested
  std::string dataWarning;
  if (hasDataTransfer)
  {
    auto loadResult = this->LoadDataFromFile(dataFilePath, req.path);
    if (!loadResult.success)
    {
      m_Bridge.DeleteNode(createResult.uid, false);
      cleanupTempFile();
      this->SendErrorResponse(res, loadResult.errorStatus, loadResult.errorResponse);
      return;
    }

    if (!m_Bridge.SetNodeData(createResult.uid, loadResult.data[0]))
    {
      m_Bridge.DeleteNode(createResult.uid, false);
      cleanupTempFile();
      this->SendErrorResponse(res, 500, ErrorResponse::InternalError(
        "Failed to assign data to node", req.path));
      return;
    }

    if (loadResult.data.size() > 1)
    {
      dataWarning = "File contained " + std::to_string(loadResult.data.size()) +
        " data objects. Only the first one was assigned to the node. " +
        std::to_string(loadResult.data.size() - 1) + " data object(s) were discarded.";
    }

    cleanupTempFile();
  }

  // Get the created node details
  auto node = m_Bridge.GetNode(createResult.uid);

  nlohmann::json response;
  response[JSON_KEY_DATA] = node.value();
  response[JSON_KEY_META]["location"] = "/api/v1/datastorage/nodes/" + createResult.uid;

  if (!createResult.failedProperties.empty())
  {
    response[JSON_KEY_META]["failed_properties"] = createResult.failedProperties;
  }

  if (!dataWarning.empty())
  {
    response[JSON_KEY_META][JSON_KEY_WARNING] = dataWarning;
  }

  this->SendJsonResponse(res, 201, response);
}

void DataStorageController::HandlePOST_nodes(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  this->HandlePOST_nodes_uid_generic(req, res, std::nullopt);
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
  if (!updatedNode.has_value())
  {
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError(
      "Node was updated but could not be retrieved", req.path));
    return;
  }

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

  // Add path_query to meta when path filter is used (per API spec Section 6.4)
  if (params.path.has_value())
  {
    response["meta"]["path_query"] = params.path.value();
  }

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

  const std::string parentUid = req.path_params.at("uid");

  // Check if parent node exists
  auto parentNode = m_Bridge.GetNode(parentUid);
  if (!parentNode.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(parentUid, req.path));
    return;
  }

  this->HandlePOST_nodes_uid_generic(req, res, parentUid);
}

void DataStorageController::HandleGET_nodes_uid_data(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  const std::string uid = req.path_params.at("uid");

  // Get a clone of the node's data for thread-safe serialization
  // The result distinguishes between "node not found" and "node has no data"
  const auto dataResult = m_Bridge.GetNodeData(uid);

  if (!dataResult.nodeFound)
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  if (dataResult.data.IsNull())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NoData(uid, req.path));
    return;
  }

  // Use the cloned data for all subsequent operations
  const auto* const baseData = dataResult.data.GetPointer();

  // Get node info for the filename hint (we know node exists at this point)
  const auto nodeJson = m_Bridge.GetNode(uid);

  // Determine transfer mode
  const std::string transferMode = this->DetermineTransferMode(req);

  // Find appropriate serializer using ITK ObjectFactory
  const std::string serializerName = std::string(baseData->GetNameOfClass()) + "Serializer";
  const auto instances = itk::ObjectFactoryBase::CreateAllInstance(serializerName.c_str());

  if (instances.empty())
  {
    this->SendErrorResponse(res, 415, ErrorResponse::UnsupportedFormat(
      "No serializer found for data type: " + std::string(baseData->GetNameOfClass()), req.path));
    return;
  }

  // Find the first valid serializer
  mitk::BaseDataSerializer* serializer = nullptr;
  for (auto& instance : instances)
  {
    serializer = dynamic_cast<mitk::BaseDataSerializer*>(instance.GetPointer());
    if (serializer != nullptr)
    {
      break;
    }
  }

  if (serializer == nullptr)
  {
    this->SendErrorResponse(res, 415, ErrorResponse::UnsupportedFormat(
      "No valid serializer found for data type: " + std::string(baseData->GetNameOfClass()), req.path));
    return;
  }

  // Create a per-request temp subdirectory for this serialization
  // This ensures all files created by the serializer are in one place
  fs::path requestTempDir;
  try
  {
    requestTempDir = fs::path(IOUtil::CreateTemporaryDirectory("data_XXXXXX", m_TempDirectory));
  }
  catch (const std::exception& e)
  {
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError(
      "Failed to create temp directory: " + std::string(e.what()), req.path));
    return;
  }

  // Configure serializer
  serializer->SetData(baseData);

  // Use node name as filename hint (from the JSON response)
  const std::string nodeName = "data_"+ uid;
  serializer->SetFilenameHint(nodeName);
  serializer->SetWorkingDirectory(requestTempDir.string());

  // Serialize
  std::string writtenFilename;
  try
  {
    writtenFilename = serializer->Serialize();
  }
  catch (const std::exception& e)
  {
    // Clean up the per-request temp directory on error
    fs::remove_all(requestTempDir);
    this->SendErrorResponse(res, 500, ErrorResponse::SerializationError(
      "Serialization failed: " + std::string(e.what()), req.path));
    return;
  }

  if (writtenFilename.empty())
  {
    fs::remove_all(requestTempDir);
    this->SendErrorResponse(res, 500, ErrorResponse::SerializationError(
      "Serialization returned empty filename", req.path));
    return;
  }

  // Build full path to serialized file using std::filesystem for OS independence
  const fs::path fullPath = requestTempDir / writtenFilename;

  if (transferMode == TRANSFER_MODE_FILE_REFERENCE)
  {
    try
    {
      // Return JSON response with file path
      // The per-request temp directory persists until server stop
      // or client explicitly cleans it up
      // Note: format is not included as it can be derived from file extension in file_path
      nlohmann::json response;
      response[JSON_KEY_TRANSFER][JSON_KEY_MODE] = TRANSFER_MODE_FILE_REFERENCE;

      // Get file size using std::filesystem
      if (fs::exists(fullPath))
      {
        response[JSON_KEY_TRANSFER][JSON_KEY_SIZE_BYTES] = static_cast<int64_t>(fs::file_size(fullPath));
      }
      response[JSON_KEY_TRANSFER][JSON_KEY_FILE_PATH] = fullPath.string();

      // Include the directory path so client knows where all related files are
      response[JSON_KEY_TRANSFER][JSON_KEY_DIRECTORY_PATH] = requestTempDir.string();

      // Add data metadata
      response[JSON_KEY_DATA_METADATA] = this->BuildDataMetadata(baseData);

      this->SendJsonResponse(res, 200, response);
    }
    catch (const std::exception& e)
    {
      fs::remove_all(requestTempDir);
      this->SendErrorResponse(res, 500, ErrorResponse::InternalError(
        "Failed to build file-reference response: " + std::string(e.what()), req.path));
    }
  }
  else // direct mode
  {
    try
    {
      // Read file and return as binary
      std::ifstream file(fullPath, std::ios::binary);
      if (!file.is_open())
      {
        fs::remove_all(requestTempDir);
        this->SendErrorResponse(res, 500, ErrorResponse::SerializationError(
          "Failed to read serialized file", req.path));
        return;
      }

      std::ostringstream contentStream;
      contentStream << file.rdbuf();
      const std::string content = contentStream.str();
      file.close();

      // For direct mode, we can clean up the per-request temp directory immediately
      // since we've read all the data into memory
      fs::remove_all(requestTempDir);

      this->SendBinaryResponse(res, content, writtenFilename);
    }
    catch (const std::exception& e)
    {
      fs::remove_all(requestTempDir);
      this->SendErrorResponse(res, 500, ErrorResponse::InternalError(
        "Failed to read and send data: " + std::string(e.what()), req.path));
    }
  }
}

void DataStorageController::HandlePUT_nodes_uid_data(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    this->SendErrorResponse(res, 503, ErrorResponse::DataStorageNotAvailable(req.path));
    return;
  }

  const std::string uid = req.path_params.at("uid");

  // Check if node exists
  const auto nodeJson = m_Bridge.GetNode(uid);
  if (!nodeJson.has_value())
  {
    this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
    return;
  }

  // Get Content-Type header
  const std::string contentType = req.has_header(HEADER_CONTENT_TYPE)
    ? req.get_header_value(HEADER_CONTENT_TYPE)
    : "";

  // Validate Content-Type
  const bool isFileReference = (contentType.find(MIME_APPLICATION_JSON) != std::string::npos);
  const bool isDirectTransfer = (contentType.find(MIME_APPLICATION_OCTET_STREAM) != std::string::npos);

  if (!isFileReference && !isDirectTransfer)
  {
    this->SendErrorResponse(res, 415, ErrorResponse::UnsupportedFormat(
      "Unsupported Content-Type. Use 'application/json' for file-reference mode or "
      "'application/octet-stream' for direct transfer mode.", req.path));
    return;
  }

  // Resolve data file path
  auto pathResult = this->ResolveDataPath(req, contentType);
  if (!pathResult.success)
  {
    this->SendErrorResponse(res, pathResult.errorStatus, pathResult.errorResponse);
    return;
  }

  // Helper for cleanup
  auto cleanupTempFile = [&]() {
    if (pathResult.isTemporary && !pathResult.filePath.empty())
    {
      try { fs::remove(pathResult.filePath); }
      catch (const std::exception&) { /* ignore */ }
    }
  };

  // Load data from file
  auto loadResult = this->LoadDataFromFile(pathResult.filePath, req.path);
  if (!loadResult.success)
  {
    cleanupTempFile();
    this->SendErrorResponse(res, loadResult.errorStatus, loadResult.errorResponse);
    return;
  }

  // Assign loaded data to node
  if (!m_Bridge.SetNodeData(uid, loadResult.data[0]))
  {
    cleanupTempFile();
    this->SendErrorResponse(res, 500, ErrorResponse::InternalError(
      "Failed to assign data to node", req.path));
    return;
  }

  cleanupTempFile();

  // Build response
  const auto updatedNodeJson = m_Bridge.GetNode(uid);

  nlohmann::json response;
  if (updatedNodeJson.has_value())
  {
    response[JSON_KEY_DATA] = updatedNodeJson.value();
  }
  else
  {
    response[JSON_KEY_DATA][JSON_KEY_UID] = uid;
  }

  if (loadResult.data.size() > 1)
  {
    response[JSON_KEY_META][JSON_KEY_WARNING] =
      "File contained " + std::to_string(loadResult.data.size()) +
      " data objects. Only the first one was assigned to the node. " +
      std::to_string(loadResult.data.size() - 1) + " data object(s) were discarded.";
  }

  this->SendJsonResponse(res, 200, response);
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
  // Per API spec: PUT /properties/{name} defaults to "node" scope
  auto params = this->ParsePropertyQueryParams(req, PropertyScope::Node);

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
  // Per API spec: DELETE /properties/{name} defaults to "node" scope
  auto params = this->ParsePropertyQueryParams(req, PropertyScope::Node);

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
  // Per API spec: PUT /properties defaults to "node" scope
  auto params = this->ParsePropertyQueryParams(req, PropertyScope::Node);

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
  // Per API spec: PATCH /properties defaults to "node" scope
  auto params = this->ParsePropertyQueryParams(req, PropertyScope::Node);

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
