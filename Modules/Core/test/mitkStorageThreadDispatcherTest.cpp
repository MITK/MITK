/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDataNode.h>
#include <mitkDataStorageService.h>
#include <mitkExceptionMacro.h>
#include <mitkImageToSurfaceFilter.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLog.h>
#include <mitkLogBackendBase.h>
#include <mitkLogMessage.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkStorageThreadDispatcherBase.h>

#include <mitkManualStorageThreadDispatcher.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceRegistration.h>

#include <vtkPolyData.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace
{
  /** \brief Collects what is logged for as long as it exists. */
  class LogCapture : public mitk::LogBackendBase
  {
  public:
    LogCapture() { mitk::RegisterBackend(this); }
    ~LogCapture() override { mitk::UnregisterBackend(this); }

    LogCapture(const LogCapture&) = delete;
    LogCapture& operator=(const LogCapture&) = delete;

    void ProcessMessage(const mitk::LogMessage& message) override
    {
      std::lock_guard<std::mutex> locked(m_Mutex);
      m_Messages.push_back(message.Message);
    }

    OutputType GetOutputType() const override { return OutputType::Other; }

    int CountContaining(const std::string& text) const
    {
      std::lock_guard<std::mutex> locked(m_Mutex);

      int count = 0;

      for (const auto& message : m_Messages)
      {
        if (message.find(text) != std::string::npos)
          ++count;
      }

      return count;
    }

  private:
    mutable std::mutex m_Mutex;
    std::vector<std::string> m_Messages;
  };
}

/**
 * Work that runs off the thread that owns the data hands its mutations back to
 * that thread, and says so when it touches something that should have been
 * prepared for it instead. Neither has any other enforcement, which is why the
 * contract is pinned here.
 */
class mitkStorageThreadDispatcherTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkStorageThreadDispatcherTestSuite);
  MITK_TEST(AddFromAnotherThread_IsHandedOverOnce_Success);
  MITK_TEST(AddOnTheOwningThread_IsNotHandedOver_Success);
  MITK_TEST(ExceptionFromAHandedOverTask_ReachesTheCaller_Success);
  MITK_TEST(UndeliverableHandOver_RunsAnywayAndSaysSo_Success);
  MITK_TEST(TouchingDataOnTheOwningThread_IsNotReported_Success);
  MITK_TEST(TouchingDataOffTheOwningThread_IsReported_Success);
  MITK_TEST(WithoutAnOwningThread_NothingIsReported_Success);
  MITK_TEST(PostOnTheOwningThread_RunsLater_Success);
  MITK_TEST(PostFromAnotherThread_RunsAndIsReleasedOnTheOwningThread_Success);
  MITK_TEST(PostWithoutAnOwningThread_IsRefused_Success);
  MITK_TEST(ImageToSurfaceOffTheOwningThread_LeavesTheSharedRepresentationAlone_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    m_Dispatcher = mitk::ManualStorageThreadDispatcher::New();

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

  void AddFromAnotherThread_IsHandedOverOnce_Success()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    auto node = mitk::DataNode::New();

    this->RunOnWorkerWhileDraining([&storage, &node]() { storage->Add(node); });

    // Once, not once per hop. What arrives on the owning thread calls Add()
    // again, and that call has to recognise where it is rather than hand the
    // very same mutation over a second time.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Adding from another thread is handed over exactly once",
                                 1,
                                 m_Dispatcher->HandedOver.load());

    CPPUNIT_ASSERT_MESSAGE("The node still has to end up in the storage", storage->Exists(node));
  }

  void AddOnTheOwningThread_IsNotHandedOver_Success()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    auto node = mitk::DataNode::New();

    storage->Add(node);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Nothing is handed over from the thread that owns the data",
                                 0,
                                 m_Dispatcher->HandedOver.load());

    CPPUNIT_ASSERT_MESSAGE("The node has to be in the storage", storage->Exists(node));
  }

  void ExceptionFromAHandedOverTask_ReachesTheCaller_Success()
  {
    auto reached = false;

    this->RunOnWorkerWhileDraining([&reached]()
      {
        try
        {
          mitk::DispatchToStorageThread([]() { mitkThrow() << "from the owning thread"; });
        }
        catch (const mitk::Exception&)
        {
          reached = true;
        }
      });

    // Left to itself it would unwind the event loop it ran on, where nobody is
    // in a position to do anything about it.
    CPPUNIT_ASSERT_MESSAGE("An exception has to come back to the thread that asked", reached);
  }

  void UndeliverableHandOver_RunsAnywayAndSaysSo_Success()
  {
    m_Dispatcher->Deliverable = false;

    LogCapture capture;

    std::atomic<bool> ran{ false };

    std::thread worker([this, &ran]() { m_Dispatcher->Execute([&ran]() { ran = true; }); });
    worker.join();

    // Dropping it would turn a mutation into a silent no-op, which is worse
    // than running it in the wrong place. Neither is right, so it is reported.
    CPPUNIT_ASSERT_MESSAGE("A hand-over that cannot be delivered still has to run", ran.load());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Running it on the calling thread has to be reported",
                                 1,
                                 capture.CountContaining("could not be handed to the thread"));
  }

  void TouchingDataOnTheOwningThread_IsNotReported_Success()
  {
    LogCapture capture;

    mitk::WarnIfOffStorageThread("Building something");

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The owning thread is where this belongs",
                                 0,
                                 capture.CountContaining("Building something"));
  }

  void TouchingDataOffTheOwningThread_IsReported_Success()
  {
    LogCapture capture;

    std::thread worker([]() { mitk::WarnIfOffStorageThread("Building something"); });
    worker.join();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("A call site that forgot to prepare its data has to be findable",
                                 1,
                                 capture.CountContaining("Building something"));
  }

  void WithoutAnOwningThread_NothingIsReported_Success()
  {
    m_Service->SetDispatcher(nullptr);

    LogCapture capture;

    auto handedOver = true;

    std::thread worker([&handedOver]()
      {
        mitk::WarnIfOffStorageThread("Building something");
        handedOver = mitk::DispatchToStorageThread([]() {});
      });

    worker.join();

    // The ordinary situation in a command line tool or a test: no thread owns
    // the data, so there is nothing to hand over to and nothing to race with.
    CPPUNIT_ASSERT_MESSAGE("There is nothing to hand over to", !handedOver);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Warning without an owning thread would be noise",
                                 0,
                                 capture.CountContaining("Building something"));
  }

  void PostOnTheOwningThread_RunsLater_Success()
  {
    auto ran = false;

    CPPUNIT_ASSERT_MESSAGE("There has to be a thread to queue to",
                           mitk::PostToStorageThread([&ran]() { ran = true; }));

    // Unlike a hand-over, which runs at once when it already is on the owning
    // thread. Callers post to get out of whatever they are in first.
    CPPUNIT_ASSERT_MESSAGE("A posted task must not run inline", !ran);

    m_Dispatcher->Drain();

    CPPUNIT_ASSERT_MESSAGE("A posted task has to run once the owning thread gets to it", ran);
  }

  void PostFromAnotherThread_RunsAndIsReleasedOnTheOwningThread_Success()
  {
    const auto owningThread = std::this_thread::get_id();

    auto queued = false;
    std::thread::id ranOn;
    std::thread::id releasedOn;

    std::thread worker([&queued, &ranOn, &releasedOn]()
      {
        // Stands in for what a worker hands back because it must not be
        // released anywhere else, such as an image that observers watch.
        std::shared_ptr<int> held(new int(0), [&releasedOn](int* value)
          {
            releasedOn = std::this_thread::get_id();
            delete value;
          });

        queued = mitk::PostToStorageThread([&ranOn, held = std::move(held)]()
          {
            ranOn = std::this_thread::get_id();
          });
      });

    worker.join();
    m_Dispatcher->Drain();

    CPPUNIT_ASSERT_MESSAGE("There has to be a thread to queue to", queued);
    CPPUNIT_ASSERT_MESSAGE("A posted task has to run on the owning thread", ranOn == owningThread);
    CPPUNIT_ASSERT_MESSAGE("What a posted task holds has to be released on the owning thread",
                           releasedOn == owningThread);
  }

  void PostWithoutAnOwningThread_IsRefused_Success()
  {
    m_Service->SetDispatcher(nullptr);

    auto ran = false;

    CPPUNIT_ASSERT_MESSAGE("There is nothing to queue to",
                           !mitk::PostToStorageThread([&ran]() { ran = true; }));

    // Running it here instead would re-enter a caller that relies on having
    // returned first.
    CPPUNIT_ASSERT_MESSAGE("A task that could not be queued must not run", !ran);
  }

  void ImageToSurfaceOffTheOwningThread_LeavesTheSharedRepresentationAlone_Success()
  {
    // A cube of ones in a volume of zeros, so that there is a surface to extract.
    auto image = mitk::Image::New();
    unsigned int dimensions[3] = { 10, 10, 10 };
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    {
      mitk::ImageWriteAccessor accessor(image);
      auto* pixels = static_cast<unsigned char*>(accessor.GetData());
      std::fill_n(pixels, 1000, static_cast<unsigned char>(0));

      for (int z = 3; z < 7; ++z)
        for (int y = 3; y < 7; ++y)
          for (int x = 3; x < 7; ++x)
            pixels[(z * 10 + y) * 10 + x] = 1;
    }

    LogCapture capture;

    mitk::Surface::Pointer surface;

    RunOnWorkerWhileDraining([&image, &surface]()
      {
        auto filter = mitk::ImageToSurfaceFilter::New();
        filter->SetInput(image);
        filter->SetThreshold(0.5);
        filter->Update();

        surface = filter->GetOutput();
        surface->DisconnectPipeline();
      });

    CPPUNIT_ASSERT_MESSAGE("No surface was extracted", surface->GetVtkPolyData()->GetNumberOfPoints() > 0);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The filter built the representation that the mappers share",
                                 0,
                                 capture.CountContaining("Building the VTK representation"));
  }

private:
  /**
   * Runs the task on a worker while this thread takes what is handed to it,
   * which is what the thread that owns the data does through its event loop.
   * Waiting any other way deadlocks against the worker.
   */
  void RunOnWorkerWhileDraining(const std::function<void()>& task)
  {
    std::atomic<bool> finished{ false };
    std::exception_ptr thrown;

    std::thread worker([&task, &finished, &thrown]()
      {
        try
        {
          task();
        }
        catch (...)
        {
          thrown = std::current_exception();
        }

        finished = true;
      });

    while (!finished)
    {
      m_Dispatcher->Drain();
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    m_Dispatcher->Drain();
    worker.join();

    if (thrown)
      std::rethrow_exception(thrown);
  }

  mitk::ManualStorageThreadDispatcher::Pointer m_Dispatcher;
  std::unique_ptr<mitk::DataStorageService> m_Service;
  us::ServiceRegistration<mitk::IDataStorageService> m_Registration;
};

MITK_TEST_SUITE_REGISTRATION(mitkStorageThreadDispatcher)
