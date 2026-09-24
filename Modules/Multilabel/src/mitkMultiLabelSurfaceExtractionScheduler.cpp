/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelSurfaceExtractionScheduler.h>

#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkIDataStorageService.h>
#include <mitkLog.h>
#include <mitkMultiLabelSurfaceNetsExtractor.h>
#include <mitkRenderingManager.h>
#include <mitkStorageThreadDispatcherBase.h>

#include <vtkImageData.h>
#include <vtkPolyData.h>

#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <exception>
#include <map>
#include <mutex>
#include <utility>

namespace
{
  using Scheduler = mitk::MultiLabelSurfaceExtractionScheduler;

  // Keeps alive what the worker reads, and is released on the thread that owns the data
  // storage, since releasing an image notifies its observers. The image is declared first so
  // that it is destroyed last: the VTK accessors of the volume refer back to it from their
  // destructors.
  struct Pins
  {
    mitk::Image::ConstPointer m_Image;
    mitk::Image::ImageDataItemPointer m_Volume;
    vtkSmartPointer<vtkImageData> m_View;
  };

  struct Job
  {
    std::uint64_t m_Id = 0;
    Scheduler::Stamp m_Stamp;
    std::vector<Scheduler::LabelValueType> m_LabelValues;
    Pins m_Pins;
  };

  struct RunningJob
  {
    std::uint64_t m_Id = 0;
    Scheduler::Stamp m_Stamp;
  };

  struct Group
  {
    std::optional<Job> m_Queued;
    std::optional<RunningJob> m_Running;
    std::optional<Scheduler::Result> m_Result;
  };

  bool HasStorageThread()
  {
    mitk::CoreServicePointer<mitk::IDataStorageService> service(mitk::CoreServices::GetDataStorageService());
    return service && nullptr != service->GetDispatcher();
  }

  void RequestRender()
  {
    if (auto* renderingManager = mitk::RenderingManager::GetInstance())
      renderingManager->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  }

  Pins Pin(const mitk::Image* groupImage, mitk::TimeStepType timeStep)
  {
    if (!groupImage->GetTimeGeometry()->IsValidTimeStep(timeStep))
      mitkThrow() << "Cannot extract the surface of a group image. It does not have time step " << timeStep << ".";

    Pins pins;
    pins.m_Image = groupImage;
    pins.m_Volume = groupImage->GetVolumeData(static_cast<int>(timeStep));

    if (pins.m_Volume.IsNotNull())
      pins.m_View = pins.m_Volume->CreateVtkImageDataView(groupImage);

    if (pins.m_View == nullptr)
      mitkThrow() << "Cannot extract the surface of a group image. Its pixel type or dimension has no VTK equivalent.";

    return pins;
  }

  vtkSmartPointer<vtkPolyData> Extract(mitk::MultiLabelSurfaceNetsExtractor& extractor, const Job& job)
  {
    // An exception must not escape the worker, and a result, even an empty one, lets
    // the group settle instead of being requested over and over.
    try
    {
      extractor.SetSmoothing(job.m_Stamp.m_Smoothed);
      return extractor.Extract(job.m_Pins.m_View, job.m_LabelValues);
    }
    catch (const std::exception& e)
    {
      MITK_ERROR << "Extracting the surface of a segmentation group failed: " << e.what();
    }

    return vtkSmartPointer<vtkPolyData>::New();
  }

  bool IsCovered(const Group& group, const Scheduler::Stamp& stamp)
  {
    return (group.m_Running.has_value() && group.m_Running->m_Stamp == stamp) ||
      (group.m_Result.has_value() && group.m_Result->m_Stamp == stamp);
  }
}

// Shared with the threads, so that it outlives a scheduler whose worker is still extracting.
struct mitk::MultiLabelSurfaceExtractionScheduler::Shared
{
  std::mutex m_Mutex;
  std::condition_variable m_Wake;

  std::map<const Image*, Group> m_Groups;

  // Groups with a queued job, the longest waiting first.
  std::deque<const Image*> m_Queue;

  std::uint64_t m_LastJobId = 0;
  bool m_Extracting = false;
  bool m_Stop = false;

  static void Work(std::shared_ptr<Shared> shared);
};

void mitk::MultiLabelSurfaceExtractionScheduler::Shared::Work(std::shared_ptr<Shared> shared)
{
  MultiLabelSurfaceNetsExtractor extractor;

  std::unique_lock lock(shared->m_Mutex);

  for (;;)
  {
    shared->m_Wake.wait(lock, [&shared]() { return shared->m_Stop || !shared->m_Queue.empty(); });

    if (shared->m_Stop)
      return;

    const auto* key = shared->m_Queue.front();
    shared->m_Queue.pop_front();

    auto& group = shared->m_Groups.at(key);
    auto job = std::move(*std::exchange(group.m_Queued, std::nullopt));
    group.m_Running = RunningJob{ job.m_Id, job.m_Stamp };
    shared->m_Extracting = true;

    lock.unlock();

    const auto surface = Extract(extractor, job);

    lock.lock();

    shared->m_Extracting = false;

    // Forget(), the destructor and a synchronous extraction in the meantime make this
    // result obsolete.
    if (auto found = shared->m_Groups.find(key);
      found != shared->m_Groups.end() && found->second.m_Running.has_value() && found->second.m_Running->m_Id == job.m_Id)
    {
      found->second.m_Running.reset();
      found->second.m_Result = Result{ surface, job.m_Stamp };
    }

    lock.unlock();

    // The pins travel with the task and are released where it runs. It captures nothing of
    // the scheduler, so it is safe after its destruction.
    PostToStorageThread([pins = std::move(job.m_Pins)]() { RequestRender(); });

    lock.lock();
  }
}

