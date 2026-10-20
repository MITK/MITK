/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkSegWithPreviewTool.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkToolManager.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

namespace
{
  // Stands in for the event loop of the GUI thread, which is the thread of the test.
  class GUIThreadQueue
  {
  public:
    mitk::SegWithPreviewTool::GUIThreadPoster GetPoster()
    {
      return [this](std::function<void()> function) {
        std::lock_guard lock(m_Mutex);
        m_Functions.push_back(std::move(function));
        m_Condition.notify_all();
      };
    }

    /** Runs what is posted until the condition holds, for ten seconds at most. */
    bool ProcessUntil(const std::function<bool()>& condition)
    {
      const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);

      while (!condition())
      {
        std::function<void()> function;

        {
          std::unique_lock lock(m_Mutex);

          if (!m_Condition.wait_until(lock, deadline, [this] { return !m_Functions.empty(); }))
            return false;

          function = std::move(m_Functions.front());
          m_Functions.pop_front();
        }

        function();
      }

      return true;
    }

    /** Runs what was posted so far. */
    void ProcessAll()
    {
      while (true)
      {
        std::function<void()> function;

        {
          std::lock_guard lock(m_Mutex);

          if (m_Functions.empty())
            return;

          function = std::move(m_Functions.front());
          m_Functions.pop_front();
        }

        function();
      }
    }

  private:
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    std::deque<std::function<void()>> m_Functions;
  };

  // Runs the work of every time step off the GUI thread, or at once.
  class AsyncTestTool : public mitk::SegWithPreviewTool
  {
  public:
    mitkClassMacro(AsyncTestTool, mitk::SegWithPreviewTool);
    itkFactorylessNewMacro(Self);

    using Superclass::SetToolManager;

    const char* GetName() const override
    {
      return "AsyncTestTool";
    }

    bool Async = true;
    bool WorkThrows = false;
    bool ConfirmCleanUpThrows = false;

    /** Called by the work, for example to wait for the test. */
    std::function<void()> InWork;

    int Updates = 0;
    int Dones = 0;
    int OtherDones = 0;
    int Confirmations = 0;
    int DonesBeforeConfirmation = -1;
    std::thread::id WorkThread;
    std::thread::id DoneThread;

    /** Starts work that is not an update, like the start of a model. */
    void StartOtherWork()
    {
      this->RunOffGUIThread(
        [this]() {
          if (InWork)
            InWork();
        },
        [this](std::exception_ptr) {
          ++OtherDones;
        });
    }

  protected:
    AsyncTestTool()
      : SegWithPreviewTool(true)
    {
      this->KeepActiveAfterAcceptOn();
      this->SetLabelTransferScope(LabelTransferScope::AllLabels);
      this->SetLabelTransferMode(LabelTransferMode::AddLabel);
    }

    ~AsyncTestTool() override
    {
      // The work uses InWork and WorkThrows.
      this->WaitForWorkOffGUIThread();
    }

    void DoUpdatePreview(const mitk::Image*, const mitk::Image*, mitk::MultiLabelSegmentation*, mitk::TimeStepType) override
    {
      ++Updates;

      if (!Async)
      {
        ++Dones;
        DoneThread = std::this_thread::get_id();
        return;
      }

      this->RunOffGUIThread(
        [this]() {
          WorkThread = std::this_thread::get_id();

          if (InWork)
            InWork();

          if (WorkThrows)
            mitkThrow() << "The work failed.";
        },
        [this](std::exception_ptr error) {
          ++Dones;
          DoneThread = std::this_thread::get_id();

          if (error != nullptr)
            std::rethrow_exception(error);
        });
    }

    void ConfirmCleanUp() override
    {
      ++Confirmations;
      DonesBeforeConfirmation = Dones;

      if (ConfirmCleanUpThrows)
        mitkThrow() << "The confirmation failed.";
    }
  };
}

class mitkSegWithPreviewToolAsyncTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSegWithPreviewToolAsyncTestSuite);
  MITK_TEST(TestWithoutPosterWorkRunsAtOnce);
  MITK_TEST(TestWorkRunsOffGUIThread);
  MITK_TEST(TestUpdateWhileWorkRunsIsIgnored);
  MITK_TEST(TestUpdateWhileOtherWorkRunsIsIgnored);
  MITK_TEST(TestWorkErrorIsReported);
  MITK_TEST(TestConfirmationFollowsUpdate);
  MITK_TEST(TestFailedConfirmationAfterUpdateIsReported);
  MITK_TEST(TestFailedUpdateOfConfirmationTransfersNothing);
  MITK_TEST(TestConfirmationWhileUpdatingIsRefused);
  MITK_TEST(TestDeactivationAbandonsUpdate);
  MITK_TEST(TestDestroyedToolIgnoresItsDone);
  CPPUNIT_TEST_SUITE_END();

  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ToolManager;
  GUIThreadQueue m_Queue;
  AsyncTestTool::Pointer m_Tool;
  std::vector<bool> m_BusyStates;
  std::vector<std::string> m_Errors;

  void OnBusy(bool isBusy)
  {
    m_BusyStates.push_back(isBusy);
  }

  void OnError(std::string message)
  {
    m_Errors.push_back(message);
  }

  void Activate(bool withPoster)
  {
    if (withPoster)
      m_Tool->SetGUIThreadPoster(m_Queue.GetPoster());

    m_Tool->SetToolManager(m_ToolManager);
    m_Tool->CurrentlyBusy += mitk::MessageDelegate1<mitkSegWithPreviewToolAsyncTestSuite, bool>(this, &mitkSegWithPreviewToolAsyncTestSuite::OnBusy);
    m_Tool->ErrorMessage += mitk::MessageDelegate1<mitkSegWithPreviewToolAsyncTestSuite, std::string>(this, &mitkSegWithPreviewToolAsyncTestSuite::OnError);
    m_Tool->Activated();
  }

  bool WaitUntilIdle()
  {
    return m_Queue.ProcessUntil([this] { return !m_Tool->IsUpdating(); });
  }

