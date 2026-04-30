/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderWindowBridge.h"

#include <cassert>
#include <stdexcept>

namespace
{
  /**
   * @brief Invoke callable on the dispatcher thread (or inline if no dispatcher),
   *        capturing and rethrowing any exception on the calling thread.
   *
   * The callable is taken by value so the caller's copy (made under the bridge mutex)
   * remains valid regardless of how the dispatcher schedules execution.
   */
  template<typename Callable>
  void Dispatch(const mitk::StorageThreadDispatcherBase::Pointer& dispatcher, Callable callable)
  {
    std::exception_ptr error;

    auto task = [callable, &error]()
    {
      try
      {
        callable();
      }
      catch (...)
      {
        error = std::current_exception();
      }
    };

    if (dispatcher.IsNotNull())
    {
      dispatcher->Execute(std::move(task));
    }
    else
    {
      task();
    }

    if (error)
      std::rethrow_exception(error);
  }
}

namespace mitk
{

const char* WindowKindToString(WindowKind kind)
{
  switch (kind)
  {
    case WindowKind::TwoD:   return "2d";
    case WindowKind::ThreeD: return "3d";
  }
  // All enum values are handled above; a value outside the enum domain is UB.
  // Trip an assert in debug to flag the contract break, keep a safe token in
  // release so the bridge cannot crash callers.
  assert(false && "WindowKindToString: value outside WindowKind enum domain");
  return "unknown";
}

void RenderWindowBridge::SetDispatcher(StorageThreadDispatcherBase* dispatcher)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Dispatcher = dispatcher;
}

void RenderWindowBridge::SetScreenshotProvider(ScreenshotProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_ScreenshotProvider = std::move(provider);
}

void RenderWindowBridge::SetPositionGetter(PositionGetter getter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_PositionGetter = std::move(getter);
}

void RenderWindowBridge::SetPositionSetter(PositionSetter setter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_PositionSetter = std::move(setter);
}

void RenderWindowBridge::SetEditorListProvider(EditorListProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_EditorListProvider = std::move(provider);
}

void RenderWindowBridge::SetStdMultiWindowListProvider(StdMultiWindowListProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_StdMultiWindowListProvider = std::move(provider);
}

void RenderWindowBridge::SetStdMultiEditorScreenshotProvider(StdMultiEditorScreenshotProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_StdMultiEditorScreenshotProvider = std::move(provider);
}

void RenderWindowBridge::SetStdMultiWindowScreenshotProvider(StdMultiWindowScreenshotProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_StdMultiWindowScreenshotProvider = std::move(provider);
}

void RenderWindowBridge::SetStdMultiCameraGetter(StdMultiCameraGetter getter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_StdMultiCameraGetter = std::move(getter);
}

void RenderWindowBridge::SetStdMultiCameraSetter(StdMultiCameraSetter setter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_StdMultiCameraSetter = std::move(setter);
}

void RenderWindowBridge::SetStdMultiSelectedSliceGetter(StdMultiSelectedSliceGetter getter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_StdMultiSelectedSliceGetter = std::move(getter);
}

void RenderWindowBridge::SetStdMultiSelectedSliceStepSetter(StdMultiSelectedSliceStepSetter setter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_StdMultiSelectedSliceStepSetter = std::move(setter);
}

void RenderWindowBridge::ResetCallbacks()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_ScreenshotProvider = {};
  m_PositionGetter = {};
  m_PositionSetter = {};
  m_EditorListProvider = {};
  m_StdMultiWindowListProvider = {};
  m_StdMultiEditorScreenshotProvider = {};
  m_StdMultiWindowScreenshotProvider = {};
  m_StdMultiCameraGetter = {};
  m_StdMultiCameraSetter = {};
  m_StdMultiSelectedSliceGetter = {};
  m_StdMultiSelectedSliceStepSetter = {};
}

bool RenderWindowBridge::HasScreenshotProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_ScreenshotProvider);
}

bool RenderWindowBridge::HasPositionGetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_PositionGetter);
}

bool RenderWindowBridge::HasPositionSetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_PositionSetter);
}

bool RenderWindowBridge::HasEditorListProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_EditorListProvider);
}

bool RenderWindowBridge::HasStdMultiWindowListProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_StdMultiWindowListProvider);
}

bool RenderWindowBridge::HasStdMultiEditorScreenshotProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_StdMultiEditorScreenshotProvider);
}

bool RenderWindowBridge::HasStdMultiWindowScreenshotProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_StdMultiWindowScreenshotProvider);
}

bool RenderWindowBridge::HasStdMultiCameraGetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_StdMultiCameraGetter);
}

bool RenderWindowBridge::HasStdMultiCameraSetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_StdMultiCameraSetter);
}

