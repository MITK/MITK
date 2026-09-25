/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkProgressTask.h>

#include <mitkCoreServices.h>
#include <mitkIProgressListener.h>
#include <mitkIProgressService.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace
{
  class RecordingListener : public mitk::IProgressListener
  {
  public:
    void OnTaskUpdated(const mitk::ProgressTaskInfo& info) override
    {
      std::scoped_lock lock(m_Mutex);
      m_Snapshots.push_back(info);
    }

    std::vector<mitk::ProgressTaskInfo> GetSnapshots(mitk::ProgressTaskId id) const
    {
      std::scoped_lock lock(m_Mutex);

      std::vector<mitk::ProgressTaskInfo> snapshots;

      for (const auto& info : m_Snapshots)
      {
        if (info.Id == id)
          snapshots.push_back(info);
      }

      return snapshots;
    }

    std::size_t GetCount() const
    {
      std::scoped_lock lock(m_Mutex);
      return m_Snapshots.size();
    }

  private:
    mutable std::mutex m_Mutex;
    std::vector<mitk::ProgressTaskInfo> m_Snapshots;
  };
}

class mitkProgressTaskTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkProgressTaskTestSuite);
  MITK_TEST(TaskLifecycle_Success);
  MITK_TEST(ConcurrentTasks_AreIndependent_Success);
  MITK_TEST(Progress_ClampsAtTotal_Success);
  MITK_TEST(AddStepsToDo_ExtendsTotal_Success);
  MITK_TEST(SetProgress_IsAbsolute_Success);
  MITK_TEST(SetProgress_NeverGoesBackwards_Success);
  MITK_TEST(IndeterminateTask_ReportsNoSteps_Success);
  MITK_TEST(TaskUnwoundByException_Finishes_Success);
  MITK_TEST(MovedTask_FinishesOnce_Success);
  MITK_TEST(Cancel_OnlyAffectsCancelableTasks_Success);
  MITK_TEST(ListenerAddedMidFlight_LearnsRunningTasks_Success);
  MITK_TEST(RemovedListener_IsNotNotified_Success);
  MITK_TEST(ConcurrentReporting_KeepsSequencesMonotonic_Success);
  MITK_TEST(ReportingTask_RaisesNoNotification_Success);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    m_Service = mitk::CoreServices::GetProgressService();
    CPPUNIT_ASSERT_MESSAGE("Progress service is registered by the core activator", nullptr != m_Service);

    m_Listener = new RecordingListener;
    m_Service->AddListener(m_Listener);
  }

  void tearDown() override
  {
    // CppUnit runs tearDown() even when the assertion in setUp() threw, so
    // there may be no service here. Dereferencing it then would take the whole
    // test driver down instead of failing the one case.
    if (nullptr != m_Service)
    {
      m_Service->RemoveListener(m_Listener);

      mitk::CoreServices::Unget(m_Service);
      m_Service = nullptr;
    }

    delete m_Listener;
    m_Listener = nullptr;
  }

  void TaskLifecycle_Success()
  {
    mitk::ProgressTaskId id = 0;

    {
      mitk::ProgressTask task("Loading files", 4);
      id = task.GetId();
      CPPUNIT_ASSERT(0 != id);

      task.Progress(2);
    }

    const auto snapshots = m_Listener->GetSnapshots(id);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), snapshots.size());

    CPPUNIT_ASSERT_EQUAL(std::string("Loading files"), snapshots.front().Name);
    CPPUNIT_ASSERT_EQUAL(4u, snapshots.front().StepsToDo);
    CPPUNIT_ASSERT_EQUAL(0u, snapshots.front().Progress);
    CPPUNIT_ASSERT(!snapshots.front().Finished);

    CPPUNIT_ASSERT_EQUAL(2u, snapshots[1].Progress);

    CPPUNIT_ASSERT(snapshots.back().Finished);
    CPPUNIT_ASSERT_EQUAL(4u, snapshots.back().Progress);
  }

  void ConcurrentTasks_AreIndependent_Success()
  {
    mitk::ProgressTask first("First", 10);
    mitk::ProgressTask second("Second", 100);

    first.Progress(3);
    second.Progress(50);
    first.Progress(2);

    CPPUNIT_ASSERT(first.GetId() != second.GetId());

    const auto firstSnapshots = m_Listener->GetSnapshots(first.GetId());
    const auto secondSnapshots = m_Listener->GetSnapshots(second.GetId());

    CPPUNIT_ASSERT_EQUAL(10u, firstSnapshots.back().StepsToDo);
    CPPUNIT_ASSERT_EQUAL(5u, firstSnapshots.back().Progress);

    CPPUNIT_ASSERT_EQUAL(100u, secondSnapshots.back().StepsToDo);
    CPPUNIT_ASSERT_EQUAL(50u, secondSnapshots.back().Progress);
  }

  void Progress_ClampsAtTotal_Success()
  {
    mitk::ProgressTask task("Clamping", 3);
    task.Progress(10);

    CPPUNIT_ASSERT_EQUAL(3u, m_Listener->GetSnapshots(task.GetId()).back().Progress);
  }

  void AddStepsToDo_ExtendsTotal_Success()
  {
    mitk::ProgressTask task("Growing", 2);
    task.Progress(2);
    task.AddStepsToDo(3);

    const auto snapshot = m_Listener->GetSnapshots(task.GetId()).back();
    CPPUNIT_ASSERT_EQUAL(5u, snapshot.StepsToDo);
    CPPUNIT_ASSERT_EQUAL(2u, snapshot.Progress);
  }

  void SetProgress_IsAbsolute_Success()
  {
    mitk::ProgressTask task("Absolute", 100);
    task.Progress(10);
    task.SetProgress(42);

    CPPUNIT_ASSERT_EQUAL(42u, m_Listener->GetSnapshots(task.GetId()).back().Progress);
  }

  void SetProgress_NeverGoesBackwards_Success()
  {
    // Phases that map onto shares of one budget report an absolute value
    // each, and they do not always end in the order they began. A listener
    // must never be shown the bar falling back.
    mitk::ProgressTask task("Backwards", 100);
    task.SetProgress(90);
    task.SetProgress(20);

    const auto snapshots = m_Listener->GetSnapshots(task.GetId());
    CPPUNIT_ASSERT_EQUAL(90u, snapshots.back().Progress);

    // The attempt is still published, so that a name change or a grown step
    // count travelling with it is not lost.
    CPPUNIT_ASSERT(snapshots.size() > 2);
  }

  void IndeterminateTask_ReportsNoSteps_Success()
  {
    mitk::ProgressTask task("Unknown extent");
    task.Progress();

    const auto snapshot = m_Listener->GetSnapshots(task.GetId()).back();
    CPPUNIT_ASSERT_EQUAL(mitk::ProgressTask::Indeterminate, snapshot.StepsToDo);
    CPPUNIT_ASSERT_EQUAL(0u, snapshot.Progress);
  }

  void TaskUnwoundByException_Finishes_Success()
  {
    mitk::ProgressTaskId id = 0;

    try
    {
      mitk::ProgressTask task("Throwing", 10);
      id = task.GetId();
      task.Progress();

      throw std::runtime_error("Simulated failure");
    }
    catch (const std::runtime_error&)
    {
    }

    CPPUNIT_ASSERT(m_Listener->GetSnapshots(id).back().Finished);
  }

  void MovedTask_FinishesOnce_Success()
  {
    mitk::ProgressTaskId id = 0;
    std::size_t finishedCount = 0;

    {
      mitk::ProgressTask task("Moved", 2);
      id = task.GetId();

      mitk::ProgressTask moved(std::move(task));
      CPPUNIT_ASSERT_EQUAL(id, moved.GetId());

      moved.Progress();
    }

    for (const auto& snapshot : m_Listener->GetSnapshots(id))
    {
      if (snapshot.Finished)
        ++finishedCount;
    }

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), finishedCount);
  }

  void Cancel_OnlyAffectsCancelableTasks_Success()
  {
    mitk::ProgressTask cancelable("Cancelable", 10, true);
    mitk::ProgressTask plain("Plain", 10);

    m_Service->RequestCancel(cancelable.GetId());
    m_Service->RequestCancel(plain.GetId());

    CPPUNIT_ASSERT(cancelable.IsCancelRequested());
    CPPUNIT_ASSERT(!plain.IsCancelRequested());

    CPPUNIT_ASSERT(m_Listener->GetSnapshots(cancelable.GetId()).back().CancelRequested);
    CPPUNIT_ASSERT(!m_Listener->GetSnapshots(plain.GetId()).back().CancelRequested);
  }

  void ListenerAddedMidFlight_LearnsRunningTasks_Success()
  {
    mitk::ProgressTask task("Already running", 10);
    task.Progress(4);

    RecordingListener latecomer;
    m_Service->AddListener(&latecomer);

    const auto snapshots = latecomer.GetSnapshots(task.GetId());
    m_Service->RemoveListener(&latecomer);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), snapshots.size());
    CPPUNIT_ASSERT_EQUAL(4u, snapshots.front().Progress);
  }

  void RemovedListener_IsNotNotified_Success()
  {
    RecordingListener temporary;
    m_Service->AddListener(&temporary);
    m_Service->RemoveListener(&temporary);

    mitk::ProgressTask task("Unobserved", 2);
    task.Progress();

    CPPUNIT_ASSERT_EQUAL(std::size_t(0), temporary.GetCount());
  }

  void ConcurrentReporting_KeepsSequencesMonotonic_Success()
  {
    constexpr unsigned int threadCount = 4;
    constexpr unsigned int stepCount = 50;

    std::mutex idMutex;
    std::vector<mitk::ProgressTaskId> ids;
    std::vector<std::thread> threads;

    for (unsigned int i = 0; i < threadCount; ++i)
    {
      threads.emplace_back([&idMutex, &ids, i]() {
        mitk::ProgressTask task("Worker " + std::to_string(i), stepCount);

        {
          std::scoped_lock lock(idMutex);
          ids.push_back(task.GetId());
        }

        for (unsigned int step = 0; step < stepCount; ++step)
          task.Progress();
      });
    }

    for (auto& thread : threads)
      thread.join();

    CPPUNIT_ASSERT_EQUAL(std::size_t(threadCount), ids.size());

    for (const auto id : ids)
    {
      const auto snapshots = m_Listener->GetSnapshots(id);
      CPPUNIT_ASSERT_EQUAL(std::size_t(stepCount + 2), snapshots.size());

      std::uint64_t lastSequence = 0;
      unsigned int lastProgress = 0;

      for (const auto& snapshot : snapshots)
      {
        CPPUNIT_ASSERT(snapshot.Sequence > lastSequence);
        CPPUNIT_ASSERT(snapshot.Progress >= lastProgress);

        lastSequence = snapshot.Sequence;
        lastProgress = snapshot.Progress;
      }

      CPPUNIT_ASSERT(snapshots.back().Finished);
      CPPUNIT_ASSERT_EQUAL(stepCount, snapshots.back().Progress);
    }
  }

  void ReportingTask_RaisesNoNotification_Success()
  {
    std::vector<float> reported;

    {
      mitk::ProgressTask task([&reported](float progress) { reported.push_back(progress); }, 4);

      CPPUNIT_ASSERT_MESSAGE("A task reporting to its caller has no id of its own", 0 == task.GetId());

      task.Progress(2);
    }

    CPPUNIT_ASSERT_MESSAGE("Nothing is shown to the user", 0 == m_Listener->GetCount());

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), reported.size());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, reported.front(), 0.001);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, reported.back(), 0.001);
  }

private:
  mitk::IProgressService* m_Service = nullptr;
  RecordingListener* m_Listener = nullptr;
};

MITK_TEST_SUITE_REGISTRATION(mitkProgressTask)
