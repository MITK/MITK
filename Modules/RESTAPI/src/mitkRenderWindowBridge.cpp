/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderWindowBridge.h"

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

    auto task = [&callable, &error]()
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

void RenderWindowBridge::ResetCallbacks()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_ScreenshotProvider = {};
  m_PositionGetter = {};
  m_PositionSetter = {};
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

Point3D RenderWindowBridge::GetSelectedPosition() const
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

  Point3D result;
  result.Fill(0.0);
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

}