bool RenderWindowBridge::HasStdMultiSelectedSliceGetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_StdMultiSelectedSliceGetter);
}

bool RenderWindowBridge::HasStdMultiSelectedSliceStepSetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_StdMultiSelectedSliceStepSetter);
}

std::vector<unsigned char> RenderWindowBridge::TakeScreenshot(
  std::optional<std::pair<int, int>> size,
  ScreenshotFormat format) const
{
  ScreenshotProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_ScreenshotProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No screenshot provider is connected to the RenderWindowBridge");

  std::vector<unsigned char> result;
  Dispatch(dispatcher, [&]() { result = provider(size, format); });
  return result;
}

SelectedPositionInfo RenderWindowBridge::GetSelectedPosition() const
{
  PositionGetter getter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    getter = m_PositionGetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!getter)
    throw std::runtime_error("No position getter is connected to the RenderWindowBridge");

  SelectedPositionInfo result;
  result.position.Fill(0.0);
  Dispatch(dispatcher, [&]() { result = getter(); });
  return result;
}

void RenderWindowBridge::SetSelectedPosition(const Point3D& pos) const
{
  PositionSetter setter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    setter = m_PositionSetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!setter)
    throw std::runtime_error("No position setter is connected to the RenderWindowBridge");

  Dispatch(dispatcher, [&]() { setter(pos); });
}

std::vector<EditorInfo> RenderWindowBridge::ListEditors() const
{
  EditorListProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_EditorListProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No editor list provider is connected to the RenderWindowBridge");

  std::vector<EditorInfo> result;
  Dispatch(dispatcher, [&]() { result = provider(); });
  return result;
}

std::vector<WindowInfo> RenderWindowBridge::ListStdMultiWindows() const
{
  StdMultiWindowListProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_StdMultiWindowListProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No StdMulti window list provider is connected to the RenderWindowBridge");

  std::vector<WindowInfo> result;
  Dispatch(dispatcher, [&]() { result = provider(); });
  return result;
}

std::vector<unsigned char> RenderWindowBridge::TakeStdMultiEditorScreenshot(
  std::optional<std::pair<int, int>> size,
  ScreenshotFormat format) const
{
  StdMultiEditorScreenshotProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_StdMultiEditorScreenshotProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No StdMulti editor screenshot provider is connected to the RenderWindowBridge");

  std::vector<unsigned char> result;
  Dispatch(dispatcher, [&]() { result = provider(size, format); });
  return result;
}

std::vector<unsigned char> RenderWindowBridge::TakeStdMultiWindowScreenshot(
  const std::string& windowName,
  std::optional<std::pair<int, int>> size,
  ScreenshotFormat format) const
{
  StdMultiWindowScreenshotProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_StdMultiWindowScreenshotProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No StdMulti window screenshot provider is connected to the RenderWindowBridge");

  std::vector<unsigned char> result;
  Dispatch(dispatcher, [&]() { result = provider(windowName, size, format); });
  return result;
}

CameraState RenderWindowBridge::GetStdMultiCamera(const std::string& windowName) const
{
  StdMultiCameraGetter getter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    getter = m_StdMultiCameraGetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!getter)
    throw std::runtime_error("No StdMulti camera getter is connected to the RenderWindowBridge");

  CameraState result;
  result.position.Fill(0.0);
  result.focalPoint.Fill(0.0);
  result.viewUp.Fill(0.0);
  Dispatch(dispatcher, [&]() { result = getter(windowName); });
  return result;
}

void RenderWindowBridge::SetStdMultiCamera(const std::string& windowName, const CameraPatch& patch) const
{
  StdMultiCameraSetter setter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    setter = m_StdMultiCameraSetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!setter)
    throw std::runtime_error("No StdMulti camera setter is connected to the RenderWindowBridge");

  Dispatch(dispatcher, [&]() { setter(windowName, patch); });
}

SliceState RenderWindowBridge::GetStdMultiSelectedSlice(const std::string& windowName) const
{
  StdMultiSelectedSliceGetter getter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    getter = m_StdMultiSelectedSliceGetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!getter)
    throw std::runtime_error("No StdMulti selected-slice getter is connected to the RenderWindowBridge");

  SliceState result;
  result.position.Fill(0.0);
  Dispatch(dispatcher, [&]() { result = getter(windowName); });
  return result;
}

void RenderWindowBridge::SetStdMultiSelectedSliceStep(const std::string& windowName, unsigned int step) const
{
  StdMultiSelectedSliceStepSetter setter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    setter = m_StdMultiSelectedSliceStepSetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!setter)
    throw std::runtime_error("No StdMulti selected-slice step setter is connected to the RenderWindowBridge");

  Dispatch(dispatcher, [&]() { setter(windowName, step); });
}

}