mitk::MultiLabelSurfaceExtractionScheduler::Stamp mitk::MultiLabelSurfaceExtractionScheduler::Stamp::Of(
  const Image* groupImage, TimeStepType timeStep, bool smoothed)
{
  return { std::max(groupImage->GetMTime(), groupImage->GetPipelineMTime()), timeStep, smoothed };
}

mitk::MultiLabelSurfaceExtractionScheduler::MultiLabelSurfaceExtractionScheduler()
  : m_Shared(std::make_shared<Shared>())
{
}

mitk::MultiLabelSurfaceExtractionScheduler::~MultiLabelSurfaceExtractionScheduler()
{
  // Released here, outside the lock, on the thread that owns the data storage.
  std::map<const Image*, Group> groups;
  bool extracting = false;

  {
    std::lock_guard lock(m_Shared->m_Mutex);
    m_Shared->m_Stop = true;
    extracting = m_Shared->m_Extracting;
    groups.swap(m_Shared->m_Groups);
    m_Shared->m_Queue.clear();
  }

  m_Shared->m_Wake.notify_all();

  if (m_Worker.joinable())
  {
    if (extracting)
    {
      m_Worker.detach();
    }
    else
    {
      m_Worker.join();
    }
  }
}

void mitk::MultiLabelSurfaceExtractionScheduler::Request(const Image* groupImage, TimeStepType timeStep, bool smoothed,
  const std::vector<LabelValueType>& labelValues)
{
  if (nullptr == groupImage)
    mitkThrow() << "Cannot extract the surface of a group image. Passed image is nullptr.";

  const auto stamp = Stamp::Of(groupImage, timeStep, smoothed);

  // Released outside the lock.
  std::optional<Job> replaced;

  {
    std::lock_guard lock(m_Shared->m_Mutex);

    if (auto found = m_Shared->m_Groups.find(groupImage); found != m_Shared->m_Groups.end())
    {
      auto& group = found->second;

      if (group.m_Queued.has_value() && group.m_Queued->m_Stamp == stamp)
        return;

      if (IsCovered(group, stamp))
      {
        replaced = std::exchange(group.m_Queued, std::nullopt);
        std::erase(m_Shared->m_Queue, groupImage);
        return;
      }
    }
  }

  Job job{ 0, stamp, labelValues, Pin(groupImage, timeStep) };

  if (!HasStorageThread())
  {
    MultiLabelSurfaceNetsExtractor extractor;
    auto surface = Extract(extractor, job);

    std::lock_guard lock(m_Shared->m_Mutex);

    auto& group = m_Shared->m_Groups[groupImage];
    replaced = std::exchange(group.m_Queued, std::nullopt);
    std::erase(m_Shared->m_Queue, groupImage);

    // Newer than whatever may still be running.
    group.m_Running.reset();
    group.m_Result = Result{ surface, stamp };

    return;
  }

  if (!m_Worker.joinable())
    m_Worker = std::thread(&Shared::Work, m_Shared);

  {
    std::lock_guard lock(m_Shared->m_Mutex);

    job.m_Id = ++m_Shared->m_LastJobId;

    auto& group = m_Shared->m_Groups[groupImage];

    if (!group.m_Queued.has_value())
      m_Shared->m_Queue.push_back(groupImage);

    replaced = std::exchange(group.m_Queued, std::move(job));
  }

  m_Shared->m_Wake.notify_all();
}

std::optional<mitk::MultiLabelSurfaceExtractionScheduler::Result> mitk::MultiLabelSurfaceExtractionScheduler::GetResult(
  const Image* groupImage) const
{
  std::lock_guard lock(m_Shared->m_Mutex);

  const auto found = m_Shared->m_Groups.find(groupImage);

  if (found == m_Shared->m_Groups.end())
    return std::nullopt;

  return found->second.m_Result;
}

bool mitk::MultiLabelSurfaceExtractionScheduler::IsPending(const Image* groupImage) const
{
  std::lock_guard lock(m_Shared->m_Mutex);

  const auto found = m_Shared->m_Groups.find(groupImage);

  return found != m_Shared->m_Groups.end() && (found->second.m_Queued.has_value() || found->second.m_Running.has_value());
}

void mitk::MultiLabelSurfaceExtractionScheduler::Forget(const Image* groupImage)
{
  // Released outside the lock.
  std::optional<Group> forgotten;

  std::lock_guard lock(m_Shared->m_Mutex);

  const auto found = m_Shared->m_Groups.find(groupImage);

  if (found == m_Shared->m_Groups.end())
    return;

  forgotten = std::move(found->second);
  m_Shared->m_Groups.erase(found);
  std::erase(m_Shared->m_Queue, groupImage);
}
