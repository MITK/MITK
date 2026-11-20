/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkErrorResponse.h>
#include <mitkImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkRestSegmentationJobs.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkToolManager.h>

#include <QCoreApplication>

#include <atomic>

namespace
{
  /**
   * Puts five voxels of the last label of the preview into it, which is the label "toolresult" that the tool
   * adds as the working segmentation has no active label. Its work, if asynchronous, waits for the gate.
   */
  class FakeTool : public mitk::SegWithPreviewTool
  {
  public:
    mitkClassMacro(FakeTool, mitk::SegWithPreviewTool);
    itkFactorylessNewMacro(Self);
    using Superclass::SetToolManager;
    using Superclass::SetTransfersAllPreviewGroups;

    const char* GetName() const override { return "FakeTool"; }

    bool Async = false;
    std::atomic<bool> Open = true;

    void OpenGate(bool open)
    {
      this->Open = open;
      this->Open.notify_all();
    }

  protected:
    FakeTool()
      : SegWithPreviewTool(true)
    {
      this->SetLabelTransferScope(LabelTransferScope::AllLabels);
      this->SetLabelTransferMode(LabelTransferMode::AddLabel);
    }

    ~FakeTool() override
    {
      this->OpenGate(true);
      this->WaitForWorkOffGUIThread();
    }

    void DoUpdatePreview(const mitk::Image*, const mitk::Image*, mitk::MultiLabelSegmentation* preview, mitk::TimeStepType) override
    {
      mitk::ImageWriteAccessor accessor(preview->GetGroupImage(0));
      auto* pixels = static_cast<mitk::MultiLabelSegmentation::LabelValueType*>(accessor.GetData());
      std::fill(pixels, pixels + 5, preview->GetAllLabelValues().back());

      if (this->Async)
        this->RunOffGUIThread([this]() { this->Open.wait(false); }, [](std::exception_ptr) {});
    }
  };
}

class mitkRestSegmentationJobsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestSegmentationJobsTestSuite);
  MITK_TEST(TestFailRestJobTakesTheMessageOfAnyException);
  MITK_TEST(TestPromptOutcomeReportsOnceBothArePresent);
  MITK_TEST(TestBusyToolIsRefusedAndWaitedFor);
  MITK_TEST(TestConfirmationFailureTellsTheOutcome);
  MITK_TEST(TestWaitingForAStartToEnd);
  MITK_TEST(TestDeactivatedToolFailsItsJob);
  MITK_TEST(TestPreviewLabelsAndVoxels);
  CPPUNIT_TEST_SUITE_END();

  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ToolManager;
  FakeTool::Pointer m_Tool;

  /** The thread of the test is the GUI thread; what the tool posts to it and the timers run here. */
  static void PumpUntil(const std::function<bool()>& condition)
  {
    while (!condition())
      QCoreApplication::processEvents();
  }

