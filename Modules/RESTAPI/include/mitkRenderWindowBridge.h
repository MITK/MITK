/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRenderWindowBridge_h
#define mitkRenderWindowBridge_h

#include <MitkRESTAPIExports.h>
#include <mitkAnatomicalPlanes.h>
#include <mitkPoint.h>
#include <mitkStorageThreadDispatcherBase.h>
#include <mitkVector.h>
#include <mitkWeakPointer.h>

#include <functional>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace mitk
{
  /**
   * \brief Format for screenshot capture.
   */
  enum class ScreenshotFormat
  {
    Png,
    Jpeg
  };

  /** Axis-aligned world bounding box.
   *
   *  Field names mirror SliceBounds (and the JSON wire form) so every
   *  bounds-shaped object the bridge exposes uses the same vocabulary:
   *  the extremes are world-coordinate positions, not opaque min/max
   *  scalars.
   */
  struct WorldBounds
  {
    Point3D minPosition;
    Point3D maxPosition;

    WorldBounds() : minPosition(), maxPosition() {}
  };

  /**
   * \brief Position and world-bounds snapshot read atomically on the UI thread. */
  struct SelectedPositionInfo
  {
    /** Current crosshair position in world coordinates. */
    Point3D position;

    /** World bounding box, if available at read time. */
    std::optional<WorldBounds> bounds;

    SelectedPositionInfo() : position() {}
  };

  /**
   * \brief Thrown by bridge callbacks when the target editor instance is not
   *        open in the workbench.
   *
   * Mapped by the controller layer to HTTP 503 EDITOR_NOT_ACTIVE.
   */
  class MITKRESTAPI_EXPORT RenderWindowBridgeNoEditorException : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  /**
   * \brief Thrown by bridge callbacks when the window id is not known.
   *
   * Mapped by the controller layer to HTTP 404 RENDER_WINDOW_NOT_FOUND.
   */
  class MITKRESTAPI_EXPORT RenderWindowBridgeUnknownWindowException : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  /**
   * \brief Thrown by bridge callbacks when the requested sub-resource does not
   *        apply to the addressed window (e.g. selected-slice on a 3D window).
   *
   * Mapped by the controller layer to HTTP 404 UNSUPPORTED_OPERATION.
   */
  class MITKRESTAPI_EXPORT RenderWindowBridgeUnsupportedOperationException : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  /**
   * \brief Thrown by bridge callbacks when a window exists but its renderer is
   *        currently unavailable (server-side state inconsistency).
   *
   * Mapped by the controller layer to HTTP 500 RENDERER_UNAVAILABLE.
   */
  class MITKRESTAPI_EXPORT RenderWindowBridgeRendererUnavailableException : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  /** Summary of a known editor alias and its current activity state. */
  struct EditorInfo
  {
    std::string alias;
    std::string pluginId;
    bool active = false;
    // Window identifiers exposed by the editor. For MxN, this is the
    // canonical fully-qualified `id` field of each window leaf in the v2
    // layout document (e.g. `mxn__widget0`): the same string used as the
    // URL path segment for sub-resources. For StdMulti, the engine-fixed
    // names (axial / sagittal / coronal / 3d). Empty when !active.
    std::vector<std::string> windowIds;
  };

  /**
   * \brief Closed set of render window kinds reported by the bridge.
   *
   * Drives which sub-resources apply (e.g. selected-slice is only valid for
   * TwoD windows). Adding a new value here must also extend WindowKindToString.
   */
  enum class WindowKind
  {
    TwoD,
    ThreeD
  };

  /** \return Wire/JSON form of a WindowKind ("2d" or "3d"). */
  MITKRESTAPI_EXPORT const char* WindowKindToString(WindowKind kind);

  /**
   * \return Wire/JSON form of an AnatomicalPlane as used in the v2 layout
   *         schema (lower-case: "axial" / "sagittal" / "coronal" / "original").
   */
  MITKRESTAPI_EXPORT const char* AnatomicalPlaneToV2String(AnatomicalPlane plane);

  /**
   * \brief Parse the v2 layout schema's view_direction string.
   *
   * \throws std::invalid_argument if \p s is not one of the four accepted
   *         lower-case tokens.
   */
  MITKRESTAPI_EXPORT AnatomicalPlane ParseV2ViewDirection(const std::string& s);

  /** Summary of a single render window exposed by an editor.
   *
   *  \c id is the URL-segment identifier of the window -- for StdMulti the
   *  engine-fixed slot tokens "axial" / "sagittal" / "coronal" / "3d";
   *  for MxN-derived editors the canonical fully-qualified cell id. It
   *  mirrors \c MxNWindowInfo::id so generic clients see the same
   *  identity field name across editor types.
   *
   *  \c viewDirection is populated for 2D windows (axial / sagittal /
   *  coronal); the 3D window leaves it as \c std::nullopt. The field is
   *  redundant with \c id under the current StdMulti definition (where
   *  the slot id equals the plane), but it puts the StdMulti and MxN
   *  window summaries on the same shape so generic clients can read
   *  \c view_direction uniformly across editor types without parsing
   *  StdMulti's slot id.
   */
  struct WindowInfo
  {
    std::string                    id; // "axial" | "sagittal" | "coronal" | "3d"
    WindowKind                     kind = WindowKind::TwoD;
    std::optional<AnatomicalPlane> viewDirection;
  };

  /**
   * \brief Summary of a single MxN cell exposed via the REST window list.
   *
   * Carries the per-cell layout-document fields that REST clients need
   * without forcing them to fetch the full layout: identity (id), optional
   * display label, kind ("2d" / "3d"), v2 view direction, and
   * selection-group label.
   *
   * Distinct from \c WindowInfo because MxN cells carry persisted
   * layout state (\c viewDirection, \c selectionGroup) that does not
   * apply to StdMulti windows. Future v3 dimensions add fields here
   * additively.
   *
   * Identity vs. display label: \c id mirrors the v2 schema's required
   * \c id field -- the canonical fully-qualified window name
   * (`<editor_name>__<bare>`), URL-segment-safe, unique within the layout
   * document, used verbatim as the URL path segment for sub-resources.
   * \c displayName mirrors the optional \c name field -- a free-form
   * human-readable label, \c std::nullopt when absent.
   *
   * \c viewDirection mirrors the v2 \c view_direction field. Under v2
   * every MxN cell is 2D and the layout schema requires a value drawn
   * from the closed enum {axial, sagittal, coronal, original}, so the
   * optional always carries a value in v2. The \c std::optional wrapper
   * is reserved purely for forward-compatibility with v3, which will
   * introduce 3D cells (\c kind == ThreeD) for which no anatomical plane
   * applies.
   */
  struct MxNWindowInfo
  {
    std::string                    id;             // canonical fully-qualified window id
    std::optional<std::string>     displayName;    // human-readable label; nullopt when absent
    WindowKind                     kind = WindowKind::TwoD;
    std::optional<AnatomicalPlane> viewDirection;  // always set for 2D cells under v2; nullopt reserved for v3 3D cells. Mapped to v2 strings via AnatomicalPlaneToV2String when present.
    std::string                    selectionGroup; // links.selection group label
  };

  /** Camera state of a single render window. */
  struct CameraState
  {
    Point3D position;
    Point3D focalPoint;
    Vector3D viewUp;
    std::optional<double> parallelScale;    // 2D windows only
    std::optional<double> perspectiveAngle; // 3D windows only

    CameraState() : position(), focalPoint(), viewUp() {}
  };

  /** Partial camera update. */
  struct CameraPatch
  {
    std::optional<Point3D>     position;
    std::optional<Point3D>     focalPoint;
    std::optional<Vector3D>    viewUp;
    std::optional<double>      parallelScale;
    std::optional<double>      perspectiveAngle;
    std::optional<std::string> standardView; // applied first if present
  };

  /** Bounds for a slice navigator. */
  struct SliceBounds
  {
    unsigned int steps = 0;
    Point3D minPosition;
    Point3D maxPosition;
    bool hasPositions = false; // false if no geometry loaded

    SliceBounds() : minPosition(), maxPosition() {}
  };

  /** Slice state of a 2D render window. */
  struct SliceState
  {
    unsigned int step = 0;
    Point3D position;
    SliceBounds bounds;

    SliceState() : position() {}
  };

  /**
   * \brief Bridge between the REST API server and the Qt/render-window layer.
   *
   * This class abstracts render-window capabilities so that the REST API module
   * remains free of Qt and Berry dependencies. All callbacks are injected at
   * runtime by the Qt plugin (org.mitk.gui.qt.restapi), which owns and
   * configures the server-owned bridge instance.
   *
   * Thread dispatching: all three dispatch functions copy the callback and
   * dispatcher under the mutex, release it, then invoke the callback (optionally
   * on the UI thread via the dispatcher). This keeps the mutex free during
   * callback execution and avoids any risk of deadlock from re-entry.
   *
   * Thread-safety: All public methods are thread-safe.
   */
  class MITKRESTAPI_EXPORT RenderWindowBridge
  {
  public:
    /**
     * \brief Callback type for taking a screenshot.
     *
     * \param size Optional output size (width, height). Both dimensions must be
     *             provided together; nullopt means native resolution.
     * \param format Desired image encoding (Png or Jpeg).
     * \note Exceptions thrown by the callback are caught by the bridge and
     *       transported to the REST thread -- they never reach the UI event loop.
     * \return Encoded image bytes.
     * \throws std::exception on failure.
     */
    using ScreenshotProvider =
      std::function<std::vector<unsigned char>(
        std::optional<std::pair<int, int>> size,
        ScreenshotFormat format)>;

    /**
     * \brief Callback type for reading the current global crosshair position and scene bounds.
     *
     * Both pieces of data are read atomically within one callback invocation so
     * that position and bounds always correspond to the same UI-thread snapshot.
     *
     * \pre Must be called on the UI thread.
     * \pre Must not re-enter RenderWindowBridge methods.
     * \note Exceptions thrown by the callback are caught by the bridge and
     *       transported to the REST thread -- they never reach the UI event loop.
     * \return SelectedPositionInfo containing position and optional bounds.
     * \throws std::exception on failure.
     */
    using PositionGetter = std::function<SelectedPositionInfo()>;

    /**
     * \brief Callback type for setting the global crosshair position.
     *
     * \pre Must be called on the UI thread.
     * \pre Must not re-enter RenderWindowBridge methods.
     * \note Exceptions thrown by the callback are caught by the bridge and
     *       transported to the REST thread -- they never reach the UI event loop.
     * \throws std::exception on failure.
     */
    using PositionSetter = std::function<void(const Point3D& pos)>;

    /** Callback type: list all known editor aliases with their activity state. */
    using EditorListProvider = std::function<std::vector<EditorInfo>()>;

    /** Callback type: list the windows of the StdMultiWidget editor. */
    using StdMultiWindowListProvider = std::function<std::vector<WindowInfo>()>;

    /** Callback type: grab the StdMultiWidget editor canvas. */
    using StdMultiEditorScreenshotProvider =
      std::function<std::vector<unsigned char>(
        std::optional<std::pair<int, int>> size,
        ScreenshotFormat format)>;

    /** Callback type: grab a single StdMulti render window. */
    using StdMultiWindowScreenshotProvider =
      std::function<std::vector<unsigned char>(
        const std::string& windowId,
        std::optional<std::pair<int, int>> size,
        ScreenshotFormat format)>;

    /** Callback type: read camera state of a StdMulti render window. */
    using StdMultiCameraGetter = std::function<CameraState(const std::string& windowId)>;

    /** Callback type: apply a camera patch on a StdMulti render window. */
    using StdMultiCameraSetter = std::function<void(const std::string& windowId, const CameraPatch& patch)>;

    /** Callback type: read the selected-slice state of a StdMulti 2D window. */
    using StdMultiSelectedSliceGetter = std::function<SliceState(const std::string& windowId)>;

    /**
     * \brief Callback type: set the selected slice of a StdMulti 2D window by step index.
     *
     * StdMulti only supports step-based slice selection; world-position
     * addressing is reserved for /rendering/selected-position. If MxN later
     * needs position-based addressing, add a separate setter alias for it.
     */
    using StdMultiSelectedSliceStepSetter = std::function<void(const std::string& windowId, unsigned int step)>;

    /** Callback type: list the windows of the MxN multi-widget editor. */
    using MxNWindowListProvider = std::function<std::vector<MxNWindowInfo>()>;

    /** Callback type: grab the MxN editor canvas. */
    using MxNEditorScreenshotProvider =
      std::function<std::vector<unsigned char>(
        std::optional<std::pair<int, int>> size,
        ScreenshotFormat format)>;

    /** Callback type: grab a single MxN cell's render window. */
    using MxNWindowScreenshotProvider =
      std::function<std::vector<unsigned char>(
        const std::string& windowId,
        std::optional<std::pair<int, int>> size,
        ScreenshotFormat format)>;

    /** Callback type: read camera state of an MxN cell. */
    using MxNCameraGetter = std::function<CameraState(const std::string& windowId)>;

    /** Callback type: apply a camera patch on an MxN cell. */
    using MxNCameraSetter = std::function<void(const std::string& windowId, const CameraPatch& patch)>;

    /** Callback type: read the selected-slice state of a 2D MxN cell. */
    using MxNSelectedSliceGetter = std::function<SliceState(const std::string& windowId)>;

    /**
     * \brief Callback type: set the selected slice of a 2D MxN cell by step index.
     *
     * Position-based slice navigation is intentionally not exposed on the
     * slice resource; per-cell world anchors live on the dedicated
     * selected-position resource (see \c MxNSelectedPositionSetter).
     */
    using MxNSelectedSliceStepSetter = std::function<void(const std::string& windowId, unsigned int step)>;

    /**
     * \brief Callback type: read the per-cell selected position (cell anchor)
     *        plus scene bounds atomically.
     *
     * Mirrors the global \c PositionGetter shape but window-id-keyed. The
     * returned bounds are the global scene AABB at the active timestep
     * (same as the global getter); both pieces of data are read in one
     * UI-thread snapshot.
     */
    using MxNSelectedPositionGetter =
      std::function<SelectedPositionInfo(const std::string& windowId)>;

    /** Callback type: set the per-cell selected position (cell anchor). */
    using MxNSelectedPositionSetter =
      std::function<void(const std::string& windowId, const Point3D& position)>;

    /**
     * \brief Callback type: serialize the editor's current layout to a v2.0
     *        JSON document.
     *
     * The bridge boundary uses \c std::string to keep the bridge header
     * free of \c nlohmann/json.hpp. The plugin serializes via
     * \c QmitkMxNMultiWidget::SerializeLayout().dump().
     */
    using MxNLayoutGetter = std::function<std::string()>;

    /**
     * \brief Callback type: apply a v2.0 JSON layout document and return the
     *        freshly serialized layout (so callers can echo the new state in
     *        a PUT response without a follow-up GET).
     *
     * The \c std::string boundary keeps \c nlohmann/json.hpp out of the bridge
     * header. Schema / structural validation throws \c mitk::Exception from
     * \c QmitkMxNMultiWidget::ApplyLayout; the controller maps that to 400
     * INVALID_REQUEST locally (this is the one site where \c mitk::Exception
     * does not become 422 RENDERING_ERROR).
     */
    using MxNLayoutSetter = std::function<std::string(const std::string& layoutJson)>;

    /**
     * \brief Set the thread dispatcher for UI thread dispatching.
     *
     * If set, all callbacks are dispatched to the UI thread before invocation.
     * If nullptr, callbacks execute directly on the calling thread.
     *
     * \param dispatcher The dispatcher, or nullptr to clear.
     */
    void SetDispatcher(StorageThreadDispatcherBase* dispatcher);

    /**
     * \brief Set or clear the screenshot provider callback.
     *
     * \param provider The callback to use, or an empty function to clear.
     */
    void SetScreenshotProvider(ScreenshotProvider provider);

    /**
     * \brief Set or clear the position getter callback.
     *
     * Should read the global crosshair position from the StdMultiWidgetEditor
     * via IRenderWindowPart::GetSelectedPosition().
     *
     * \param getter Callback to read current crosshair position, or empty to clear.
     */
    void SetPositionGetter(PositionGetter getter);

    /**
     * \brief Set or clear the position setter callback.
     *
     * Should update the global crosshair position via
     * IRenderWindowPart::SetSelectedPosition() on the StdMultiWidgetEditor.
     *
     * \param setter Callback to move crosshair to a position, or empty to clear.
     */
    void SetPositionSetter(PositionSetter setter);

    /** Set or clear the editor list provider. */
    void SetEditorListProvider(EditorListProvider provider);

    /** Set or clear the StdMulti window list provider. */
    void SetStdMultiWindowListProvider(StdMultiWindowListProvider provider);

    /** Set or clear the StdMulti editor screenshot provider. */
    void SetStdMultiEditorScreenshotProvider(StdMultiEditorScreenshotProvider provider);

    /** Set or clear the StdMulti per-window screenshot provider. */
    void SetStdMultiWindowScreenshotProvider(StdMultiWindowScreenshotProvider provider);

    /** Set or clear the StdMulti camera getter. */
    void SetStdMultiCameraGetter(StdMultiCameraGetter getter);

    /** Set or clear the StdMulti camera setter. */
    void SetStdMultiCameraSetter(StdMultiCameraSetter setter);

    /** Set or clear the StdMulti selected-slice getter. */
    void SetStdMultiSelectedSliceGetter(StdMultiSelectedSliceGetter getter);

    /** Set or clear the StdMulti selected-slice step setter. */
    void SetStdMultiSelectedSliceStepSetter(StdMultiSelectedSliceStepSetter setter);

    /** Set or clear the MxN window list provider. */
    void SetMxNWindowListProvider(MxNWindowListProvider provider);

    /** Set or clear the MxN editor screenshot provider. */
    void SetMxNEditorScreenshotProvider(MxNEditorScreenshotProvider provider);

    /** Set or clear the MxN per-window screenshot provider. */
    void SetMxNWindowScreenshotProvider(MxNWindowScreenshotProvider provider);

    /** Set or clear the MxN camera getter. */
    void SetMxNCameraGetter(MxNCameraGetter getter);

    /** Set or clear the MxN camera setter. */
    void SetMxNCameraSetter(MxNCameraSetter setter);

    /** Set or clear the MxN selected-slice getter. */
    void SetMxNSelectedSliceGetter(MxNSelectedSliceGetter getter);

    /** Set or clear the MxN selected-slice step setter. */
    void SetMxNSelectedSliceStepSetter(MxNSelectedSliceStepSetter setter);

    /** Set or clear the MxN per-cell selected-position getter. */
    void SetMxNSelectedPositionGetter(MxNSelectedPositionGetter getter);

    /** Set or clear the MxN per-cell selected-position setter. */
    void SetMxNSelectedPositionSetter(MxNSelectedPositionSetter setter);

    /** Set or clear the MxN layout getter (returns v2.0 JSON document as string). */
    void SetMxNLayoutGetter(MxNLayoutGetter getter);

    /** Set or clear the MxN layout setter (accepts v2.0 JSON; returns the freshly serialized result). */
    void SetMxNLayoutSetter(MxNLayoutSetter setter);

    /**
     * \brief Clear all registered callbacks in a single atomic operation.
     *
     * Should be called by the UI-layer owner (e.g. the workbench plugin activator)
     * before the objects captured by the callbacks are destroyed.
     *
     * \note The dispatcher (set via SetDispatcher) is intentionally retained:
     *       it is owned by the REST server, which outlives the callback owner,
     *       so clearing it here would create a needless re-bind on every plugin
     *       cycle. Use SetDispatcher(nullptr) explicitly if a separation is
     *       ever required.
     */
    void ResetCallbacks();

    /** \return true if a screenshot provider is currently set. */
    bool HasScreenshotProvider() const;

    /** \return true if a position getter is currently set. */
    bool HasPositionGetter() const;

    /** \return true if a position setter is currently set. */
    bool HasPositionSetter() const;

    bool HasEditorListProvider() const;
    bool HasStdMultiWindowListProvider() const;
    bool HasStdMultiEditorScreenshotProvider() const;
    bool HasStdMultiWindowScreenshotProvider() const;
    bool HasStdMultiCameraGetter() const;
    bool HasStdMultiCameraSetter() const;
    bool HasStdMultiSelectedSliceGetter() const;
    bool HasStdMultiSelectedSliceStepSetter() const;

    /** \return true if an MxN window list provider is currently set. */
    bool HasMxNWindowListProvider() const;
    /** \return true if an MxN editor screenshot provider is currently set. */
    bool HasMxNEditorScreenshotProvider() const;
    /** \return true if an MxN per-window screenshot provider is currently set. */
    bool HasMxNWindowScreenshotProvider() const;
    /** \return true if an MxN camera getter is currently set. */
    bool HasMxNCameraGetter() const;
    /** \return true if an MxN camera setter is currently set. */
    bool HasMxNCameraSetter() const;
    /** \return true if an MxN selected-slice getter is currently set. */
    bool HasMxNSelectedSliceGetter() const;
    /** \return true if an MxN selected-slice step setter is currently set. */
    bool HasMxNSelectedSliceStepSetter() const;
    /** \return true if an MxN selected-position getter is currently set. */
    bool HasMxNSelectedPositionGetter() const;
    /** \return true if an MxN selected-position setter is currently set. */
    bool HasMxNSelectedPositionSetter() const;
    /** \return true if an MxN layout getter is currently set. */
    bool HasMxNLayoutGetter() const;
    /** \return true if an MxN layout setter is currently set. */
    bool HasMxNLayoutSetter() const;

    /**
     * \brief Capture a screenshot using the registered provider.
     *
     * The provider is copied under the lock and called outside the lock so
     * that the mutex is not held during the (potentially long) operation.
     * Dispatches to the UI thread if a dispatcher is set.
     *
     * \pre HasScreenshotProvider() is true.
     * \throws std::runtime_error if no provider is set.
     * \throws Any exception thrown by the provider.
     */
    std::vector<unsigned char> TakeScreenshot(
      std::optional<std::pair<int, int>> size,
      ScreenshotFormat format) const;

    /**
     * \brief Read the current global crosshair position and scene bounds atomically.
     *
     * Dispatches to the UI thread if a dispatcher is set.
     *
     * \pre HasPositionGetter() is true.
     * \throws std::runtime_error if no getter is set.
     * \throws Any exception thrown by the getter.
     */
    SelectedPositionInfo GetSelectedPosition() const;

    /**
     * \brief Move the crosshair to the given world position.
     *
     * Dispatches to the UI thread if a dispatcher is set.
     *
     * \pre HasPositionSetter() is true.
     * \throws std::runtime_error if no setter is set.
     * \throws Any exception thrown by the setter.
     */
    void SetSelectedPosition(const Point3D& pos) const;

    /**
     * \brief Invoke the editor list provider.
     * \throws std::runtime_error if no provider is set.
     */
    std::vector<EditorInfo> ListEditors() const;

    /**
     * \brief Invoke the StdMulti window list provider.
     * \throws std::runtime_error if no provider is set.
     * \throws RenderWindowBridgeNoEditorException if the editor is not open.
     */
    std::vector<WindowInfo> ListStdMultiWindows() const;

    /**
     * \brief Invoke the StdMulti editor screenshot provider.
     * \throws std::runtime_error if no provider is set.
     * \throws RenderWindowBridgeNoEditorException if the editor is not open.
     */
    std::vector<unsigned char> TakeStdMultiEditorScreenshot(
      std::optional<std::pair<int, int>> size,
      ScreenshotFormat format) const;

    /**
     * \brief Invoke the StdMulti per-window screenshot provider.
     * \throws std::runtime_error if no provider is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     */
    std::vector<unsigned char> TakeStdMultiWindowScreenshot(
      const std::string& windowId,
      std::optional<std::pair<int, int>> size,
      ScreenshotFormat format) const;

    CameraState GetStdMultiCamera(const std::string& windowId) const;
    void SetStdMultiCamera(const std::string& windowId, const CameraPatch& patch) const;
    SliceState GetStdMultiSelectedSlice(const std::string& windowId) const;
    void SetStdMultiSelectedSliceStep(const std::string& windowId, unsigned int step) const;

    /**
     * \brief Invoke the MxN window list provider.
     * \throws std::runtime_error if no provider is set.
     * \throws RenderWindowBridgeNoEditorException if the editor is not open.
     */
    std::vector<MxNWindowInfo> ListMxNWindows() const;

    /**
     * \brief Invoke the MxN editor screenshot provider.
     * \throws std::runtime_error if no provider is set.
     * \throws RenderWindowBridgeNoEditorException if the editor is not open.
     */
    std::vector<unsigned char> TakeMxNEditorScreenshot(
      std::optional<std::pair<int, int>> size,
      ScreenshotFormat format) const;

    /**
     * \brief Invoke the MxN per-window screenshot provider.
     * \throws std::runtime_error if no provider is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     */
    std::vector<unsigned char> TakeMxNWindowScreenshot(
      const std::string& windowId,
      std::optional<std::pair<int, int>> size,
      ScreenshotFormat format) const;

    /**
     * \brief Read the camera state of a specific MxN cell.
     *
     * \param windowId Canonical fully-qualified window id (e.g. "mxn__widget0").
     * \pre HasMxNCameraGetter() is true.
     * \throws std::runtime_error if no getter is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     * \throws Any exception thrown by the getter.
     */
    CameraState GetMxNCamera(const std::string& windowId) const;

    /**
     * \brief Apply a camera patch to a specific MxN cell.
     *
     * \param windowId Canonical fully-qualified window id.
     * \param patch Partial camera update.
     * \pre HasMxNCameraSetter() is true.
     * \throws std::runtime_error if no setter is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     * \throws Any exception thrown by the setter.
     */
    void SetMxNCamera(const std::string& windowId, const CameraPatch& patch) const;

    /**
     * \brief Read the selected-slice state of a 2D MxN cell.
     *
     * \param windowId Canonical fully-qualified window id.
     * \pre HasMxNSelectedSliceGetter() is true.
     * \pre windowId addresses a 2D MxN cell.
     * \throws std::runtime_error if no getter is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     * \throws Any exception thrown by the getter.
     */
    SliceState GetMxNSelectedSlice(const std::string& windowId) const;

    /**
     * \brief Set the selected slice of a 2D MxN cell by step index.
     *
     * \param windowId Canonical fully-qualified window id.
     * \param step Zero-based slice index along the cell's view direction.
     * \pre HasMxNSelectedSliceStepSetter() is true.
     * \pre windowId addresses a 2D MxN cell.
     * \throws std::runtime_error if no setter is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     * \throws Any exception thrown by the setter.
     */
    void SetMxNSelectedSliceStep(const std::string& windowId, unsigned int step) const;

    /**
     * \brief Read the per-cell selected position and scene bounds atomically.
     *
     * \param windowId Canonical fully-qualified window id.
     * \pre HasMxNSelectedPositionGetter() is true.
     * \throws std::runtime_error if no getter is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     * \throws Any exception thrown by the getter.
     */
    SelectedPositionInfo GetMxNSelectedPosition(const std::string& windowId) const;

    /**
     * \brief Set the per-cell selected position (cell anchor).
     *
     * \param windowId Canonical fully-qualified window id.
     * \param position World coordinate for the cell's anchor.
     * \pre HasMxNSelectedPositionSetter() is true.
     * \throws std::runtime_error if no setter is set.
     * \throws RenderWindowBridgeNoEditorException / RenderWindowBridgeUnknownWindowException.
     * \throws Any exception thrown by the setter.
     */
    void SetMxNSelectedPosition(const std::string& windowId, const Point3D& position) const;

    /** Get the current MxN layout as a v2.0 JSON document (string). */
    std::string GetMxNLayout() const;

    /**
     * \brief Apply a v2.0 layout document; returns the freshly serialized layout.
     *
     * The string boundary keeps the bridge header free of nlohmann/json.hpp.
     * Schema / structural failures escape as \c mitk::Exception (the engine's
     * native throw type) which the controller maps locally to 400.
     */
    std::string SetMxNLayout(const std::string& layoutJson) const;

  private:
    ScreenshotProvider m_ScreenshotProvider;
    PositionGetter m_PositionGetter;
    PositionSetter m_PositionSetter;
    EditorListProvider m_EditorListProvider;
    StdMultiWindowListProvider m_StdMultiWindowListProvider;
    StdMultiEditorScreenshotProvider m_StdMultiEditorScreenshotProvider;
    StdMultiWindowScreenshotProvider m_StdMultiWindowScreenshotProvider;
    StdMultiCameraGetter m_StdMultiCameraGetter;
    StdMultiCameraSetter m_StdMultiCameraSetter;
    StdMultiSelectedSliceGetter m_StdMultiSelectedSliceGetter;
    StdMultiSelectedSliceStepSetter m_StdMultiSelectedSliceStepSetter;
    MxNWindowListProvider m_MxNWindowListProvider;
    MxNEditorScreenshotProvider m_MxNEditorScreenshotProvider;
    MxNWindowScreenshotProvider m_MxNWindowScreenshotProvider;
    MxNCameraGetter m_MxNCameraGetter;
    MxNCameraSetter m_MxNCameraSetter;
    MxNSelectedSliceGetter m_MxNSelectedSliceGetter;
    MxNSelectedSliceStepSetter m_MxNSelectedSliceStepSetter;
    MxNSelectedPositionGetter m_MxNSelectedPositionGetter;
    MxNSelectedPositionSetter m_MxNSelectedPositionSetter;
    MxNLayoutGetter m_MxNLayoutGetter;
    MxNLayoutSetter m_MxNLayoutSetter;
    mutable std::mutex m_Mutex;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;
  };
}

#endif
