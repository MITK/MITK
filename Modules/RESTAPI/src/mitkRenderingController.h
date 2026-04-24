/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderingController_h
#define mitkRenderingController_h

#include <mitkDataStorageBridge.h>
#include <mitkStorageThreadDispatcherBase.h>
#include <httplib.h>

#include <functional>

#include <MitkRESTAPIExports.h>

namespace mitk
{
  class RenderWindowBridge;

  /**
   * \brief Handles all /api/v1/rendering endpoints.
   *
   * All RenderingManager calls are dispatched to the main/UI thread via
   * the StorageThreadDispatcherBase. In headless/test mode (no dispatcher),
   * tasks execute directly on the calling thread.
   *
   * Endpoints:
   * - POST /rendering/update              -> HandlePOST_update()
   * - POST /rendering/reinit              -> HandlePOST_reinit()
   * - GET  /rendering/selected-position   -> HandleGET_selectedPosition()
   * - PUT  /rendering/selected-position   -> HandlePUT_selectedPosition()
   * - GET  /rendering/selected-time       -> HandleGET_selectedTime()
   * - PUT  /rendering/selected-time       -> HandlePUT_selectedTime()
   * - GET  /rendering/screenshot          -> HandleGET_screenshot()
   */
  class MITKRESTAPI_EXPORT RenderingController
  {
  public:
    /**
     * \brief Construct a RenderingController.
     *
     * \param bridge Reference to the DataStorageBridge for data access.
     */
    explicit RenderingController(DataStorageBridge& bridge);

    /**
     * \brief Set the thread dispatcher for RenderingManager operations.
     *
     * If set, all RenderingManager calls are dispatched to the storage-owning
     * (main/UI) thread. If nullptr, calls execute directly (headless/test mode).
     *
     * \param dispatcher The dispatcher, or nullptr to clear.
     */
    void SetDispatcher(StorageThreadDispatcherBase* dispatcher);

    /**
     * \brief Set or clear the RenderWindowBridge for interactions with the rendering stack of
     * the application.
     *
     * \param bridge The bridge, or nullptr to clear (screenshot endpoint returns 503).
     */
    void SetRenderWindowBridge(RenderWindowBridge* bridge);

    /**
     * \brief Handle POST /rendering/update request.
     *
     * Triggers RenderingManager::RequestUpdateAll() with optional type filter.
     * Optional body: {"type": "all" | "2d" | "3d"} (default: "all").
     *
     * \param req The HTTP request.
     * \param res The HTTP response to populate.
     */
    void HandlePOST_update(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle POST /rendering/reinit request.
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
     * \pre DataStorage must be connected (503 otherwise).
     * \pre When "uids" is given: must be a non-empty array of strings (400 otherwise).
     * \pre When "uids" is given: every UID must identify an existing node (404 on first failure).
     * \pre When "uids" is given: every node must have data with a valid TimeGeometry (422 on first failure).
     *
     * \param req The HTTP request.
     * \param res The HTTP response to populate.
     */
    void HandlePOST_reinit(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/selected-position request.
     *
     * Returns the current crosshair position and the world-space AABB
     * (i.e. the reinit geometry). If no input geometry is available,
     * bounds.min and bounds.max are null.
     *
     * Returns 503 if no render window bridge or position getter is connected.
     *
     * Response 200: {"position": [x, y, z], "bounds": {"min": [...], "max": [...]}}
     *
     * \param req The HTTP request.
     * \param res The HTTP response to populate.
     */
    void HandleGET_selectedPosition(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/selected-position request.
     *
     * Moves the global application crosshair to the given 3D world position.
     * No bounds checking is performed. Out of bounds values will be ignored.
     *
     * Required body: {"position": [x, y, z]} — exactly 3 numbers.
     *
     * Response 204 on success. Returns 503 if no render window bridge or
     * position setter is connected.
     *
     * \param req The HTTP request (must contain a JSON body with "position").
     * \param res The HTTP response to populate.
     */
    void HandlePUT_selectedPosition(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/selected-time request.
     *
     * Returns the current time step and time point from the global
     * TimeNavigationController, together with the time geometry bounds.
     *
     * Response 200: {"timepoint_ms": ..., "timestep": ..., "bounds": {...}}
     *
     * \param req The HTTP request.
     * \param res The HTTP response to populate.
     */
    void HandleGET_selectedTime(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/selected-time request.
     *
     * Sets the active time step or time point in the global TimeNavigationController.
     * Exactly one of "timepoint_ms" (number) or "timestep" (non-negative integer)
     * must be present in the body.
     * No bounds checking is performed. Out of bounds values will be ignored.
     *
     * Response 204 on success.
     *
     * \param req The HTTP request (must contain a JSON body with exactly one of the fields).
     * \param res The HTTP response to populate.
     */
    void HandlePUT_selectedTime(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors request.
     *
     * Returns the full list of known editor aliases with their current
     * activity state. Concept §3 / WP2 E1.
     *
     * Response 200: [{"alias":..., "plugin_id":..., "active":...}, ...]
     */
    void HandleGET_editors(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti request (WP2 E2).
     *
     * Returns metadata about the StdMultiWidgetEditor.
     */
    void HandleGET_stdmultiInfo(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/windows request (WP2 E4).
     *
     * Returns the list of render windows of the StdMultiWidget editor.
     */
    void HandleGET_stdmultiWindows(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/windows/{name} request (WP2 E5).
     *
     * Per-window summary. Controller-side validates {name} before bridge dispatch.
     */
    void HandleGET_stdmultiWindow(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/screenshot request.
     *
     * Captures a screenshot of the active application window.
     *
     * Query parameters:
     * - format: "png" (default) or "jpeg"
     * - width, height: optional positive integers; both must be given together.
     *
     * Response 200 with binary image body (Content-Type: image/png or image/jpeg).
     *
     * \pre RenderWindowBridge must be set and have a screenshot provider (503 otherwise).
     *
     * \param req The HTTP request.
     * \param res The HTTP response to populate.
     */
    void HandleGET_screenshot(const httplib::Request& req, httplib::Response& res) const;

  private:
    /**
     * \brief Execute a task on the dispatch thread, or directly if no dispatcher is set.
     *
     * \param task The task to execute.
     */
    void Dispatch(std::function<void()> task) const;

    void SendErrorResponse(httplib::Response& res, int status, const nlohmann::json& error) const;

    /**
     * \brief Map a bridge exception thrown by a RenderWindowBridge callback to
     *        a matching HTTP status and RFC 7807 error payload.
     *
     * Recognises the three typed bridge exceptions:
     * - RenderWindowBridgeNoEditorException           -> 503 EDITOR_NOT_ACTIVE
     * - RenderWindowBridgeUnknownWindowException      -> 404 RENDER_WINDOW_NOT_FOUND
     * - RenderWindowBridgeUnsupportedOperationException -> 404 UNSUPPORTED_OPERATION
     *
     * Any other std::exception is reported as 500 INTERNAL_ERROR.
     *
     * \param e    The caught exception.
     * \param instance The request path for the RFC 7807 "instance" field.
     * \return A pair of {HTTP status, JSON payload} ready for SendErrorResponse.
     */
    static std::pair<int, nlohmann::json> MapBridgeException(
      const std::exception& e, const std::string& instance);

    /**
     * \brief True if the given window name is a known StdMultiWidget window.
     *
     * The set is {"axial", "sagittal", "coronal", "3d"}. Used by handlers to
     * reject unknown window names at the controller layer with 404
     * RENDER_WINDOW_NOT_FOUND (before any bridge dispatch).
     */
    static bool IsValidStdMultiWindowName(const std::string& name);

    /** True if the given StdMulti window is the 3D window. */
    static bool IsStd3dWindow(const std::string& name);

    DataStorageBridge& m_Bridge;
    RenderWindowBridge* m_RenderWindowBridge = nullptr;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;
  };
}

#endif
