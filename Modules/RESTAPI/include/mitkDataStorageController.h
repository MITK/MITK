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
     * @brief Parse query parameters for node list requests.
     */
    NodeQueryParams ParseNodeQueryParams(const httplib::Request& req) const;

    /**
     * @brief Parse query parameters for property requests.
     */
    PropertyQueryParams ParsePropertyQueryParams(const httplib::Request& req) const;

    DataStorageBridge& m_Bridge;
  };
}

#endif