public:
  void setUp() override
  {
    // Two time steps, so that confirming all of them needs another update.
    unsigned int dimensions[] = { 4, 4, 4, 2 };

    auto referenceImage = mitk::Image::New();
    referenceImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 4, dimensions);

    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(referenceImage);
    mitk::Color color;
    color.Set(1.0f, 0.0f, 0.0f);
    segmentation->AddLabel("Label", color, 0);

    auto referenceNode = mitk::DataNode::New();
    referenceNode->SetData(referenceImage);

    auto workingNode = mitk::DataNode::New();
    workingNode->SetData(segmentation);

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_DataStorage->Add(referenceNode);
    m_DataStorage->Add(workingNode);

    m_ToolManager = mitk::ToolManager::New(m_DataStorage);
    m_ToolManager->SetReferenceData(referenceNode);
    m_ToolManager->SetWorkingData(workingNode);

    m_Tool = AsyncTestTool::New();
    m_BusyStates.clear();
    m_Errors.clear();
  }

  void tearDown() override
  {
    if (m_Tool.IsNotNull())
    {
      m_Tool->InWork = nullptr;
      this->WaitUntilIdle();
      m_Tool->Deactivated();
      m_Tool = nullptr;
    }

    m_Queue.ProcessAll();
    m_ToolManager = nullptr;
    m_DataStorage = nullptr;
  }

  void TestWithoutPosterWorkRunsAtOnce()
  {
    this->Activate(false);
    m_Tool->UpdatePreview();

    CPPUNIT_ASSERT(!m_Tool->IsUpdating());
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Dones);
    CPPUNIT_ASSERT(m_Tool->WorkThread == std::this_thread::get_id());
    CPPUNIT_ASSERT((m_BusyStates == std::vector<bool>{ true, false }));
  }

  void TestWorkRunsOffGUIThread()
  {
    this->Activate(true);
    m_Tool->UpdatePreview();

    // The GUI thread is free again before the work has finished.
    CPPUNIT_ASSERT(m_Tool->IsUpdating());
    CPPUNIT_ASSERT_EQUAL(0, m_Tool->Dones);
    CPPUNIT_ASSERT((m_BusyStates == std::vector<bool>{ true }));

    CPPUNIT_ASSERT(this->WaitUntilIdle());
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Dones);
    CPPUNIT_ASSERT(m_Tool->WorkThread != std::this_thread::get_id());
    CPPUNIT_ASSERT(m_Tool->DoneThread == std::this_thread::get_id());
    CPPUNIT_ASSERT((m_BusyStates == std::vector<bool>{ true, false }));
    CPPUNIT_ASSERT(m_Errors.empty());
  }

  void TestUpdateWhileWorkRunsIsIgnored()
  {
    std::atomic<bool> release = false;
    m_Tool->InWork = [&release] {
      while (!release)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };

    this->Activate(true);
    m_Tool->UpdatePreview();
    m_Tool->UpdatePreview();

    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Updates);

    release = true;
    CPPUNIT_ASSERT(this->WaitUntilIdle());
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Updates);
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Dones);
  }

  void TestUpdateWhileOtherWorkRunsIsIgnored()
  {
    std::atomic<bool> release = false;
    m_Tool->InWork = [&release] {
      while (!release)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };

    this->Activate(true);
    m_Tool->StartOtherWork();

    // The work of an update could not start meanwhile.
    m_Tool->UpdatePreview();
    CPPUNIT_ASSERT_THROW(m_Tool->ConfirmSegmentation(), mitk::Exception);

    CPPUNIT_ASSERT_EQUAL(0, m_Tool->Updates);
    CPPUNIT_ASSERT(!m_Tool->IsUpdating());
    CPPUNIT_ASSERT(m_BusyStates.empty());

    release = true;
    CPPUNIT_ASSERT(m_Queue.ProcessUntil([this] { return m_Tool->OtherDones > 0; }));
  }

  void TestWorkErrorIsReported()
  {
    m_Tool->WorkThrows = true;

    this->Activate(true);
    m_Tool->UpdatePreview();

    CPPUNIT_ASSERT(this->WaitUntilIdle());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), m_Errors.size());
    CPPUNIT_ASSERT((m_BusyStates == std::vector<bool>{ true, false }));
  }

  void TestConfirmationFollowsUpdate()
  {
    this->Activate(true);
    m_Tool->UpdatePreview();
    CPPUNIT_ASSERT(this->WaitUntilIdle());
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Updates);

    // The lazy tool computed one time step, and confirming all of them
    // updates both first.
    m_Tool->SetCreateAllTimeSteps(true);
    m_Tool->ConfirmSegmentation();

    CPPUNIT_ASSERT(m_Tool->IsUpdating());
    CPPUNIT_ASSERT_EQUAL(0, m_Tool->Confirmations);

    CPPUNIT_ASSERT(m_Queue.ProcessUntil([this] { return m_Tool->Confirmations > 0; }));
    CPPUNIT_ASSERT_EQUAL(3, m_Tool->Updates);
    CPPUNIT_ASSERT_EQUAL(3, m_Tool->DonesBeforeConfirmation);
    CPPUNIT_ASSERT(!m_Tool->IsUpdating());
  }

  void TestFailedConfirmationAfterUpdateIsReported()
  {
    this->Activate(true);
    m_Tool->UpdatePreview();
    CPPUNIT_ASSERT(this->WaitUntilIdle());

    // The confirmation of all time steps follows an update, so what it throws
    // no longer reaches the caller of ConfirmSegmentation().
    m_Tool->ConfirmCleanUpThrows = true;
    m_Tool->SetCreateAllTimeSteps(true);
    m_Tool->ConfirmSegmentation();

    CPPUNIT_ASSERT(m_Queue.ProcessUntil([this] { return m_Tool->Confirmations > 0; }));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), m_Errors.size());
  }

  void TestFailedUpdateOfConfirmationTransfersNothing()
  {
    this->Activate(true);
    m_Tool->UpdatePreview();
    CPPUNIT_ASSERT(this->WaitUntilIdle());

    // The update that the confirmation of all time steps needs fails, so
    // there is no result to transfer.
    m_Tool->WorkThrows = true;
    m_Tool->SetCreateAllTimeSteps(true);
    m_Tool->ConfirmSegmentation();

    CPPUNIT_ASSERT(this->WaitUntilIdle());
    m_Queue.ProcessAll();

    CPPUNIT_ASSERT_EQUAL(0, m_Tool->Confirmations);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), m_Errors.size());
  }

  void TestConfirmationWhileUpdatingIsRefused()
  {
    std::atomic<bool> release = false;
    m_Tool->InWork = [&release] {
      while (!release)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };

    this->Activate(true);
    m_Tool->UpdatePreview();

    CPPUNIT_ASSERT_THROW(m_Tool->ConfirmSegmentation(), mitk::Exception);

    release = true;
    CPPUNIT_ASSERT(this->WaitUntilIdle());
    CPPUNIT_ASSERT_EQUAL(0, m_Tool->Confirmations);
  }

  void TestDeactivationAbandonsUpdate()
  {
    std::atomic<bool> release = false;
    m_Tool->InWork = [&release] {
      while (!release)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };

    this->Activate(true);

    // Both time steps, while the first one computes the tool is switched off
    // and on again, as when the next task of a task list is loaded.
    m_Tool->UpdatePreview(true);
    m_Tool->Deactivated();
    CPPUNIT_ASSERT_NO_THROW(m_Tool->Activated());
    CPPUNIT_ASSERT(m_Tool->IsUpdating());

    release = true;
    CPPUNIT_ASSERT(this->WaitUntilIdle());

    // The second time step is not computed for the abandoned update.
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Updates);
    CPPUNIT_ASSERT_EQUAL(1, m_Tool->Dones);
    CPPUNIT_ASSERT((m_BusyStates == std::vector<bool>{ true, false }));

    // The new activation updates as usual.
    m_Tool->UpdatePreview();
    CPPUNIT_ASSERT(this->WaitUntilIdle());
    CPPUNIT_ASSERT_EQUAL(2, m_Tool->Updates);
    CPPUNIT_ASSERT(m_Errors.empty());
  }

  void TestDestroyedToolIgnoresItsDone()
  {
    std::atomic<bool> release = false;
    m_Tool->InWork = [&release] {
      while (!release)
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };

    this->Activate(true);
    m_Tool->UpdatePreview();

    // The destructor waits for the work, and what the work posted afterwards
    // finds the tool gone.
    std::thread releaser([&release] {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      release = true;
    });

    m_Tool->Deactivated();
    m_Tool = nullptr;
    releaser.join();

    m_Queue.ProcessAll();
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSegWithPreviewToolAsync)
