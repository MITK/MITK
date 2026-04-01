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
#include <mitkWeakPointer.h>

#include <functional>
#include <mutex>
#include <optional>
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
  };

  /**
   * \brief Position and world-bounds snapshot read atomically on the UI thread. */
  struct SelectedPositionInfo
  {
    /** Current crosshair position in world coordinates. */
    Point3D position;

    /** World bounding box, if available at read time. */
    std::optional<WorldBounds> bounds;
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

  private:
    ScreenshotProvider m_ScreenshotProvider;
    PositionGetter m_PositionGetter;
    PositionSetter m_PositionSetter;
    mutable std::mutex m_Mutex;
    WeakPointer<StorageThreadDispatcherBase> m_Dispatcher;
  };
}

#endif
