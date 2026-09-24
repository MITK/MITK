/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDataStorageService.h>
#include <mitkExceptionMacro.h>
#include <mitkImageWriteAccessor.h>
#include <mitkMultiLabelSurfaceExtractionScheduler.h>
#include <mitkStorageThreadDispatcherBase.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceRegistration.h>

#include <itkEventObject.h>

#include <vtkImageData.h>
#include <vtkPolyData.h>

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

namespace
{
  /**
   * Stands in for the dispatcher the workbench installs. The thread that owns the data is
   * whichever thread constructs this, and it runs what is posted to it by calling Drain(), the
   * way the Qt implementation lets its event loop do it.
   */
  class FakeDispatcher : public mitk::StorageThreadDispatcherBase
  {
  public:
    mitkClassMacro(FakeDispatcher, mitk::StorageThreadDispatcherBase);
    itkFactorylessNewMacro(Self);

    /** How often anything was posted. */
    std::atomic<int> Posted{ 0 };

    bool IsDispatchThread() const override
    {
      return std::this_thread::get_id() == m_OwningThread;
    }

    void Post(std::function<void()> task) override
    {
      ++Posted;

      std::lock_guard<std::mutex> locked(m_Mutex);
      m_Queue.push_back(std::move(task));
    }

    /** \brief Run whatever has been posted, as an event loop would. */
    void Drain()
    {
      for (;;)
      {
        std::function<void()> task;

        {
          std::lock_guard<std::mutex> locked(m_Mutex);

          if (m_Queue.empty())
            return;

          task = std::move(m_Queue.front());
          m_Queue.pop_front();
        }

        task();
      }
    }

  protected:
    FakeDispatcher()
      : m_OwningThread(std::this_thread::get_id())
    {
    }

    ~FakeDispatcher() override = default;

    bool ExecuteDispatched(std::function<void()> task) override
    {
      std::promise<void> ran;
      auto done = ran.get_future();

      this->Post([&task, &ran]()
        {
          try
          {
            task();
          }
          catch (...)
          {
          }

          ran.set_value();
        });

      done.wait();

      return true;
    }

  private:
    std::thread::id m_OwningThread;

    std::mutex m_Mutex;
    std::deque<std::function<void()>> m_Queue;
  };

  using Scheduler = mitk::MultiLabelSurfaceExtractionScheduler;

  /** A cubic group image of label 1 inside background, a ball of the given radius. */
  mitk::Image::Pointer MakeGroupImage(unsigned int size, unsigned int radius)
  {
    auto image = mitk::Image::New();
    unsigned int dimensions[3] = { size, size, size };
    image->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dimensions);

    mitk::ImageWriteAccessor accessor(image);
    auto* pixels = static_cast<mitk::Label::PixelType*>(accessor.GetData());

    const auto center = static_cast<double>(size) / 2.0;
    const auto radiusSquared = static_cast<double>(radius) * radius;

    for (unsigned int z = 0; z < size; ++z)
      for (unsigned int y = 0; y < size; ++y)
        for (unsigned int x = 0; x < size; ++x)
        {
          const auto dx = x - center;
          const auto dy = y - center;
          const auto dz = z - center;
          pixels[(z * size + y) * size + x] = dx * dx + dy * dy + dz * dz <= radiusSquared ? 1 : 0;
        }

    return image;
  }

  /** Large enough that the worker is still extracting it after LetTheWorkerStart(). */
  mitk::Image::Pointer MakeLargeGroupImage()
  {
    return MakeGroupImage(256, 110);
  }

  /**
   * Gives the worker time to pick up what was just requested, so that a test meets a running
   * extraction rather than a queued one. The tests pass either way, but only then do they
   * exercise what happens to a running extraction.
   */
  void LetTheWorkerStart()
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  mitk::Image::Pointer MakeSmallGroupImage()
  {
    return MakeGroupImage(16, 5);
  }

  /** Whether the scheduler holds no reference to the image any more, having finished with it. */
  bool IsReleased(const mitk::Image* image, int references)
  {
    return image->GetReferenceCount() == references;
  }
}

/**
 * The scheduler is what keeps the 3D rendering of a segmentation from freezing the GUI, and
 * what it hands to the worker has to be released on the thread that owns the data. Neither
 * is visible in the Workbench until it goes wrong, which is why the contract is pinned here.
 */
