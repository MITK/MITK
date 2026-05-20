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
#include <mitkRenderWindowBridge.h>
#include <mitkStorageThreadDispatcherBase.h>
#include <httplib.h>

#include <functional>
#include <mutex>
#include <optional>
#include <utility>

#include <MitkRESTAPIExports.h>

namespace mitk
{

  /**
   * \brief Handles all /api/v1/rendering endpoints.
   *
   * All RenderingManager calls are dispatched to the main/UI thread via
   * the StorageThreadDispatcherBase. In headless/test mode (no dispatcher),
   * tasks execute directly on the calling thread.
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
     * Thread-safety: serialised against Dispatch() via an internal mutex, so
     * the dispatcher may be set or replaced while requests are in flight.
     *
     * \param dispatcher The dispatcher, or nullptr to clear.
     */
    void SetDispatcher(StorageThreadDispatcherBase* dispatcher);

    /**
     * \brief Set or clear the RenderWindowBridge for interactions with the rendering stack of
     * the application.
     *
     * \note Lifecycle contract: this setter is intended to be called exactly
     * once during \c RestServer::Start(), before the server thread starts
     * accepting connections. The bridge pointer is not re-bound at runtime
     * once request handlers are reachable. Because of this set-once-at-startup
     * contract, the read sites in \c HandleGET_* / \c HandlePUT_* deliberately
     * access \c m_RenderWindowBridge without holding \c m_DispatcherMutex --
     * which would otherwise add a lock to every request without protecting
     * against a race that does not exist in normal operation. Do not relax
     * this contract without also serialising the read sites.
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
     * - No body (or body without "uids"): global reinit -- fits all render windows to the
     *   bounding box of all currently visible data. Calls
     *   RenderingManager::InitializeViewsByBoundingObjects(dataStorage).
     * - Body with "uids" containing one UID: single-node reinit -- fits render windows to
     *   the bounding geometry of that node.
     * - Body with "uids" containing multiple UIDs: multi-node reinit -- fits render windows
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
     * bounds.min_position and bounds.max_position are null.
     *
     * Returns 503 if no render window bridge or position getter is connected.
     *
     * Response 200: {"position": [x, y, z], "bounds": {"min_position": [...], "max_position": [...]}}
     *
     * \param req The HTTP request.
     * \param res The HTTP response to populate.
     */
    void HandleGET_selectedPosition(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/selected-position request.
     *
     * Moves the global application crosshair to the given 3D world position.
     * No bounds checking is performed; out-of-range values are clamped/snapped by MITK.
     *
     * Required body: {"position": [x, y, z]} -- exactly 3 numbers.
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
     * No bounds checking is performed; out-of-range values may be clamped by the
     * time navigation controller.
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
     * activity state.
     *
     * Response 200: [{"alias":..., "plugin_id":..., "active":...}, ...]
     */
    void HandleGET_editors(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti request.
     *
     * Returns metadata about the StdMultiWidgetEditor.
     */
    void HandleGET_stdmultiInfo(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/windows request.
     *
     * Returns the list of render windows of the StdMultiWidget editor.
     */
    void HandleGET_stdmultiWindows(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/windows/{id} request.
     *
     * Per-window summary. Controller-side validates {id} before bridge dispatch.
     */
    void HandleGET_stdmultiWindow(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/windows/{id}/camera.
     *
     * 2D windows return `parallel_scale`, the 3D window returns `perspective_angle`.
     */
    void HandleGET_stdmultiCamera(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/editors/stdmulti/windows/{id}/camera.
     *
     * Partial update. Rejects: unknown fields, 2D-only field on 3D and vice
     * versa, unknown `standard_view` values, non-positive `parallel_scale`,
     * out-of-range `perspective_angle`, invalid JSON, empty body.
     */
    void HandlePUT_stdmultiCamera(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/windows/{id}/selected-slice.
     *
     * Returns {step, position, bounds}. Returns 404 UNSUPPORTED_OPERATION for the 3D window.
     */
    void HandleGET_stdmultiSelectedSlice(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/editors/stdmulti/windows/{id}/selected-slice.
     *
     * Body accepts only `{"step": N}`. A `position` field triggers 400 with a
     * hint pointing at /rendering/selected-position (StdMulti slices are coupled).
     * The 3D window returns 404 UNSUPPORTED_OPERATION.
     */
    void HandlePUT_stdmultiSelectedSlice(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/screenshot.
     *
     * Editor-canvas grab. Query contract identical to /rendering/screenshot.
     */
    void HandleGET_stdmultiScreenshot(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/stdmulti/windows/{id}/screenshot.
     *
     * Single-window offscreen grab. Query contract identical to /rendering/screenshot.
     */
    void HandleGET_stdmultiWindowScreenshot(const httplib::Request& req, httplib::Response& res) const;

    // ---- MxN editor handlers ----

    /** Handle GET /rendering/editors/mxn. */
    void HandleGET_mxnInfo(const httplib::Request& req, httplib::Response& res) const;

    /** Handle GET /rendering/editors/mxn/windows. */
    void HandleGET_mxnWindows(const httplib::Request& req, httplib::Response& res) const;

    /** Handle GET /rendering/editors/mxn/windows/{id}. */
    void HandleGET_mxnWindow(const httplib::Request& req, httplib::Response& res) const;

    /** Handle GET /rendering/editors/mxn/layout. */
    void HandleGET_mxnLayout(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/editors/mxn/layout.
     *
     * Applies a v2.0 layout document. Response 200 echoes the freshly
     * serialized layout (so callers don't need a follow-up GET to refresh
     * cached cell ids after a tear-down).
     *
     * Maps `mitk::Exception` thrown by the engine's ApplyLayout to 400
     * INVALID_REQUEST. This is the one site under the MxN handlers where
     * mitk::Exception does not fall through to 422 RENDERING_ERROR; the
     * assumption is that every mitk::Exception out of ApplyLayout is a
     * document-shape failure (version, schema, duplicate id, unknown view
     * direction, missing group reference, type error). If the engine ever
     * broadens ApplyLayout's failure model to runtime issues, narrow the
     * catch.
     */
    void HandlePUT_mxnLayout(const httplib::Request& req, httplib::Response& res) const;

    /** Handle GET /rendering/editors/mxn/windows/{id}/camera. */
    void HandleGET_mxnCamera(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/editors/mxn/windows/{id}/camera.
     *
     * Partial update; reuses the StdMulti ParseCameraPatch validation. Under
     * v2 the MxN cell `is3d` is hard-coded to `false`: the schema's
     * `view_direction` enum has no `3d` value. When v3 introduces a 3D MxN
     * cell type, derive `is3d` from the cell's MxNWindowInfo::kind.
     */
    void HandlePUT_mxnCamera(const httplib::Request& req, httplib::Response& res) const;

    /** Handle GET /rendering/editors/mxn/windows/{id}/selected-slice. */
    void HandleGET_mxnSelectedSlice(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/editors/mxn/windows/{id}/selected-slice.
     *
     * Step-only. Body containing `position` returns 400 with a hint pointing
     * at the per-cell selected-position resource for cell-local world anchor
     * moves and at /rendering/selected-position for global moves.
     */
    void HandlePUT_mxnSelectedSlice(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/mxn/windows/{id}/selected-position.
     *
     * Returns the per-cell 3D world anchor + scene bounds. Distinct from the
     * global /rendering/selected-position, which targets the StdMulti
     * anchor. Per-cell anchors may legitimately diverge from the global one.
     */
    void HandleGET_mxnSelectedPosition(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle PUT /rendering/editors/mxn/windows/{id}/selected-position.
     *
     * Sets the per-cell 3D anchor via
     * QmitkAbstractMultiWidget::SetSelectedPosition(point, widgetName). Whether
     * the change propagates to other cells / the global anchor depends on the
     * workbench's interactive coupling toggle, which is not exposed via REST.
     * No range checking; out-of-range values are clamped/snapped by MITK.
     */
    void HandlePUT_mxnSelectedPosition(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/mxn/screenshot.
     *
     * Captures the MxN multi-widget editor canvas (multi-widget area only -
     * no toolbars, no side panels). Query and response contract identical to
     * GET /rendering/screenshot.
     */
    void HandleGET_mxnScreenshot(const httplib::Request& req, httplib::Response& res) const;

    /**
     * \brief Handle GET /rendering/editors/mxn/windows/{id}/screenshot.
     *
     * Single MxN cell viewport capture without resizing the live render
     * surface. Query and response contract identical to
     * GET /rendering/screenshot.
     */
    void HandleGET_mxnWindowScreenshot(const httplib::Request& req, httplib::Response& res) const;

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
     * Recognises the four typed bridge exceptions:
     * - RenderWindowBridgeNoEditorException              -> 503 EDITOR_NOT_ACTIVE
     * - RenderWindowBridgeUnknownWindowException         -> 404 RENDER_WINDOW_NOT_FOUND
     * - RenderWindowBridgeUnsupportedOperationException  -> 404 UNSUPPORTED_OPERATION
     * - RenderWindowBridgeRendererUnavailableException   -> 500 RENDERER_UNAVAILABLE
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
     * \brief True if the given window id is a known StdMultiWidget window.
     *
     * The set is {"axial", "sagittal", "coronal", "3d"}. Used by handlers to
     * reject unknown window ids at the controller layer with 404
     * RENDER_WINDOW_NOT_FOUND (before any bridge dispatch).
     */
    static bool IsValidStdMultiWindowId(const std::string& id);

    /** True if the given StdMulti window is the 3D window. */
    static bool IsStdMulti3dWindow(const std::string& id);

    /**
     * \brief Read a required path parameter, defaulting to the empty string.
     *
     * The httplib route pattern marks the parameter as mandatory, so an
     * absent entry cannot reach a handler in normal operation; the empty
     * fallback exists solely to keep the call sites total.
     */
    static std::string ReadRequiredPathParam(const httplib::Request& req,
                                             const std::string& key);

    DataStorageBridge& m_Bridge;
    RenderWindowBridge* m_RenderWindowBridge = nullptr;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;
    mutable std::mutex m_DispatcherMutex;
  };
}

#endif
