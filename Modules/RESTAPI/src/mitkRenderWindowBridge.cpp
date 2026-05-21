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

const char* AnatomicalPlaneToV2String(AnatomicalPlane plane)
{
  switch (plane)
  {
    case AnatomicalPlane::Axial:    return "axial";
    case AnatomicalPlane::Sagittal: return "sagittal";
    case AnatomicalPlane::Coronal:  return "coronal";
    case AnatomicalPlane::Original: return "original";
  }
  throw std::invalid_argument(
    "AnatomicalPlaneToV2String: value outside AnatomicalPlane enum domain (" +
    std::to_string(static_cast<int>(plane)) + ")");
}

AnatomicalPlane ParseV2ViewDirection(const std::string& s)
{
  if (s == "axial")    return AnatomicalPlane::Axial;
  if (s == "sagittal") return AnatomicalPlane::Sagittal;
  if (s == "coronal")  return AnatomicalPlane::Coronal;
  if (s == "original") return AnatomicalPlane::Original;
  throw std::invalid_argument("ParseV2ViewDirection: unknown view_direction '" + s + "'");
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

void RenderWindowBridge::SetMxNWindowListProvider(MxNWindowListProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNWindowListProvider = std::move(provider);
}

void RenderWindowBridge::SetMxNEditorScreenshotProvider(MxNEditorScreenshotProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNEditorScreenshotProvider = std::move(provider);
}

void RenderWindowBridge::SetMxNWindowScreenshotProvider(MxNWindowScreenshotProvider provider)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNWindowScreenshotProvider = std::move(provider);
}

void RenderWindowBridge::SetMxNCameraGetter(MxNCameraGetter getter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNCameraGetter = std::move(getter);
}

void RenderWindowBridge::SetMxNCameraSetter(MxNCameraSetter setter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNCameraSetter = std::move(setter);
}

void RenderWindowBridge::SetMxNSelectedSliceGetter(MxNSelectedSliceGetter getter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNSelectedSliceGetter = std::move(getter);
}

void RenderWindowBridge::SetMxNSelectedSliceStepSetter(MxNSelectedSliceStepSetter setter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNSelectedSliceStepSetter = std::move(setter);
}

void RenderWindowBridge::SetMxNSelectedPositionGetter(MxNSelectedPositionGetter getter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNSelectedPositionGetter = std::move(getter);
}

void RenderWindowBridge::SetMxNSelectedPositionSetter(MxNSelectedPositionSetter setter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNSelectedPositionSetter = std::move(setter);
}

void RenderWindowBridge::SetMxNLayoutGetter(MxNLayoutGetter getter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNLayoutGetter = std::move(getter);
}

void RenderWindowBridge::SetMxNLayoutSetter(MxNLayoutSetter setter)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_MxNLayoutSetter = std::move(setter);
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
  m_MxNWindowListProvider = {};
  m_MxNEditorScreenshotProvider = {};
  m_MxNWindowScreenshotProvider = {};
  m_MxNCameraGetter = {};
  m_MxNCameraSetter = {};
  m_MxNSelectedSliceGetter = {};
  m_MxNSelectedSliceStepSetter = {};
  m_MxNSelectedPositionGetter = {};
  m_MxNSelectedPositionSetter = {};
  m_MxNLayoutGetter = {};
  m_MxNLayoutSetter = {};
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

bool RenderWindowBridge::HasMxNWindowListProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNWindowListProvider);
}

bool RenderWindowBridge::HasMxNEditorScreenshotProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNEditorScreenshotProvider);
}

bool RenderWindowBridge::HasMxNWindowScreenshotProvider() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNWindowScreenshotProvider);
}

bool RenderWindowBridge::HasMxNCameraGetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNCameraGetter);
}

bool RenderWindowBridge::HasMxNCameraSetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNCameraSetter);
}

bool RenderWindowBridge::HasMxNSelectedSliceGetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNSelectedSliceGetter);
}

bool RenderWindowBridge::HasMxNSelectedSliceStepSetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNSelectedSliceStepSetter);
}

bool RenderWindowBridge::HasMxNSelectedPositionGetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNSelectedPositionGetter);
}

bool RenderWindowBridge::HasMxNSelectedPositionSetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNSelectedPositionSetter);
}

bool RenderWindowBridge::HasMxNLayoutGetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNLayoutGetter);
}

bool RenderWindowBridge::HasMxNLayoutSetter() const
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  return static_cast<bool>(m_MxNLayoutSetter);
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

