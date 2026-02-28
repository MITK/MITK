/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderingController_h
#define mitkRenderingController_h

#include "mitkDataStorageBridge.h"
#include <mitkStorageThreadDispatcherBase.h>
#include <httplib.h>

#include <functional>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  /**
   * @brief Handles all /api/v1/rendering endpoints.
   *
   * All RenderingManager calls are dispatched to the main/UI thread via
   * the StorageThreadDispatcherBase. In headless/test mode (no dispatcher),
   * tasks execute directly on the calling thread.
   *
   * Endpoints:
   * - POST /rendering/update   -> HandlePOST_update()
   * - POST /rendering/reinit   -> HandlePOST_reinit()
   */
  class MITKRESTAPI_EXPORT RenderingController
  {
  public:
    /**
     * @brief Construct a RenderingController.
     *
     * @param bridge Reference to the DataStorageBridge for data access.
     */
    explicit RenderingController(DataStorageBridge& bridge);

    /**
     * @brief Set the thread dispatcher for RenderingManager operations.
     *
     * If set, all RenderingManager calls are dispatched to the storage-owning
     * (main/UI) thread. If nullptr, calls execute directly (headless/test mode).
     *
     * @param dispatcher The dispatcher, or nullptr to clear.
     */
    void SetDispatcher(StorageThreadDispatcherBase* dispatcher);

    /**
     * @brief Handle POST /rendering/update request.
     *
     * Triggers RenderingManager::RequestUpdateAll() with optional type filter.
     * Optional body: {"type": "all" | "2d" | "3d"} (default: "all").
     *
     * @param req The HTTP request.
     * @param res The HTTP response to populate.
     */
    void HandlePOST_update(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief Handle POST /rendering/reinit request.
     *
     * Three operating modes:
     * - No body (or body without "uids"): global reinit — fits all render windows to the
     *   bounding box of all currently visible data. Calls
     *   RenderingManager::InitializeViewsByBoundingObjects(dataStorage).
     * - Body with "uids" containing one UID: single-node reinit — fits render windows to
     *   the bounding geometry of that node.
     * - Body with "uids" containing multiple UIDs: multi-node reinit — fits render windows
     *   to the combined bounding geometry of all listed nodes.
     *   Both node cases call RenderingManager::InitializeViews(geometry, REQUEST_UPDATE_ALL, true).
     *
     * @pre DataStorage must be connected (503 otherwise).
     * @pre When "uids" is given: must be a non-empty array of strings (400 otherwise).
     * @pre When "uids" is given: every UID must identify an existing node (404 on first failure).
     * @pre When "uids" is given: every node must have data with a valid TimeGeometry (422 on first failure).
     *
     * @param req The HTTP request.
     * @param res The HTTP response to populate.
     */
    void HandlePOST_reinit(const httplib::Request& req, httplib::Response& res);

  private:
    /**
     * @brief Execute a task on the dispatch thread, or directly if no dispatcher is set.
     *
     * @param task The task to execute.
     */
    void Dispatch(std::function<void()> task) const;

    void SendErrorResponse(httplib::Response& res, int status, const nlohmann::json& error);

    DataStorageBridge& m_Bridge;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;
  };
}

#endif
