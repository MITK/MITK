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

  /** Axis-aligned world bounding box. */
  struct WorldBounds
  {
    Point3D min;
    Point3D max;

    WorldBounds() : min(), max() {}
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
   * \brief Thrown by bridge callbacks when the window name is not known.
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

  /** Summary of a known editor alias and its current activity state. */
  struct EditorInfo
  {
    std::string alias;
    std::string pluginId;
    bool active = false;
    std::vector<std::string> windowNames; // empty when !active
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

  /** Summary of a single render window exposed by an editor. */
  struct WindowInfo
  {
    std::string name; // "axial" | "sagittal" | "coronal" | "3d"
    WindowKind  kind = WindowKind::TwoD;
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
     *       transported to the REST thread — they never reach the UI event loop.
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
     *       transported to the REST thread — they never reach the UI event loop.
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
     *       transported to the REST thread — they never reach the UI event loop.
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
        const std::string& windowName,
        std::optional<std::pair<int, int>> size,
        ScreenshotFormat format)>;

    /** Callback type: read camera state of a StdMulti render window. */
    using StdMultiCameraGetter = std::function<CameraState(const std::string& windowName)>;

    /** Callback type: apply a camera patch on a StdMulti render window. */
    using StdMultiCameraSetter = std::function<void(const std::string& windowName, const CameraPatch& patch)>;

    /** Callback type: read the selected-slice state of a StdMulti 2D window. */
    using StdMultiSelectedSliceGetter = std::function<SliceState(const std::string& windowName)>;

    /**
     * \brief Callback type: set the selected slice of a StdMulti 2D window by step index.
     *
     * StdMulti only supports step-based slice selection; world-position
     * addressing is reserved for /rendering/selected-position. If MxN later
     * needs position-based addressing, add a separate setter alias for it.
     */
    using StdMultiSelectedSliceStepSetter = std::function<void(const std::string& windowName, unsigned int step)>;

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

    /**
     * \brief Clear all registered callbacks in a single atomic operation.
     *
     * Should be called by the UI-layer owner (e.g. the workbench plugin activator)
     * before the objects captured by the callbacks are destroyed.
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
      const std::string& windowName,
      std::optional<std::pair<int, int>> size,
      ScreenshotFormat format) const;

    CameraState GetStdMultiCamera(const std::string& windowName) const;
    void SetStdMultiCamera(const std::string& windowName, const CameraPatch& patch) const;
    SliceState GetStdMultiSelectedSlice(const std::string& windowName) const;
    void SetStdMultiSelectedSliceStep(const std::string& windowName, unsigned int step) const;

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
    mutable std::mutex m_Mutex;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;
  };
}

#endif
