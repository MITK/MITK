/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDataStorageController_h
#define mitkDataStorageController_h

#include "mitkDataStorageBridge.h"
#include "mitkNodeQueryParams.h"
#include <httplib.h>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * @brief Handles all /api/v1/datastorage/* endpoints.
   *
   * Handler method naming convention: HandleMETHOD_path_segments()
   * The method name directly reflects the route for clarity.
   *
   * Node endpoints:
   * - GET    /nodes                        -> HandleGET_nodes()
   * - POST   /nodes                        -> HandlePOST_nodes()
   * - GET    /nodes/:uid                   -> HandleGET_nodes_uid()
   * - PATCH  /nodes/:uid                   -> HandlePATCH_nodes_uid()
   * - DELETE /nodes/:uid                   -> HandleDELETE_nodes_uid()
   * - GET    /nodes/:uid/children          -> HandleGET_nodes_uid_children()
   * - POST   /nodes/:uid/children          -> HandlePOST_nodes_uid_children()
   *
   * Data endpoints (on nodes):
   * - GET    /nodes/:uid/data              -> HandleGET_nodes_uid_data()
   * - PUT    /nodes/:uid/data              -> HandlePUT_nodes_uid_data()
   *
   * Property endpoints (on nodes):
   * - GET    /nodes/:uid/properties        -> HandleGET_nodes_uid_properties()
   * - GET    /nodes/:uid/properties/:key   -> HandleGET_nodes_uid_properties_key()
   * - PUT    /nodes/:uid/properties/:key   -> HandlePUT_nodes_uid_properties_key()
   * - DELETE /nodes/:uid/properties/:key   -> HandleDELETE_nodes_uid_properties_key()
   * - PATCH  /nodes/:uid/properties        -> HandlePATCH_nodes_uid_properties()
   */
  class MITKRESTAPI_EXPORT DataStorageController
  {
  public:
    /**
     * @brief Construct a DataStorageController.
     *
     * @param bridge Reference to the DataStorageBridge for data operations.
     */
    explicit DataStorageController(DataStorageBridge& bridge);

    /**
     * @brief Set the temporary directory for data serialization.
     *
     * This directory is used for storing serialized data files when
     * responding to GET /nodes/{uid}/data requests.
     *
     * @param tempDir Path to the temporary directory.
     */
    void SetTempDirectory(const std::string& tempDir);

    // Node operations

    /**
     * @brief Handle GET /datastorage/nodes request.
     *
     * Lists all nodes, optionally filtered.
     */
    void HandleGET_nodes(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle POST /datastorage/nodes request.
     *
     * Creates a new node.
     */
    void HandlePOST_nodes(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle GET /datastorage/nodes/:uid request.
     *
     * Gets a single node by UID.
     */
    void HandleGET_nodes_uid(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle PATCH /datastorage/nodes/:uid request.
     *
     * Updates a node (e.g., reparenting).
     */
    void HandlePATCH_nodes_uid(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle DELETE /datastorage/nodes/:uid request.
     *
     * Deletes a node.
     */
    void HandleDELETE_nodes_uid(const httplib::Request& req, httplib::Response& res);

    // Children operations

    /**
     * @brief Handle GET /datastorage/nodes/:uid/children request.
     *
     * Lists child nodes of a given node.
     */
    void HandleGET_nodes_uid_children(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle POST /datastorage/nodes/:uid/children request.
     *
     * Creates a new child node.
     */
    void HandlePOST_nodes_uid_children(const httplib::Request& req, httplib::Response& res);

    // Data operations

    /**
     * @brief Handle GET /datastorage/nodes/:uid/data request.
     *
     * Downloads the binary data of a node.
     * Supports direct mode (binary response) and file-reference mode (JSON with path).
     */
    void HandleGET_nodes_uid_data(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle PUT /datastorage/nodes/:uid/data request.
     *
     * Uploads/replaces the binary data of a node.
     * Can also set data on nodes that currently have null data.
     */
    void HandlePUT_nodes_uid_data(const httplib::Request& req, httplib::Response& res);

    // Property operations

    /**
     * @brief Handle GET /datastorage/nodes/:uid/properties request.
     *
     * Gets all properties of a node.
     */
    void HandleGET_nodes_uid_properties(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle GET /datastorage/nodes/:uid/properties/:key request.
     *
     * Gets a single property value.
     */
    void HandleGET_nodes_uid_properties_key(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle PUT /datastorage/nodes/:uid/properties/:key request.
     *
     * Sets a single property value.
     */
    void HandlePUT_nodes_uid_properties_key(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle DELETE /datastorage/nodes/:uid/properties/:key request.
     *
     * Deletes a property.
     */
    void HandleDELETE_nodes_uid_properties_key(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle PUT /datastorage/nodes/:uid/properties request.
     *
     * Replaces all properties (full replacement).
     */
    void HandlePUT_nodes_uid_properties(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle PATCH /datastorage/nodes/:uid/properties request.
     *
     * Updates multiple properties (merge semantics).
     */
    void HandlePATCH_nodes_uid_properties(const httplib::Request& req, httplib::Response& res);

  private:
    void SendJsonResponse(httplib::Response& res, int status, const nlohmann::json& body);
    void SendErrorResponse(httplib::Response& res, int status, const nlohmann::json& error);

    /**
     * @brief Send a binary response with appropriate headers.
     *
     * @param res The response object.
     * @param data The binary data.
     * @param filename The filename for Content-Disposition header (includes extension for format).
     */
    void SendBinaryResponse(httplib::Response& res, const std::string& data,
                            const std::string& filename);

    /**
     * @brief Parse query parameters for node list requests.
     */
    NodeQueryParams ParseNodeQueryParams(const httplib::Request& req) const;

    /**
     * @brief Parse query parameters for property requests.
     *
     * @param req The HTTP request.
     * @param defaultScope Default scope if not specified in request (default: All for GET, Node for PUT/PATCH).
     * @return Parsed property query parameters.
     */
    PropertyQueryParams ParsePropertyQueryParams(const httplib::Request& req, PropertyScope defaultScope = PropertyScope::All) const;

    /**
     * @brief Determine transfer mode from request headers.
     *
     * Checks X-MITK-Transfer-Mode header and Accept header.
     *
     * @param req The request.
     * @return "direct" or "file-reference".
     */
    std::string DetermineTransferMode(const httplib::Request& req) const;

    /**
     * @brief Extract filename from Content-Disposition header.
     *
     * @param req The request.
     * @return The filename, or empty string if not found.
     */
    std::string ExtractFilenameFromContentDisposition(const httplib::Request& req) const;

    /**
     * @brief Build data_metadata JSON for a BaseData object.
     *
     * @param data The data object.
     * @return JSON with type-specific metadata.
     */
    nlohmann::json BuildDataMetadata(const mitk::BaseData* data) const;

    /**
     * @brief Result of resolving a data file path from a request.
     */
    struct ResolveDataPathResult
    {
      bool success;                    ///< Whether resolution succeeded
      std::string filePath;            ///< Resolved file path
      bool isTemporary;                ///< True if file was created from direct upload (should be deleted)
      int errorStatus;                 ///< HTTP status code if failed
      nlohmann::json errorResponse;    ///< Error response JSON if failed
    };

    /**
     * @brief Resolve data file path from request body.
     *
     * Handles both direct transfer (binary body -> temp file) and
     * file-reference mode (JSON body with file_path).
     *
     * @param req The HTTP request.
     * @param contentType The Content-Type header value.
     * @return Result with file path or error information.
     */
    ResolveDataPathResult ResolveDataPath(const httplib::Request& req, const std::string& contentType);

    /**
     * @brief Result of loading data from a file.
     */
    struct LoadDataResult
    {
      bool success;                              ///< Whether loading succeeded
      std::vector<mitk::BaseData::Pointer> data; ///< Loaded data objects
      int errorStatus;                           ///< HTTP status code if failed
      nlohmann::json errorResponse;              ///< Error response JSON if failed
    };

    /**
     * @brief Load data from a file path.
     *
     * @param filePath The file path to load from.
     * @param requestPath The original request path (for error messages).
     * @return Result with loaded data or error information.
     */
    LoadDataResult LoadDataFromFile(const std::string& filePath, const std::string& requestPath);

    /**helper to avoid redundant code. Assumes that if parentUID is set, we are in "child mode".*/
    void HandlePOST_nodes_uid_generic(const httplib::Request& req, httplib::Response& res, const std::optional<std::string>& parentUID);


    DataStorageBridge& m_Bridge;
    std::string m_TempDirectory;
  };
}

#endif