public:
  void setUp() override
  {
    static int argc = 1;
    static char name[] = "test";
    static char* argv[] = { name, nullptr };
    static QCoreApplication application(argc, argv);

    unsigned int dimensions[] = { 4, 4, 4 };
    auto reference = mitk::Image::New();
    reference->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);
    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(reference);
    segmentation->AddLabel("Label", mitk::Color(), 0);

    auto referenceNode = mitk::DataNode::New();
    referenceNode->SetData(reference);
    auto workingNode = mitk::DataNode::New();
    workingNode->SetData(segmentation);

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_DataStorage->Add(referenceNode);
    m_DataStorage->Add(workingNode);
    m_ToolManager = mitk::ToolManager::New(m_DataStorage);
    m_ToolManager->SetReferenceData(referenceNode);
    m_ToolManager->SetWorkingData(workingNode);

    m_Tool = FakeTool::New();
    m_Tool->SetGUIThreadPoster([](std::function<void()> function) {
      QMetaObject::invokeMethod(QCoreApplication::instance(), std::move(function), Qt::QueuedConnection);
    });
    m_Tool->SetToolManager(m_ToolManager);
    m_Tool->Activated();
  }

  void tearDown() override
  {
    m_Tool->OpenGate(true);
    this->PumpUntil([this] { return !m_Tool->IsUpdating(); });
    m_Tool->Deactivated();
    m_Tool = nullptr;
  }

  void TestFailRestJobTakesTheMessageOfAnyException()
  {
    const mitk::RestException restException(mitk::ErrorResponse::InvalidRequest("Bad request.", "/p"));
    const std::vector<std::pair<std::exception_ptr, std::string>> cases = {
      { std::make_exception_ptr(restException), restException.what() },
      { mitk::MakeRestError("The description."), "The description." },
      { std::make_exception_ptr(std::runtime_error("Plain.")), "Plain." },
      { std::make_exception_ptr(42), "An unknown error occurred." }
    };

    for (const auto& [error, message] : cases)
    {
      mitk::RestJob job("id", "work");
      mitk::FailRestJob(job, error);
      CPPUNIT_ASSERT(job.GetState() == mitk::RestJob::State::Failed);
      CPPUNIT_ASSERT_EQUAL(message, job.ToJson()["error"].get<std::string>());
    }
  }

  void TestPromptOutcomeReportsOnceBothArePresent()
  {
    auto job = std::make_shared<mitk::RestJob>("id", "prompt");
    mitk::RestPromptOutcome outcome;
    outcome.Result = { { "done", true } };

    // The prompt can end before its job exists, or after.
    outcome.Ended = true;
    outcome.ReportToJob();
    outcome.Job = job;
    outcome.Ended = false;
    outcome.ReportToJob();
    CPPUNIT_ASSERT(!job->HasEnded());

    outcome.Ended = true;
    outcome.ReportToJob();
    CPPUNIT_ASSERT_EQUAL(true, job->ToJson()["result"]["done"].get<bool>());
  }

  void TestBusyToolIsRefusedAndWaitedFor()
  {
    int calls = 0;
    mitk::WhenRestToolIdle(m_Tool, [&calls] { ++calls; });
    CPPUNIT_ASSERT_EQUAL(1, calls);
    CPPUNIT_ASSERT_NO_THROW(mitk::ThrowIfRestToolBusy(m_Tool, "/p"));

    // Updating off the GUI thread: requests are refused, and the wait polls until the work ends.
    m_Tool->Async = true;
    m_Tool->OpenGate(false);
    m_Tool->UpdatePreview();
    CPPUNIT_ASSERT_THROW(mitk::ThrowIfRestToolBusy(m_Tool, "/p"), mitk::RestException);
    mitk::WhenRestToolIdle(m_Tool, [&calls] { ++calls; });
    CPPUNIT_ASSERT_EQUAL(1, calls);

    m_Tool->OpenGate(true);
    this->PumpUntil([&calls] { return calls == 2; });
    CPPUNIT_ASSERT_NO_THROW(mitk::ThrowIfRestToolBusy(m_Tool, "/p"));
  }

  void TestConfirmationFailureTellsTheOutcome()
  {
    // An empty answer must always mean a transfer.
    CPPUNIT_ASSERT_EQUAL(std::string("The confirmation did not end. Nothing was confirmed."), mitk::GetRestConfirmationFailure(*m_Tool));

    m_Tool->UpdatePreview();
    m_Tool->ConfirmSegmentation();
    CPPUNIT_ASSERT(mitk::GetRestConfirmationFailure(*m_Tool).empty());
  }

  void TestWaitingForAStartToEnd()
  {
    int calls = 0;
    bool starting = true;
    auto job = std::make_shared<mitk::RestJob>("id", "start");

    mitk::WhenRestStartEnded(job, [&starting] { return starting; }, [&calls] { ++calls; });
    CPPUNIT_ASSERT_EQUAL(0, calls);
    starting = false;
    this->PumpUntil([&calls] { return calls == 1; });

    // Nothing is called for a job that has ended.
    job->Fail("Gone.");
    mitk::WhenRestStartEnded(job, [] { return false; }, [&calls] { ++calls; });
    CPPUNIT_ASSERT_EQUAL(1, calls);
  }

  void TestDeactivatedToolFailsItsJob()
  {
    // The manager has no active tool, so the tool is no longer the active one.
    mitk::RestJob job("id", "work");
    CPPUNIT_ASSERT(mitk::FailRestJobIfDeactivated(job, m_ToolManager, m_Tool));
    CPPUNIT_ASSERT_EQUAL(std::string("FakeTool was deactivated while it worked."), job.ToJson()["error"].get<std::string>());
    CPPUNIT_ASSERT(!mitk::FailRestJobIfDeactivated(job, m_ToolManager, nullptr));
  }

  void TestPreviewLabelsAndVoxels()
  {
    m_Tool->UpdatePreview();
    CPPUNIT_ASSERT_EQUAL(std::uint64_t(5), mitk::CountRestPreviewVoxels(m_Tool));

    // The active label of the segmentation is in a second group.
    auto* segmentation = dynamic_cast<mitk::MultiLabelSegmentation*>(m_ToolManager->GetWorkingData(0)->GetData());
    const auto group = segmentation->AddGroup();
    segmentation->SetActiveLabel(segmentation->AddLabel("Other", mitk::Color(), group)->GetValue());

    // No value, which only the confirm gives; the group of the active label.
    const auto labels = mitk::DescribeRestPreviewLabels(m_Tool, m_ToolManager, false);
    CPPUNIT_ASSERT_EQUAL(std::string("toolresult"), labels.at(1)["name"].get<std::string>());
    CPPUNIT_ASSERT(!labels[1].contains("value") && !labels[1].contains("voxels"));
    CPPUNIT_ASSERT_EQUAL(static_cast<int>(group), labels[1]["group"].get<int>());
    CPPUNIT_ASSERT_EQUAL(5, mitk::DescribeRestPreviewLabels(m_Tool, m_ToolManager, true)[1]["voxels"].get<int>());

    // The groups of the preview stand for those of the segmentation, so each label keeps its own.
    m_Tool->SetTransfersAllPreviewGroups(true);
    CPPUNIT_ASSERT_EQUAL(0, mitk::DescribeRestPreviewLabels(m_Tool, m_ToolManager, false)[1]["group"].get<int>());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestSegmentationJobs)