std::vector<MxNWindowInfo> RenderWindowBridge::ListMxNWindows() const
{
  MxNWindowListProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_MxNWindowListProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No MxN window list provider is connected to the RenderWindowBridge");

  std::vector<MxNWindowInfo> result;
  Dispatch(dispatcher, [&]() { result = provider(); });
  return result;
}

std::vector<unsigned char> RenderWindowBridge::TakeMxNEditorScreenshot(
  std::optional<std::pair<int, int>> size,
  ScreenshotFormat format) const
{
  MxNEditorScreenshotProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_MxNEditorScreenshotProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No MxN editor screenshot provider is connected to the RenderWindowBridge");

  std::vector<unsigned char> result;
  Dispatch(dispatcher, [&]() { result = provider(size, format); });
  return result;
}

std::vector<unsigned char> RenderWindowBridge::TakeMxNWindowScreenshot(
  const std::string& windowId,
  std::optional<std::pair<int, int>> size,
  ScreenshotFormat format) const
{
  MxNWindowScreenshotProvider provider;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    provider = m_MxNWindowScreenshotProvider;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!provider)
    throw std::runtime_error("No MxN window screenshot provider is connected to the RenderWindowBridge");

  std::vector<unsigned char> result;
  Dispatch(dispatcher, [&]() { result = provider(windowId, size, format); });
  return result;
}

CameraState RenderWindowBridge::GetMxNCamera(const std::string& windowId) const
{
  MxNCameraGetter getter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    getter = m_MxNCameraGetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!getter)
    throw std::runtime_error("No MxN camera getter is connected to the RenderWindowBridge");

  CameraState result;
  result.position.Fill(0.0);
  result.focalPoint.Fill(0.0);
  result.viewUp.Fill(0.0);
  Dispatch(dispatcher, [&]() { result = getter(windowId); });
  return result;
}

void RenderWindowBridge::SetMxNCamera(const std::string& windowId, const CameraPatch& patch) const
{
  MxNCameraSetter setter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    setter = m_MxNCameraSetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!setter)
    throw std::runtime_error("No MxN camera setter is connected to the RenderWindowBridge");

  Dispatch(dispatcher, [&]() { setter(windowId, patch); });
}

SliceState RenderWindowBridge::GetMxNSelectedSlice(const std::string& windowId) const
{
  MxNSelectedSliceGetter getter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    getter = m_MxNSelectedSliceGetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!getter)
    throw std::runtime_error("No MxN selected-slice getter is connected to the RenderWindowBridge");

  SliceState result;
  result.position.Fill(0.0);
  Dispatch(dispatcher, [&]() { result = getter(windowId); });
  return result;
}

void RenderWindowBridge::SetMxNSelectedSliceStep(const std::string& windowId, unsigned int step) const
{
  MxNSelectedSliceStepSetter setter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    setter = m_MxNSelectedSliceStepSetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!setter)
    throw std::runtime_error("No MxN selected-slice step setter is connected to the RenderWindowBridge");

  Dispatch(dispatcher, [&]() { setter(windowId, step); });
}

SelectedPositionInfo RenderWindowBridge::GetMxNSelectedPosition(const std::string& windowId) const
{
  MxNSelectedPositionGetter getter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    getter = m_MxNSelectedPositionGetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!getter)
    throw std::runtime_error("No MxN selected-position getter is connected to the RenderWindowBridge");

  SelectedPositionInfo result;
  result.position.Fill(0.0);
  Dispatch(dispatcher, [&]() { result = getter(windowId); });
  return result;
}

void RenderWindowBridge::SetMxNSelectedPosition(const std::string& windowId, const Point3D& position) const
{
  MxNSelectedPositionSetter setter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    setter = m_MxNSelectedPositionSetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!setter)
    throw std::runtime_error("No MxN selected-position setter is connected to the RenderWindowBridge");

  Dispatch(dispatcher, [&]() { setter(windowId, position); });
}

std::string RenderWindowBridge::GetMxNLayout() const
{
  MxNLayoutGetter getter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    getter = m_MxNLayoutGetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!getter)
    throw std::runtime_error("No MxN layout getter is connected to the RenderWindowBridge");

  std::string result;
  Dispatch(dispatcher, [&]() { result = getter(); });
  return result;
}

std::string RenderWindowBridge::SetMxNLayout(const std::string& layoutJson) const
{
  MxNLayoutSetter setter;
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    setter = m_MxNLayoutSetter;
    dispatcher = m_Dispatcher.Lock();
  }

  if (!setter)
    throw std::runtime_error("No MxN layout setter is connected to the RenderWindowBridge");

  std::string result;
  Dispatch(dispatcher, [&]() { result = setter(layoutJson); });
  return result;
}

}