class mitkMultiLabelSurfaceExtractionSchedulerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSurfaceExtractionSchedulerTestSuite);
  MITK_TEST(WithoutAnOwningThread_ExtractsRightAway_Success);
  MITK_TEST(WithAnOwningThread_ExtractsOnTheWorker_Success);
  MITK_TEST(ImageReleasedDuringAnExtraction_IsDestroyedOnTheOwningThread_Success);
  MITK_TEST(RequestLikeTheResult_IsIgnored_Success);
  MITK_TEST(LatestRequest_Wins_Success);
  MITK_TEST(Forget_DiscardsARunningExtraction_Success);
  MITK_TEST(DestructionDuringAnExtraction_ReleasesTheImageLater_Success);
  MITK_TEST(InvalidRequest_Throws_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    m_Dispatcher = FakeDispatcher::New();

    m_Service = std::make_unique<mitk::DataStorageService>();
    m_Service->SetDispatcher(m_Dispatcher);

    m_Registration =
      us::GetModuleContext()->RegisterService<mitk::IDataStorageService>(m_Service.get());
  }

  void tearDown() override
  {
    if (m_Registration)
      m_Registration.Unregister();

    m_Service.reset();
    m_Dispatcher = nullptr;
  }

  void WithoutAnOwningThread_ExtractsRightAway_Success()
  {
    m_Service->SetDispatcher(nullptr);

    auto image = MakeSmallGroupImage();
    Scheduler scheduler;

    scheduler.Request(image, 0, true, { 1 });

    // Headless rendering and tests rely on the surface being there after the request.
    const auto result = scheduler.GetResult(image);
    CPPUNIT_ASSERT_MESSAGE("Without an owning thread the result has to be there at once", result.has_value());
    CPPUNIT_ASSERT_MESSAGE("No surface was extracted", result->m_Surface->GetNumberOfCells() > 0);
    CPPUNIT_ASSERT_MESSAGE("The result has to carry the stamp of the request",
                           result->m_Stamp == Scheduler::Stamp::Of(image, 0, true));
    CPPUNIT_ASSERT_MESSAGE("Nothing may be left pending", !scheduler.IsPending(image));
  }

  void WithAnOwningThread_ExtractsOnTheWorker_Success()
  {
    auto image = MakeSmallGroupImage();
    const auto references = image->GetReferenceCount();
    Scheduler scheduler;

    scheduler.Request(image, 0, false, { 1 });

    CPPUNIT_ASSERT_MESSAGE("The extraction did not finish",
                           this->WaitUntil([&]() { return IsReleased(image, references); }));

    const auto result = scheduler.GetResult(image);
    CPPUNIT_ASSERT_MESSAGE("A finished extraction has to leave a result", result.has_value());
    CPPUNIT_ASSERT_MESSAGE("No surface was extracted", result->m_Surface->GetNumberOfCells() > 0);
    CPPUNIT_ASSERT_MESSAGE("The result has to carry the stamp of the request",
                           result->m_Stamp == Scheduler::Stamp::Of(image, 0, false));
    CPPUNIT_ASSERT_MESSAGE("Nothing may be left pending", !scheduler.IsPending(image));

    // Extracting inline would have posted nothing.
    CPPUNIT_ASSERT_MESSAGE("The worker has to ask the owning thread to render", m_Dispatcher->Posted.load() > 0);
  }

  void ImageReleasedDuringAnExtraction_IsDestroyedOnTheOwningThread_Success()
  {
    auto image = MakeLargeGroupImage();

    // Gives the volume VTK accessors, whose destructors refer back to the image. Releasing
    // the volume after the image would read freed memory.
    image->GetVtkImageData(0);

    const auto owningThread = std::this_thread::get_id();
    std::thread::id destroyedOn;
    std::atomic<bool> destroyed{ false };

    image->AddObserver(itk::DeleteEvent(), [&destroyedOn, &destroyed](const itk::EventObject&)
      {
        destroyedOn = std::this_thread::get_id();
        destroyed = true;
      });

    Scheduler scheduler;
    scheduler.Request(image, 0, true, { 1 });

    // From here on, the request holds the last reference.
    image = nullptr;

    CPPUNIT_ASSERT_MESSAGE("The image was never released", this->WaitUntil([&destroyed]() { return destroyed.load(); }));

    // Destroying an image notifies its observers, which belong to the owning thread.
    CPPUNIT_ASSERT_MESSAGE("The image has to be destroyed on the owning thread", destroyedOn == owningThread);
  }

  void RequestLikeTheResult_IsIgnored_Success()
  {
    auto image = MakeSmallGroupImage();
    const auto references = image->GetReferenceCount();
    Scheduler scheduler;

    scheduler.Request(image, 0, true, { 1 });

    CPPUNIT_ASSERT_MESSAGE("The extraction did not finish",
                           this->WaitUntil([&]() { return IsReleased(image, references); }));

    const auto surface = scheduler.GetResult(image)->m_Surface;

    // The mapper requests on every render while it waits, and every renderer does.
    scheduler.Request(image, 0, true, { 1 });

    CPPUNIT_ASSERT_MESSAGE("A request like the result must not queue anything", !scheduler.IsPending(image));
    CPPUNIT_ASSERT_MESSAGE("The result must stay the same", scheduler.GetResult(image)->m_Surface == surface);
  }

  void LatestRequest_Wins_Success()
  {
    // Keeps the worker busy, so that the requests below meet a queue.
    auto busy = MakeLargeGroupImage();
    auto image = MakeSmallGroupImage();
    const auto busyReferences = busy->GetReferenceCount();
    const auto references = image->GetReferenceCount();
    Scheduler scheduler;

    scheduler.Request(busy, 0, true, { 1 });
    scheduler.Request(image, 0, true, { 1 });
    image->Modified();
    scheduler.Request(image, 0, true, { 1 });

    const auto latest = Scheduler::Stamp::Of(image, 0, true);

    CPPUNIT_ASSERT_MESSAGE("The extractions did not finish", this->WaitUntil([&]()
      {
        return IsReleased(busy, busyReferences) && IsReleased(image, references);
      }));

    const auto result = scheduler.GetResult(image);
    CPPUNIT_ASSERT_MESSAGE("A finished extraction has to leave a result", result.has_value());
    CPPUNIT_ASSERT_MESSAGE("The result has to be the one of the latest request", result->m_Stamp == latest);
  }

  void Forget_DiscardsARunningExtraction_Success()
  {
    auto image = MakeLargeGroupImage();
    const auto references = image->GetReferenceCount();
    Scheduler scheduler;

    scheduler.Request(image, 0, true, { 1 });
    LetTheWorkerStart();
    scheduler.Forget(image);

    CPPUNIT_ASSERT_MESSAGE("A forgotten group must not be pending", !scheduler.IsPending(image));

    CPPUNIT_ASSERT_MESSAGE("The extraction did not finish",
                           this->WaitUntil([&]() { return IsReleased(image, references); }));

    // A group goes away when it is forgotten. Its image may be destroyed and its address
    // reused by the next one, which must not find the old surface.
    CPPUNIT_ASSERT_MESSAGE("A forgotten group must not get a result", !scheduler.GetResult(image).has_value());
  }

  void DestructionDuringAnExtraction_ReleasesTheImageLater_Success()
  {
    auto image = MakeLargeGroupImage();
    const auto references = image->GetReferenceCount();

    {
      Scheduler scheduler;
      scheduler.Request(image, 0, true, { 1 });
      LetTheWorkerStart();
    }

    // The mapper, and with it the scheduler, goes away with the node, which must not wait
    // for an extraction that cannot be interrupted. The worker finishes on its own.
    CPPUNIT_ASSERT_MESSAGE("The image was never released",
                           this->WaitUntil([&]() { return IsReleased(image, references); }));
  }

  void InvalidRequest_Throws_Success()
  {
    auto image = MakeSmallGroupImage();
    Scheduler scheduler;

    CPPUNIT_ASSERT_THROW(scheduler.Request(nullptr, 0, true, { 1 }), mitk::Exception);
    CPPUNIT_ASSERT_THROW(scheduler.Request(image, 1, true, { 1 }), mitk::Exception);
    CPPUNIT_ASSERT_MESSAGE("An invalid request must not queue anything", !scheduler.IsPending(image));
  }

private:
  /**
   * Waits while this thread runs what is posted to it, which is what the thread that owns the
   * data does through its event loop.
   */
  bool WaitUntil(const std::function<bool()>& condition)
  {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(60);

    while (std::chrono::steady_clock::now() < deadline)
    {
      m_Dispatcher->Drain();

      if (condition())
        return true;

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return false;
  }

  FakeDispatcher::Pointer m_Dispatcher;
  std::unique_ptr<mitk::DataStorageService> m_Service;
  us::ServiceRegistration<mitk::IDataStorageService> m_Registration;
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSurfaceExtractionScheduler)
