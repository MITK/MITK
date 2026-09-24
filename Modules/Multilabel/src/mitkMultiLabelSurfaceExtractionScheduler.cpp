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

  // A group image and a smoothing.
  using QueueEntry = std::pair<const mitk::Image*, bool>;

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

  // Queued jobs and results are kept per smoothing, which renderers may want differently. Not
  // per time step, which renderers share: while scrubbing through time, only the time step
  // arrived at is to wait for the worker.
  struct Group
  {
    std::map<bool, Job> m_Queued;
    std::optional<RunningJob> m_Running;

    // All from the same data, see Store().
    std::map<bool, Scheduler::Result> m_Results;

    // Whether the stamp is being extracted or has been.
    bool IsCovered(const Scheduler::Stamp& stamp) const
    {
      if (m_Running.has_value() && m_Running->m_Stamp == stamp)
        return true;

      const auto found = m_Results.find(stamp.m_Smoothed);
      return found != m_Results.end() && found->second.m_Stamp == stamp;
    }

    // Keeps only the results of the newest data: a result from older data than a kept one is
    // dropped, and one from newer data drops the kept ones.
    void Store(Scheduler::Result result)
    {
      const auto dataMTime = result.m_Stamp.m_DataMTime;

      const bool outdated = std::any_of(m_Results.cbegin(), m_Results.cend(),
        [dataMTime](const auto& entry) { return entry.second.m_Stamp.m_DataMTime > dataMTime; });

      if (outdated)
        return;

      std::erase_if(m_Results,
        [dataMTime](const auto& entry) { return entry.second.m_Stamp.m_DataMTime < dataMTime; });

      m_Results.insert_or_assign(result.m_Stamp.m_Smoothed, std::move(result));
    }
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
}

// Shared with the threads, so that it outlives a scheduler whose worker is still extracting.
struct mitk::MultiLabelSurfaceExtractionScheduler::Shared
{
  std::mutex m_Mutex;
  std::condition_variable m_Wake;

  std::map<const Image*, Group> m_Groups;

  // Groups and smoothings with a queued job, the longest waiting first.
  std::deque<QueueEntry> m_Queue;

  std::uint64_t m_LastJobId = 0;
  bool m_Extracting = false;
  bool m_Stop = false;

  static void Work(std::shared_ptr<Shared> shared);

  // Removes what is queued for the smoothing, if anything, for the caller to release.
  std::optional<Job> Unqueue(const Image* groupImage, Group& group, bool smoothed);
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

    const auto [key, smoothed] = shared->m_Queue.front();
    shared->m_Queue.pop_front();

    auto& group = shared->m_Groups.at(key);
    const auto queued = group.m_Queued.find(smoothed);
    auto job = std::move(queued->second);
    group.m_Queued.erase(queued);
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
      found->second.Store(Result{ surface, job.m_Stamp });
    }

    lock.unlock();

    // The pins travel with the task and are released where it runs. It captures nothing of
    // the scheduler, so it is safe after its destruction.
    PostToStorageThread([pins = std::move(job.m_Pins)]() { RequestRender(); });

    lock.lock();
  }
}

std::optional<Job> mitk::MultiLabelSurfaceExtractionScheduler::Shared::Unqueue(const Image* groupImage, Group& group,
  bool smoothed)
{
  const auto queued = group.m_Queued.find(smoothed);

  if (queued == group.m_Queued.end())
    return std::nullopt;

  std::optional<Job> job = std::move(queued->second);
  group.m_Queued.erase(queued);
  std::erase(m_Queue, QueueEntry{ groupImage, smoothed });

  return job;
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

      if (auto queued = group.m_Queued.find(smoothed); queued != group.m_Queued.end() && queued->second.m_Stamp == stamp)
        return;

      if (group.IsCovered(stamp))
      {
        // Whatever is still queued for the smoothing was asked for before, and is no longer
        // wanted.
        replaced = m_Shared->Unqueue(groupImage, group, smoothed);
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
    replaced = m_Shared->Unqueue(groupImage, group, smoothed);

    // Newer than whatever may still be running.
    group.m_Running.reset();
    group.Store(Result{ surface, stamp });

    return;
  }

  if (!m_Worker.joinable())
    m_Worker = std::thread(&Shared::Work, m_Shared);

  {
    std::lock_guard lock(m_Shared->m_Mutex);

    job.m_Id = ++m_Shared->m_LastJobId;

    auto& group = m_Shared->m_Groups[groupImage];

    // A replaced job keeps its place in the queue.
    if (auto queued = group.m_Queued.find(smoothed); queued != group.m_Queued.end())
    {
      replaced = std::move(queued->second);
      queued->second = std::move(job);
    }
    else
    {
      m_Shared->m_Queue.push_back({ groupImage, smoothed });
      group.m_Queued.emplace(smoothed, std::move(job));
    }
  }

  m_Shared->m_Wake.notify_all();
}

std::optional<mitk::MultiLabelSurfaceExtractionScheduler::Result> mitk::MultiLabelSurfaceExtractionScheduler::GetResult(
  const Image* groupImage, TimeStepType timeStep, bool smoothed) const
{
  std::lock_guard lock(m_Shared->m_Mutex);

  const auto found = m_Shared->m_Groups.find(groupImage);

  if (found == m_Shared->m_Groups.end())
    return std::nullopt;

  const auto& results = found->second.m_Results;

  for (const bool preferred : { smoothed, !smoothed })
  {
    if (const auto result = results.find(preferred); result != results.end() && result->second.m_Stamp.m_TimeStep == timeStep)
      return result->second;
  }

  return std::nullopt;
}

bool mitk::MultiLabelSurfaceExtractionScheduler::IsPending(const Image* groupImage) const
{
  std::lock_guard lock(m_Shared->m_Mutex);

  const auto found = m_Shared->m_Groups.find(groupImage);

  return found != m_Shared->m_Groups.end() && (!found->second.m_Queued.empty() || found->second.m_Running.has_value());
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
  std::erase_if(m_Shared->m_Queue, [groupImage](const QueueEntry& entry) { return entry.first == groupImage; });
}
