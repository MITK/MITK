/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkActionOriginScope.h>
#include <mitkIProgressService.h>
#include <mitkInteractionConst.h>
#include <mitkLimitedLinearUndo.h>
#include <mitkOperation.h>
#include <mitkOperationEvent.h>
#include <mitkRestRoute.h>
#include <mitkRestWorkbench.h>
#include <mitkVerboseLimitedLinearUndo.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace
{
  /** A progress service that reports the tasks that it is given. */
  class FakeProgressService : public mitk::IProgressService
  {
  public:
    std::vector<mitk::ProgressTaskInfo> Tasks;

    bool AddListener(mitk::IProgressListener*) override { return false; }
    bool RemoveListener(const mitk::IProgressListener*) override { return false; }
    std::vector<mitk::ProgressTaskInfo> GetActiveTasks() const override { return this->Tasks; }
    void RequestCancel(mitk::ProgressTaskId) override {}
    std::shared_ptr<mitk::ProgressTaskState> StartTask(const std::string&, unsigned int, bool) override { return nullptr; }
    void UpdateTask(mitk::ProgressTaskId, const std::string&, unsigned int, unsigned int) override {}
    void FinishTask(mitk::ProgressTaskId) override {}
  };

  mitk::ProgressTaskInfo MakeTask(mitk::ProgressTaskId id, const std::string& name, bool finished = false)
  {
    mitk::ProgressTaskInfo task;
    task.Id = id;
    task.Name = name;
    task.Finished = finished;
    return task;
  }

  struct Failure
  {
    int Status = 0;
    std::string Code;
    std::string Message;
  };

  Failure GetFailureOf(const std::function<void()>& call)
  {
    try
    {
      call();
    }
    catch (const mitk::RestException& e)
    {
      const auto& error = e.GetError()["error"];
      return { error["status"].get<int>(), error["code"].get<std::string>(), error["message"].get<std::string>() };
    }

    return {};
  }

  /** The error of a response, or an empty code for a data response. */
  std::string GetCode(const mitk::RestResponse& response)
  {
    const auto body = nlohmann::json::parse(response.Body);
    return body.contains("error") ? body["error"]["code"].get<std::string>() : std::string();
  }

  class TestOperation : public mitk::Operation
  {
  public:
    TestOperation() : mitk::Operation(mitk::OpTEST) {}
  };

  void AddStep(mitk::UndoModel* model, const std::string& description)
  {
    model->SetOperationEvent(new mitk::OperationEvent(nullptr, new TestOperation, new TestOperation, description));

    // Steps with the same id are one entry of the history.
    mitk::OperationEvent::IncCurrObjectEventId();
  }
}

class mitkRestWorkbenchTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestWorkbenchTestSuite);
  MITK_TEST(HistoryOfNoModelIsEmpty);
  MITK_TEST(HistoryListsTheMostRecentStepFirstAndWhoMadeIt);
  MITK_TEST(NextStepIsTheOneOnTopOfTheStack);
  MITK_TEST(NothingToUndoOrRedoAnswers409);
  MITK_TEST(StepsOfTheUserAreNotTheClients);
  MITK_TEST(StepsOfTheClientAreUndoneAndRedone);
  MITK_TEST(StepsOfAnotherClientAreNotOwn);
  MITK_TEST(ModelsWithoutOriginsTellNoOwnSteps);
  MITK_TEST(TasksAreDescribedWithTheirProgress);
  MITK_TEST(TasksAreFoundByTheirNumber);
  MITK_TEST(TaskIdsThatAreNoNumbersAreRefused);
  CPPUNIT_TEST_SUITE_END();

public:
  void HistoryOfNoModelIsEmpty()
  {
    const auto history = mitk::DescribeRestHistory(nullptr);

    CPPUNIT_ASSERT(history["undo"].is_array() && history["undo"].empty());
    CPPUNIT_ASSERT(history["redo"].is_array() && history["redo"].empty());

    auto model = mitk::VerboseLimitedLinearUndo::New();
    CPPUNIT_ASSERT(mitk::DescribeRestHistory(model)["undo"].empty());
  }

  void HistoryListsTheMostRecentStepFirstAndWhoMadeIt()
  {
    auto model = mitk::VerboseLimitedLinearUndo::New();

    AddStep(model, "Step by the user");

    {
      const mitk::ActionOriginScope scope("client");
      AddStep(model, "Step by the client");
    }

    CPPUNIT_ASSERT(model->Undo());

    // Seen by the client: the redo list holds its step, the undo list the user's.
    {
      const mitk::ActionOriginScope scope("client");
      const auto history = mitk::DescribeRestHistory(model);

      CPPUNIT_ASSERT_EQUAL(std::size_t(1), history["undo"].size());
      CPPUNIT_ASSERT_EQUAL(std::string("Step by the user"), history["undo"][0]["description"].get<std::string>());
      CPPUNIT_ASSERT(!history["undo"][0]["own"].get<bool>());
      CPPUNIT_ASSERT_EQUAL(std::size_t(1), history["redo"].size());
      CPPUNIT_ASSERT_EQUAL(std::string("Step by the client"), history["redo"][0]["description"].get<std::string>());
      CPPUNIT_ASSERT(history["redo"][0]["own"].get<bool>());
    }

    // The user is the empty name: the same steps from the other side.
    const auto history = mitk::DescribeRestHistory(model);
    CPPUNIT_ASSERT(history["undo"][0]["own"].get<bool>());
    CPPUNIT_ASSERT(!history["redo"][0]["own"].get<bool>());

    CPPUNIT_ASSERT(model->Redo());

    const auto both = mitk::DescribeRestHistory(model);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), both["undo"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("Step by the client"), both["undo"][0]["description"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Step by the user"), both["undo"][1]["description"].get<std::string>());
    CPPUNIT_ASSERT(both["redo"].empty());
  }

  void NextStepIsTheOneOnTopOfTheStack()
  {
    CPPUNIT_ASSERT_EQUAL(std::string(), mitk::DescribeRestNextStep(nullptr, true));
    CPPUNIT_ASSERT_EQUAL(std::string(), mitk::DescribeRestNextStep(nullptr, false));

    auto model = mitk::VerboseLimitedLinearUndo::New();
    CPPUNIT_ASSERT_EQUAL(std::string(), mitk::DescribeRestNextStep(model, true));

    AddStep(model, "First");
    AddStep(model, "Second");

    CPPUNIT_ASSERT_EQUAL(std::string("Second"), mitk::DescribeRestNextStep(model, true));
    CPPUNIT_ASSERT_EQUAL(std::string(), mitk::DescribeRestNextStep(model, false));

    CPPUNIT_ASSERT(model->Undo());

    CPPUNIT_ASSERT_EQUAL(std::string("First"), mitk::DescribeRestNextStep(model, true));
    CPPUNIT_ASSERT_EQUAL(std::string("Second"), mitk::DescribeRestNextStep(model, false));
  }

  void NothingToUndoOrRedoAnswers409()
  {
    auto model = mitk::VerboseLimitedLinearUndo::New();

    for (mitk::UndoModel* candidate : { static_cast<mitk::UndoModel*>(nullptr), static_cast<mitk::UndoModel*>(model.GetPointer()) })
    {
      const auto undo = mitk::TakeRestStep(candidate, true, "/api/v1/workbench/undo");
      CPPUNIT_ASSERT_EQUAL(409, undo.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("NOTHING_TO_UNDO"), GetCode(undo));
      CPPUNIT_ASSERT(undo.Body.find("There is nothing to undo.") != std::string::npos);

      const auto redo = mitk::TakeRestStep(candidate, false, "/api/v1/workbench/redo");
      CPPUNIT_ASSERT_EQUAL(409, redo.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("NOTHING_TO_REDO"), GetCode(redo));
      CPPUNIT_ASSERT(redo.Body.find("There is nothing to redo.") != std::string::npos);
    }

    // A step to undo is no step to redo.
    AddStep(model, "Step");
    CPPUNIT_ASSERT_EQUAL(std::string("NOTHING_TO_REDO"), GetCode(mitk::TakeRestStep(model, false, "/p")));
  }

  void StepsOfTheUserAreNotTheClients()
  {
    auto model = mitk::VerboseLimitedLinearUndo::New();
    AddStep(model, "Step by the user");

    const mitk::ActionOriginScope scope("client");
    const auto response = mitk::TakeRestStep(model, true, "/api/v1/workbench/undo");

    CPPUNIT_ASSERT_EQUAL(409, response.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("NOT_YOUR_STEP"), GetCode(response));
    CPPUNIT_ASSERT(response.Body.find("\\\"Step by the user\\\", is not yours. Ask the user to undo it.") != std::string::npos);

    // Nothing was undone.
    CPPUNIT_ASSERT(!model->UndoListEmpty());
    CPPUNIT_ASSERT(model->RedoListEmpty());

    // The same for a redo that the client sees.
    CPPUNIT_ASSERT(model->Undo());
    const auto redo = mitk::TakeRestStep(model, false, "/api/v1/workbench/redo");
    CPPUNIT_ASSERT_EQUAL(std::string("NOT_YOUR_STEP"), GetCode(redo));
    CPPUNIT_ASSERT(redo.Body.find("Ask the user to redo it.") != std::string::npos);
    CPPUNIT_ASSERT(!model->RedoListEmpty());
  }

  void StepsOfTheClientAreUndoneAndRedone()
  {
    auto model = mitk::VerboseLimitedLinearUndo::New();
    AddStep(model, "Step by the user");

    const mitk::ActionOriginScope scope("client");
    AddStep(model, "Step by the client");

    const auto undone = mitk::TakeRestStep(model, true, "/api/v1/workbench/undo");

    CPPUNIT_ASSERT_EQUAL(200, undone.Status);
    const auto data = nlohmann::json::parse(undone.Body)["data"];
    CPPUNIT_ASSERT_EQUAL(std::string("Step by the client"), data["undone"].get<std::string>());
    CPPUNIT_ASSERT(!data.contains("redone"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), data["undo"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("Step by the user"), data["undo"][0]["description"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), data["redo"].size());
    CPPUNIT_ASSERT(data["redo"][0]["own"].get<bool>());

    // The step of the user is next, and not the client's to undo.
    CPPUNIT_ASSERT_EQUAL(std::string("NOT_YOUR_STEP"), GetCode(mitk::TakeRestStep(model, true, "/p")));

    const auto redone = mitk::TakeRestStep(model, false, "/api/v1/workbench/redo");

    CPPUNIT_ASSERT_EQUAL(200, redone.Status);
    const auto redoneData = nlohmann::json::parse(redone.Body)["data"];
    CPPUNIT_ASSERT_EQUAL(std::string("Step by the client"), redoneData["redone"].get<std::string>());
    CPPUNIT_ASSERT(!redoneData.contains("undone"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), redoneData["undo"].size());
    CPPUNIT_ASSERT(redoneData["redo"].empty());
  }

  void StepsOfAnotherClientAreNotOwn()
  {
    auto model = mitk::VerboseLimitedLinearUndo::New();

    {
      const mitk::ActionOriginScope other("other client");
      AddStep(model, "Step by the other");
    }

    const mitk::ActionOriginScope scope("client");
    CPPUNIT_ASSERT_EQUAL(std::string("NOT_YOUR_STEP"), GetCode(mitk::TakeRestStep(model, true, "/p")));
    CPPUNIT_ASSERT(!model->UndoListEmpty());
  }

  void ModelsWithoutOriginsTellNoOwnSteps()
  {
    // Only the verbose model knows who made a step, so that nothing is the client's.
    auto model = mitk::LimitedLinearUndo::New();
    AddStep(model, "Step");

    const mitk::ActionOriginScope scope("client");
    const auto response = mitk::TakeRestStep(model, true, "/p");

    CPPUNIT_ASSERT_EQUAL(409, response.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("NOT_YOUR_STEP"), GetCode(response));
    CPPUNIT_ASSERT(!model->UndoListEmpty());
  }

  void TasksAreDescribedWithTheirProgress()
  {
    auto task = MakeTask(7, "Loading");
    task.StepsToDo = 8;
    task.Progress = 2;
    task.Cancelable = true;
    task.StartTime = std::chrono::steady_clock::now() - std::chrono::seconds(10);

    const auto json = mitk::ToRestJson(task);

    CPPUNIT_ASSERT_EQUAL(7, json["id"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("Loading"), json["name"].get<std::string>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.25, json["progress"].get<double>(), 1e-12);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, json["seconds"].get<double>(), 1e-12);
    CPPUNIT_ASSERT(json["cancelable"].get<bool>());
    CPPUNIT_ASSERT(!json["cancel_requested"].get<bool>());

    // The user's task is not the client's, and the client's is.
    CPPUNIT_ASSERT(json["own"].get<bool>() == mitk::ActionOriginScope::GetCurrent().empty());

    // Unknown progress has no value.
    auto indeterminate = MakeTask(8, "Waiting");
    indeterminate.Origin = "client";
    indeterminate.CancelRequested = true;

    const auto unknown = mitk::ToRestJson(indeterminate);
    CPPUNIT_ASSERT(unknown["progress"].is_null());
    CPPUNIT_ASSERT(!unknown["own"].get<bool>());
    CPPUNIT_ASSERT(unknown["cancel_requested"].get<bool>());
    CPPUNIT_ASSERT(!unknown["cancelable"].get<bool>());

    const mitk::ActionOriginScope scope("client");
    CPPUNIT_ASSERT(mitk::ToRestJson(indeterminate)["own"].get<bool>());
    CPPUNIT_ASSERT(!mitk::ToRestJson(task)["own"].get<bool>());

    // A task without steps done has made no progress; a finished one has made all of it.
    task.Progress = 0;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, mitk::ToRestJson(task)["progress"].get<double>(), 1e-12);
    task.Progress = 8;
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, mitk::ToRestJson(task)["progress"].get<double>(), 1e-12);
  }

  void TasksAreFoundByTheirNumber()
  {
    FakeProgressService service;
    service.Tasks = { MakeTask(3, "Third"), MakeTask(12, "Twelfth"), MakeTask(5, "Finished", true) };

    const auto found = mitk::FindRestTask(service, "12", "/p");
    CPPUNIT_ASSERT(found.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("Twelfth"), found->Name);

    // Leading zeros are the same number.
    CPPUNIT_ASSERT_EQUAL(std::string("Third"), mitk::FindRestTask(service, "0003", "/p")->Name);

    // No task, and no running task, with the number.
    CPPUNIT_ASSERT(!mitk::FindRestTask(service, "4", "/p").has_value());
    CPPUNIT_ASSERT(!mitk::FindRestTask(service, "5", "/p").has_value());
    CPPUNIT_ASSERT(!mitk::FindRestTask(service, "0", "/p").has_value());

    service.Tasks.clear();
    CPPUNIT_ASSERT(!mitk::FindRestTask(service, "3", "/p").has_value());
  }

  void TaskIdsThatAreNoNumbersAreRefused()
  {
    FakeProgressService service;
    service.Tasks = { MakeTask(1, "First") };

    for (const std::string text : { "", "abc", "1abc", "1 ", " 1", "+1", "-1", "1.0", "0x1", "18446744073709551616" })
    {
      const auto failure = GetFailureOf([&]() { mitk::FindRestTask(service, text, "/api/v1/workbench/tasks/x"); });
      CPPUNIT_ASSERT_EQUAL_MESSAGE("\"" + text + "\"", 400, failure.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), failure.Code);
      CPPUNIT_ASSERT_EQUAL(std::string("The task id must be a number."), failure.Message);
    }

    // The largest id is one.
    service.Tasks = { MakeTask(18446744073709551615ull, "Last") };
    CPPUNIT_ASSERT_EQUAL(std::string("Last"), mitk::FindRestTask(service, "18446744073709551615", "/p")->Name);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestWorkbench)
