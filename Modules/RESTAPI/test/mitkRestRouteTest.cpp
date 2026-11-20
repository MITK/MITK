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
#include <mitkErrorResponse.h>
#include <mitkException.h>
#include <mitkRestGuiThread.h>
#include <mitkRestHelpers.h>
#include <mitkRestPoints.h>
#include <mitkRestServer.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkVtkPropRenderer.h>
#include <mitkWeakPointer.h>
#include "mitkRestRouter.h"

#include <httplib.h>

#include <nlohmann/json.hpp>

#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{
  /** A dispatcher whose thread is busy: it keeps the tasks until Run(). */
  class BusyDispatcher : public mitk::StorageThreadDispatcherBase
  {
  public:
    mitkClassMacro(BusyDispatcher, mitk::StorageThreadDispatcherBase);
    itkFactorylessNewMacro(Self);

    bool IsDispatchThread() const override
    {
      return false;
    }

    void Post(std::function<void()> task) override
    {
      {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Tasks.push_back(std::move(task));
      }

      m_Posted.notify_all();
    }

    bool WaitForTask(std::chrono::seconds timeout)
    {
      std::unique_lock<std::mutex> lock(m_Mutex);
      return m_Posted.wait_for(lock, timeout, [this]() { return !m_Tasks.empty(); });
    }

    void Run()
    {
      std::vector<std::function<void()>> tasks;

      {
        std::lock_guard<std::mutex> lock(m_Mutex);
        tasks.swap(m_Tasks);
      }

      for (const auto& task : tasks)
        task();
    }

  protected:
    bool ExecuteDispatched(std::function<void()> task) override
    {
      std::promise<void> done;

      this->Post([&task, &done]() {
        task();
        done.set_value();
      });

      done.get_future().wait();
      return true;
    }

  private:
    std::mutex m_Mutex;
    std::condition_variable m_Posted;
    std::vector<std::function<void()>> m_Tasks;
  };

  /** Runs the tasks that a call from another thread has handed to the dispatcher, once there are some. */
  void RunDispatched(BusyDispatcher* dispatcher)
  {
    CPPUNIT_ASSERT(dispatcher->WaitForTask(std::chrono::seconds(10)));
    dispatcher->Run();
  }

  /** Joins its threads on every way out of a scope: a failed assertion would end the process otherwise. */
  struct WorkerThreads
  {
    ~WorkerThreads()
    {
      for (auto& thread : Threads)
      {
        if (thread.joinable())
          thread.join();
      }
    }

    std::vector<std::thread> Threads;
  };

  /**
   * A renderer under the name of a render window of the standard display, in a
   * window that was never shown. Its camera looks down onto the plane z = 0,
   * with one millimeter to each pixel and the origin in the middle.
   */
  class TestWindow
  {
  public:
    TestWindow(const std::string& name, int width, int height)
      : m_RenderWindow(vtkSmartPointer<vtkRenderWindow>::New())
    {
      m_RenderWindow->SetSize(width, height);
      m_Renderer = mitk::VtkPropRenderer::New(name.c_str(), m_RenderWindow);
      mitk::BaseRenderer::AddInstance(m_RenderWindow, m_Renderer);

      auto* camera = m_Renderer->GetVtkRenderer()->GetActiveCamera();
      camera->ParallelProjectionOn();
      camera->SetPosition(0.0, 0.0, 100.0);
      camera->SetFocalPoint(0.0, 0.0, 0.0);
      camera->SetViewUp(0.0, 1.0, 0.0);
      camera->SetParallelScale(height / 2.0);
      camera->SetClippingRange(1.0, 1000.0);
    }

    ~TestWindow()
    {
      mitk::BaseRenderer::RemoveInstance(m_RenderWindow);
    }

    TestWindow(const TestWindow&) = delete;
    TestWindow& operator=(const TestWindow&) = delete;

    mitk::VtkPropRenderer* GetRenderer() const
    {
      return m_Renderer;
    }

    /** A body that gives pixels of a screenshot of the window as it looks now. */
    nlohmann::json MakeBody(const std::string& window, const nlohmann::json& pixels, const nlohmann::json& imageSize) const
    {
      return { { "window", window }, { "pixels", pixels }, { "image_size", imageSize },
        { "view_state", mitk::GetRestViewState(m_Renderer) } };
    }

  private:
    vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
    mitk::VtkPropRenderer::Pointer m_Renderer;
  };

  /** The status and the code of the error that a call throws; 0 and an empty code if it throws none. */
  std::pair<int, std::string> GetErrorOf(const std::function<void()>& call)
  {
    try
    {
      call();
    }
    catch (const mitk::RestException& e)
    {
      return { e.GetError()["error"]["status"].get<int>(), e.GetError()["error"]["code"].get<std::string>() };
    }

    return { 0, "" };
  }

  mitk::RestServerConfig MakeConfig()
  {
    mitk::RestServerConfig config;
    config.enabled = true;
    config.host = "127.0.0.1";
    // A free port of the system's choice: a fixed one can be taken or reserved.
    config.port = 0;
    return config;
  }

  mitk::RestRoute MakeRoute(const std::string& method, const std::string& path, const std::string& answer)
  {
    mitk::RestRoute route;
    route.Method = method;
    route.Path = path;
    route.Summary = answer;
    route.Handler = [answer](const mitk::RestRequest& request) {
      return mitk::RestResponse::FromData({ { "answer", answer }, { "params", request.PathParams } });
    };
    return route;
  }

  mitk::RestRequest MakeRequest(const std::string& method, const std::string& target, const std::string& token = "")
  {
    mitk::RestRequest request;
    request.Method = method;
    request.SetTarget(target);

    if (!token.empty())
      request.Headers["authorization"] = "Bearer " + token;

    return request;
  }

  std::string GetAnswer(const mitk::RestResponse& response)
  {
    return nlohmann::json::parse(response.Body)["data"]["answer"].get<std::string>();
  }

  std::string GetErrorCode(const std::string& body)
  {
    return nlohmann::json::parse(body)["error"]["code"].get<std::string>();
  }
}

class mitkRestRouteTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestRouteTestSuite);
  MITK_TEST(SetTargetDecodesPathAndQuery);
  MITK_TEST(RouterPrefersLiteralSegments);
  MITK_TEST(RouterRejectsSameShape);
  MITK_TEST(RouterForgetsRemovedRoutes);
  MITK_TEST(AddedRouteIsServedOverHttp);
  MITK_TEST(AddedRoutesRejectBrowserRequests);
  MITK_TEST(UnknownPathsAreAnsweredWithJson);
  MITK_TEST(HandleRequestReachesBuiltInRoutes);
  MITK_TEST(HandleRequestNeedsRunningServer);
  MITK_TEST(BodyOfTheWrongShapeIsAClientError);
  MITK_TEST(CreatedTokenIsAcceptedWhenAuthIsRequired);
  MITK_TEST(TokenWithoutFileAccessCannotLoadFiles);
  MITK_TEST(EndpointsListsBuiltInAndAddedRoutes);
  MITK_TEST(JobsCanBeAwaited);
  MITK_TEST(EndedJobsAreForgottenBeyondTheLimit);
  MITK_TEST(TokenSurvivesRestartOfTheServer);
  MITK_TEST(SecondStartKeepsTheRunningConfiguration);
  MITK_TEST(JobsCanBeCanceled);
  MITK_TEST(StopEndsWaitsForJobs);
  MITK_TEST(InterruptionEndsOnlyTheWaitsOfItsRequests);
  MITK_TEST(StopEndsCallsThatWaitForTheDispatchThread);
  MITK_TEST(RoutesActUnderTheNameOfTheirClient);
  MITK_TEST(StopEndsBuiltInRequestsThatWaitForTheDispatchThread);
  MITK_TEST(WorldPointsAreParsed);
  MITK_TEST(RestColorsAreParsedAndFormatted);
  MITK_TEST(PixelsNeedTheViewStateOfTheirScreenshot);
  MITK_TEST(PixelsMapOntoTheWindowOfTheirScreenshot);
  MITK_TEST(PixelsAreRefusedWhereTheyDoNotMapOntoThePlane);
  MITK_TEST(CallInGuiThreadPassesResultsAndRunsInTheDispatchThread);
  MITK_TEST(CallInGuiThreadPassesExceptionsOn);
  MITK_TEST(CallInGuiThreadWithoutDispatcherRunsInTheCallingThread);
  MITK_TEST(JobsEndingDuringTheWaitAreAnsweredAtOnce);
  MITK_TEST(WaitIsANumberOfSeconds);
  MITK_TEST(ServerForOneClient);
  MITK_TEST(RemovedNodeHeldByAHandlerDiesInTheDispatchThread);
  CPPUNIT_TEST_SUITE_END();

public:
  void SetTargetDecodesPathAndQuery()
  {
    mitk::RestRequest request;
    request.SetTarget("/api/v1/manual%20pages?q=level%20window&limit=3");

    CPPUNIT_ASSERT_EQUAL(std::string("/api/v1/manual pages"), request.Path);
    CPPUNIT_ASSERT_EQUAL(std::string("level window"), request.GetQueryParam("q").value_or(""));
    CPPUNIT_ASSERT_EQUAL(std::string("3"), request.GetQueryParam("limit").value_or(""));
    CPPUNIT_ASSERT(!request.GetQueryParam("missing").has_value());
  }

  void RouterPrefersLiteralSegments()
  {
    mitk::RestRouter router;
    router.Add(MakeRoute("GET", "/api/v1/editors/:id", "any"));
    router.Add(MakeRoute("GET", "/api/v1/editors/mxn", "mxn"));

    auto match = router.Find("GET", "/api/v1/editors/mxn");
    CPPUNIT_ASSERT(match.has_value());
    CPPUNIT_ASSERT(match->PathParams.empty());

    match = router.Find("GET", "/api/v1/editors/stdmulti");
    CPPUNIT_ASSERT(match.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("stdmulti"), match->PathParams.at("id"));

    CPPUNIT_ASSERT(!router.Find("GET", "/api/v1/editors").has_value());
    CPPUNIT_ASSERT(!router.Find("GET", "/api/v1/editors/").has_value());
    CPPUNIT_ASSERT(!router.Find("POST", "/api/v1/editors/mxn").has_value());
    CPPUNIT_ASSERT(router.Find("HEAD", "/api/v1/editors/mxn").has_value());

    // Equally many literal segments: the earlier literal wins, not the route added first.
    router.Add(MakeRoute("GET", "/api/v1/things/:id/items", "later literal"));
    router.Add(MakeRoute("GET", "/api/v1/things/all/:kind", "earlier literal"));

    match = router.Find("GET", "/api/v1/things/all/items");
    CPPUNIT_ASSERT(match.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("items"), match->PathParams.at("kind"));
  }

  void RouterRejectsSameShape()
  {
    mitk::RestRouter router;
    router.Add(MakeRoute("GET", "/api/v1/labels/:value", "a"));

    CPPUNIT_ASSERT_THROW(router.Add(MakeRoute("GET", "/api/v1/labels/:id", "b")), mitk::Exception);
    CPPUNIT_ASSERT_THROW(router.Add(MakeRoute("FETCH", "/api/v1/other", "c")), mitk::Exception);
    CPPUNIT_ASSERT_THROW(router.Add(MakeRoute("GET", "api/v1/other", "d")), mitk::Exception);
    CPPUNIT_ASSERT_THROW(router.Add(MakeRoute("GET", "/api/v1/a/:id/b/:id", "f")), mitk::Exception);
    CPPUNIT_ASSERT_NO_THROW(router.Add(MakeRoute("PUT", "/api/v1/labels/:id", "e")));
  }

  void RouterForgetsRemovedRoutes()
  {
    mitk::RestRouter router;
    const auto id = router.Add(MakeRoute("GET", "/api/v1/a", "a"));
    CPPUNIT_ASSERT(router.Find("GET", "/api/v1/a").has_value());

    router.Remove(id);
    CPPUNIT_ASSERT(!router.Find("GET", "/api/v1/a").has_value());
    CPPUNIT_ASSERT_NO_THROW(router.Add(MakeRoute("GET", "/api/v1/a", "a")));
  }

  void AddedRouteIsServedOverHttp()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    const auto id = server.AddRoute(MakeRoute("GET", "/things/:name", "thing"));

    CPPUNIT_ASSERT(server.Start());

    httplib::Client client("127.0.0.1", server.GetRunningConfig()->port);
    const auto res = client.Get("/api/v1/things/chair");
    const auto health = client.Get("/api/v1/health");

    server.RemoveRoute(id);
    const auto removed = client.Get("/api/v1/things/chair");
    server.Stop();

    CPPUNIT_ASSERT(res != nullptr);
    CPPUNIT_ASSERT_EQUAL(200, res->status);
    const auto body = nlohmann::json::parse(res->body);
    CPPUNIT_ASSERT_EQUAL(std::string("chair"), body["data"]["params"]["name"].get<std::string>());

    CPPUNIT_ASSERT(health != nullptr);
    CPPUNIT_ASSERT_EQUAL(200, health->status);

    CPPUNIT_ASSERT(removed != nullptr);
    CPPUNIT_ASSERT_EQUAL(404, removed->status);
  }

  void AddedRoutesRejectBrowserRequests()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.AddRoute(MakeRoute("POST", "/things", "made"));

    CPPUNIT_ASSERT(server.Start());

    httplib::Client client("127.0.0.1", server.GetRunningConfig()->port);
    const auto post = [&client](const std::string& name, const std::string& value) {
      return client.Post("/api/v1/things", httplib::Headers{ { name, value } }, "{}", "application/json");
    };

    const auto res = post("Origin", "https://example.org");
    const auto crossSite = post("Sec-Fetch-Site", "cross-site");
    const auto sameOrigin = post("Sec-Fetch-Site", "same-origin");
    const auto foreignHost = post("Host", "rebound.example.org:8080");
    const auto localHost = post("Host", "LocalHost:8080");
    server.Stop();

    CPPUNIT_ASSERT(res != nullptr);
    CPPUNIT_ASSERT_EQUAL(403, res->status);
    CPPUNIT_ASSERT_EQUAL(std::string("ACCESS_DENIED"), GetErrorCode(res->body));

    CPPUNIT_ASSERT(crossSite != nullptr);
    CPPUNIT_ASSERT_EQUAL(403, crossSite->status);
    CPPUNIT_ASSERT(sameOrigin != nullptr);
    CPPUNIT_ASSERT_EQUAL(200, sameOrigin->status);
    CPPUNIT_ASSERT(foreignHost != nullptr);
    CPPUNIT_ASSERT_EQUAL(403, foreignHost->status);
    CPPUNIT_ASSERT(localHost != nullptr);
    CPPUNIT_ASSERT_EQUAL(200, localHost->status);
  }

  void UnknownPathsAreAnsweredWithJson()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    CPPUNIT_ASSERT(server.Start());

    httplib::Client client("127.0.0.1", server.GetRunningConfig()->port);
    const auto res = client.Delete("/api/v1/nothing/here");
    server.Stop();

    CPPUNIT_ASSERT(res != nullptr);
    CPPUNIT_ASSERT_EQUAL(404, res->status);
    CPPUNIT_ASSERT_EQUAL(std::string("NOT_FOUND"), GetErrorCode(res->body));
  }

  void HandleRequestReachesBuiltInRoutes()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.AddRoute(MakeRoute("GET", "/things/:name", "thing"));

    CPPUNIT_ASSERT(server.Start());

    const auto health = server.HandleRequest(MakeRequest("GET", "/api/v1/health"));
    const auto thing = server.HandleRequest(MakeRequest("GET", "/api/v1/things/lamp"));
    const auto log = server.GetRequestLog();
    server.Stop();

    CPPUNIT_ASSERT_EQUAL(200, health.Status);
    CPPUNIT_ASSERT(health.HasJsonBody());
    CPPUNIT_ASSERT(nlohmann::json::parse(health.Body).contains("data"));

    CPPUNIT_ASSERT_EQUAL(200, thing.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("thing"), GetAnswer(thing));

    // Each request once, although the built-in handlers log themselves.
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), log.size());
  }

  void HandleRequestNeedsRunningServer()
  {
    mitk::RestServer server;
    server.AddRoute(MakeRoute("GET", "/things", "thing"));

    const auto response = server.HandleRequest(MakeRequest("GET", "/api/v1/things"));
    CPPUNIT_ASSERT_EQUAL(503, response.Status);
  }

  void BodyOfTheWrongShapeIsAClientError()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    mitk::RestRoute route;
    route.Method = "POST";
    route.Path = "/things";
    route.Handler = [](const mitk::RestRequest& request) {
      return mitk::RestResponse::FromData({ { "size", request.GetJsonBody().value("size", 1) } });
    };
    server.AddRoute(std::move(route));

    CPPUNIT_ASSERT(server.Start());

    auto request = MakeRequest("POST", "/api/v1/things");
    request.Body = "[1, 2]";
    const auto notAnObject = server.HandleRequest(request);
    request.Body = R"({"size": "large"})";
    const auto wrongType = server.HandleRequest(request);
    server.Stop();

    CPPUNIT_ASSERT_EQUAL(400, notAnObject.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("INVALID_REQUEST"), GetErrorCode(notAnObject.Body));
    CPPUNIT_ASSERT_EQUAL(400, wrongType.Status);
  }

  void CreatedTokenIsAcceptedWhenAuthIsRequired()
  {
    mitk::RestServer server;
    auto config = MakeConfig();
    config.requireAuth = true;
    config.apiToken = "main-token";
    server.SetConfig(config);
    server.AddRoute(MakeRoute("GET", "/things", "thing"));

    const auto token = server.CreateToken("test", mitk::RestTokenRights());
    CPPUNIT_ASSERT_EQUAL(std::size_t(64), token.size());

    CPPUNIT_ASSERT(server.Start());

    httplib::Client client("127.0.0.1", server.GetRunningConfig()->port);
    const auto withoutToken = client.Get("/api/v1/things");
    const auto withCreatedToken = client.Get("/api/v1/things", httplib::Headers{ { "Authorization", "Bearer " + token } });
    const auto withMainToken = client.Get("/api/v1/datastorage/nodes", httplib::Headers{ { "Authorization", "Bearer main-token" } });
    const auto withLowerCaseScheme = client.Get("/api/v1/things", httplib::Headers{ { "Authorization", "bearer " + token } });

    const auto inProcessHealth = server.HandleRequest(MakeRequest("GET", "/api/v1/health"));
    const auto inProcessWithout = server.HandleRequest(MakeRequest("GET", "/api/v1/things"));
    const auto inProcessWith = server.HandleRequest(MakeRequest("GET", "/api/v1/things", token));

    server.RemoveToken(token);
    const auto withRemovedToken = client.Get("/api/v1/things", httplib::Headers{ { "Authorization", "Bearer " + token } });
    server.Stop();

    CPPUNIT_ASSERT(withoutToken != nullptr && withCreatedToken != nullptr && withMainToken != nullptr && withRemovedToken != nullptr);
    CPPUNIT_ASSERT_EQUAL(401, withoutToken->status);
    CPPUNIT_ASSERT_EQUAL(200, withCreatedToken->status);
    CPPUNIT_ASSERT(withMainToken->status != 401);
    CPPUNIT_ASSERT(withLowerCaseScheme != nullptr);
    CPPUNIT_ASSERT_EQUAL(200, withLowerCaseScheme->status);
    CPPUNIT_ASSERT_EQUAL(200, inProcessHealth.Status);
    CPPUNIT_ASSERT_EQUAL(401, inProcessWithout.Status);
    CPPUNIT_ASSERT_EQUAL(200, inProcessWith.Status);
    CPPUNIT_ASSERT_EQUAL(401, withRemovedToken->status);
  }

  void TokenWithoutFileAccessCannotLoadFiles()
  {
    auto dataStorage = mitk::StandaloneDataStorage::New();

    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.SetDataStorage(dataStorage);

    const auto restricted = server.CreateToken("restricted", mitk::RestTokenRights());

    mitk::RestTokenRights fileRights;
    fileRights.FileAccess = true;
    const auto permitted = server.CreateToken("permitted", fileRights);

    CPPUNIT_ASSERT(server.Start());

    const auto makeLoad = [](const std::string& token) {
      auto request = MakeRequest("POST", "/api/v1/datastorage/nodes", token);
      request.Headers["content-type"] = "application/json";
      request.Body = nlohmann::json{ { "name", "loaded" }, { "transfer", { { "file_path", "this/file/does/not/exist.nrrd" } } } }.dump();
      return request;
    };

    const auto restrictedLoad = server.HandleRequest(makeLoad(restricted));
    const auto permittedLoad = server.HandleRequest(makeLoad(permitted));
    const auto anonymousLoad = server.HandleRequest(makeLoad(""));

    auto fileReference = MakeRequest("GET", "/api/v1/datastorage/nodes/node_1/data", restricted);
    fileReference.Headers["x-mitk-transfer-mode"] = "file-reference";
    const auto restrictedDownload = server.HandleRequest(fileReference);

    server.Stop();

    CPPUNIT_ASSERT_EQUAL(403, restrictedLoad.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("FILE_ACCESS_DENIED"), GetErrorCode(restrictedLoad.Body));

    // The file does not exist, which only clients with file access may learn.
    CPPUNIT_ASSERT_EQUAL(422, permittedLoad.Status);
    CPPUNIT_ASSERT_EQUAL(422, anonymousLoad.Status);

    CPPUNIT_ASSERT_EQUAL(403, restrictedDownload.Status);
  }

  void EndpointsListsBuiltInAndAddedRoutes()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.AddRoute(MakeRoute("GET", "/things/:name", "Get a thing"));

    CPPUNIT_ASSERT(server.Start());

    const auto all = server.HandleRequest(MakeRequest("GET", "/api/v1/endpoints"));
    const auto things = server.HandleRequest(MakeRequest("GET", "/api/v1/endpoints?path=/api/v1/things&details=true"));
    const auto thing = server.HandleRequest(MakeRequest("GET", "/api/v1/endpoints?path=/api/v1/things/{name}"));
    const auto nodes = server.HandleRequest(MakeRequest("GET", "/api/v1/endpoints?path=/api/v1/datastorage/nodes/{uid}/properties&details=true"));
    server.Stop();

    CPPUNIT_ASSERT_EQUAL(200, all.Status);
    const auto allData = nlohmann::json::parse(all.Body)["data"];

    bool hasHealth = false;
    bool hasEndpoints = false;

    for (const auto& endpoint : allData)
    {
      if (endpoint["method"] == "GET" && endpoint["path"] == "/api/v1/health")
      {
        hasHealth = true;
        CPPUNIT_ASSERT(!endpoint.contains("experimental"));
      }

      hasEndpoints = hasEndpoints || endpoint["path"] == "/api/v1/endpoints";
    }

    CPPUNIT_ASSERT(hasHealth);
    CPPUNIT_ASSERT(hasEndpoints);

    const auto thingsData = nlohmann::json::parse(things.Body)["data"];
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), thingsData.size());
    CPPUNIT_ASSERT_EQUAL(std::string("/api/v1/things/{name}"), thingsData[0]["path"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Get a thing"), thingsData[0]["summary"].get<std::string>());
    CPPUNIT_ASSERT(thingsData[0].contains("description"));
    CPPUNIT_ASSERT(thingsData[0].value("experimental", false));

    // Paths filter as the listing writes them.
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), nlohmann::json::parse(thing.Body)["data"].size());

    const auto nodesData = nlohmann::json::parse(nodes.Body)["data"];
    CPPUNIT_ASSERT(!nodesData.empty());

    for (const auto& endpoint : nodesData)
    {
      // References are resolved, so each operation reads on its own.
      CPPUNIT_ASSERT(endpoint["description"].dump().find("$ref") == std::string::npos);
    }
  }

  void JobsCanBeAwaited()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    // Declared before the promise: a broken promise releases the workers as
    // well, so they can be joined on every way out.
    WorkerThreads workers;

    std::promise<void> release;
    const auto released = release.get_future().share();

    mitk::RestRoute work;
    work.Method = "POST";
    work.Path = "/work";
    work.Handler = [&server, &workers, released](const mitk::RestRequest& request) {
      auto job = server.CreateJob("Work a little");

      workers.Threads.emplace_back([job, released]() {
        released.wait();
        job->Succeed({ { "answer", 42 } });
      });

      return mitk::RestJob::Respond(request, job);
    };
    server.AddRoute(work);

    CPPUNIT_ASSERT(server.Start());

    const auto started = server.HandleRequest(MakeRequest("POST", "/api/v1/work"));
    const auto startedJob = nlohmann::json::parse(started.Body)["data"];
    const auto running = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/" + startedJob["id"].get<std::string>()));
    release.set_value();
    const auto awaited = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/" + startedJob["id"].get<std::string>() + "?wait=10"));
    const auto direct = server.HandleRequest(MakeRequest("POST", "/api/v1/work?wait=10"));
    const auto list = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs"));
    const auto missing = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/job_999"));
    const auto noNumber = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/" + startedJob["id"].get<std::string>() + "?wait=nan"));

    for (auto& worker : workers.Threads)
      worker.join();

    server.Stop();

    CPPUNIT_ASSERT_EQUAL(202, started.Status);
    CPPUNIT_ASSERT_EQUAL("/api/v1/jobs/" + startedJob["id"].get<std::string>(), started.Headers.at("Location"));
    CPPUNIT_ASSERT_EQUAL(std::string("running"), startedJob["state"].get<std::string>());

    CPPUNIT_ASSERT_EQUAL(200, running.Status);

    const auto awaitedJob = nlohmann::json::parse(awaited.Body)["data"];
    CPPUNIT_ASSERT_EQUAL(std::string("succeeded"), awaitedJob["state"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(42, awaitedJob["result"]["answer"].get<int>());

    CPPUNIT_ASSERT_EQUAL(200, direct.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("succeeded"), nlohmann::json::parse(direct.Body)["data"]["state"].get<std::string>());

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), nlohmann::json::parse(list.Body)["data"].size());
    CPPUNIT_ASSERT_EQUAL(404, missing.Status);
    CPPUNIT_ASSERT_EQUAL(400, noNumber.Status);
  }

  void EndedJobsAreForgottenBeyondTheLimit()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto running = server.CreateJob("Keeps running");

    for (int i = 0; i < 110; ++i)
      server.CreateJob("Ends at once")->Succeed();

    const auto latest = server.CreateJob("Latest");

    CPPUNIT_ASSERT(server.Start());
    const auto list = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs"));
    server.Stop();

    CPPUNIT_ASSERT_EQUAL(200, list.Status);

    std::set<std::string> ids;
    int ended = 0;
    const auto body = nlohmann::json::parse(list.Body);

    for (const auto& job : body["data"])
    {
      ids.insert(job["id"].get<std::string>());

      if (job["state"].get<std::string>() == "succeeded")
        ++ended;
    }

    // The oldest ended jobs go, the running ones stay however old they are.
    CPPUNIT_ASSERT_EQUAL(100, ended);
    CPPUNIT_ASSERT_EQUAL(std::size_t(102), ids.size());
    CPPUNIT_ASSERT(ids.count(running->GetId()) == 1);
    CPPUNIT_ASSERT(ids.count(latest->GetId()) == 1);
    CPPUNIT_ASSERT(ids.count("job_2") == 0);
    CPPUNIT_ASSERT(ids.count("job_11") == 0);
    CPPUNIT_ASSERT(ids.count("job_12") == 1);
  }

  void TokenSurvivesRestartOfTheServer()
  {
    mitk::RestServer server;
    auto config = MakeConfig();
    config.requireAuth = true;
    config.apiToken = "main-token";
    server.SetConfig(config);
    server.AddRoute(MakeRoute("GET", "/things", "thing"));

    const auto token = server.CreateToken("test", mitk::RestTokenRights());

    CPPUNIT_ASSERT(server.Start());
    server.Stop();
    CPPUNIT_ASSERT(server.Start());

    const auto with = server.HandleRequest(MakeRequest("GET", "/api/v1/things", token));
    const auto without = server.HandleRequest(MakeRequest("GET", "/api/v1/things"));
    const auto tokens = server.GetTokens();
    server.Stop();

    CPPUNIT_ASSERT_EQUAL(200, with.Status);
    CPPUNIT_ASSERT_EQUAL(401, without.Status);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), tokens.size());
  }

  void SecondStartKeepsTheRunningConfiguration()
  {
    mitk::RestServer server;
    auto other = MakeConfig();
    other.requireAuth = true;
    other.apiToken = "other-token";

    CPPUNIT_ASSERT(server.Start(MakeConfig(), "first"));
    CPPUNIT_ASSERT(server.Start(other, "second"));

    const auto running = server.GetRunningConfig();
    const auto startedBy = server.GetStartedBy();
    server.Stop();

    CPPUNIT_ASSERT(running.has_value());
    CPPUNIT_ASSERT(!running->requireAuth);
    CPPUNIT_ASSERT_EQUAL(std::string("first"), startedBy);
  }

  void JobsCanBeCanceled()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto cancelable = server.CreateJob("Wait for cancellation");
    bool stopped = false;
    cancelable->SetCancelHandler([&stopped]() { stopped = true; });

    const auto stubborn = server.CreateJob("Cannot be stopped");

    CPPUNIT_ASSERT(server.Start());

    const auto canceled = server.HandleRequest(MakeRequest("DELETE", "/api/v1/jobs/" + cancelable->GetId()));
    const auto again = server.HandleRequest(MakeRequest("DELETE", "/api/v1/jobs/" + cancelable->GetId()));
    const auto refused = server.HandleRequest(MakeRequest("DELETE", "/api/v1/jobs/" + stubborn->GetId()));

    // Work that ends after the cancellation does not change the outcome.
    cancelable->Succeed();

    server.Stop();

    CPPUNIT_ASSERT_EQUAL(200, canceled.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("canceled"), nlohmann::json::parse(canceled.Body)["data"]["state"].get<std::string>());
    CPPUNIT_ASSERT(stopped);
    CPPUNIT_ASSERT(cancelable->GetState() == mitk::RestJob::State::Canceled);
    CPPUNIT_ASSERT_EQUAL(std::string("JOB_NOT_CANCELABLE"), GetErrorCode(refused.Body));
    CPPUNIT_ASSERT_EQUAL(409, again.Status);
    CPPUNIT_ASSERT_EQUAL(409, refused.Status);
    CPPUNIT_ASSERT(!stubborn->HasEnded());
  }

  void StopEndsWaitsForJobs()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto job = server.CreateJob("Never ends");

    CPPUNIT_ASSERT(server.Start());

    auto waiting = std::async(std::launch::async, [&job, port = server.GetRunningConfig()->port]() {
      httplib::Client client("127.0.0.1", port);
      return client.Get("/api/v1/jobs/" + job->GetId() + "?wait=20");
    });

    // Lets the request arrive; Stop() refuses those that come later.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    const auto start = std::chrono::steady_clock::now();
    server.Stop();
    const auto duration = std::chrono::steady_clock::now() - start;

    const auto res = waiting.get();

    CPPUNIT_ASSERT(duration < std::chrono::seconds(10));
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT_EQUAL(200, res->status);
    CPPUNIT_ASSERT_EQUAL(std::string("running"), nlohmann::json::parse(res->body)["data"]["state"].get<std::string>());
  }

  void InterruptionEndsOnlyTheWaitsOfItsRequests()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto job = server.CreateJob("Never ends by itself");

    CPPUNIT_ASSERT(server.Start());

    const auto target = "/api/v1/jobs/" + job->GetId() + "?wait=20";

    auto interrupted = MakeRequest("GET", target);
    interrupted.WaitInterruption = std::make_shared<mitk::RestWaitInterruption>();

    auto other = MakeRequest("GET", target);
    other.WaitInterruption = std::make_shared<mitk::RestWaitInterruption>();

    const auto start = std::chrono::steady_clock::now();
    auto first = std::async(std::launch::async, [&server, interrupted]() { return server.HandleRequest(interrupted); });
    auto second = std::async(std::launch::async, [&server, other]() { return server.HandleRequest(other); });

    interrupted.WaitInterruption->Interrupt();

    const auto firstAnswer = first.get();
    const auto duration = std::chrono::steady_clock::now() - start;
    const bool secondWaits = second.wait_for(std::chrono::seconds(0)) == std::future_status::timeout;

    job->Succeed();
    const auto secondAnswer = second.get();

    server.Stop();

    CPPUNIT_ASSERT(duration < std::chrono::seconds(10));
    CPPUNIT_ASSERT_EQUAL(std::string("running"), nlohmann::json::parse(firstAnswer.Body)["data"]["state"].get<std::string>());
    CPPUNIT_ASSERT(secondWaits);
    CPPUNIT_ASSERT_EQUAL(std::string("succeeded"), nlohmann::json::parse(secondAnswer.Body)["data"]["state"].get<std::string>());
  }

  void StopEndsCallsThatWaitForTheDispatchThread()
  {
    auto dispatcher = BusyDispatcher::New();

    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.SetDispatcher(dispatcher);

    std::atomic<bool> ran = false;

    mitk::RestRoute route;
    route.Method = "GET";
    route.Path = "/dispatched";
    route.Handler = [&server, &ran](const mitk::RestRequest&) {
      server.Dispatch([&ran]() { ran = true; });
      return mitk::RestResponse::FromData("done");
    };
    server.AddRoute(route);

    CPPUNIT_ASSERT_EQUAL(503, this->StopDuringRequest(server, dispatcher, "/api/v1/dispatched"));
    CPPUNIT_ASSERT(!ran);
  }

  void RoutesActUnderTheNameOfTheirClient()
  {
    auto dispatcher = BusyDispatcher::New();

    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.SetDispatcher(dispatcher);

    mitk::RestRoute route;
    route.Method = "GET";
    route.Path = "/origin";
    route.Handler = [&server](const mitk::RestRequest&) {
      const auto inHandler = mitk::ActionOriginScope::GetCurrent();
      const auto inGuiThread = mitk::CallInGuiThread(server, []() { return mitk::ActionOriginScope::GetCurrent(); });
      return mitk::RestResponse::FromData({ { "handler", inHandler }, { "gui", inGuiThread } });
    };
    server.AddRoute(route);

    const auto token = server.CreateToken("test client", mitk::RestTokenRights());
    CPPUNIT_ASSERT(server.Start());

    const auto ask = [&server, &dispatcher](const std::string& requestToken) {
      auto response = std::async(std::launch::async, [&server, requestToken]() {
        return server.HandleRequest(MakeRequest("GET", "/api/v1/origin", requestToken));
      });

      CPPUNIT_ASSERT(dispatcher->WaitForTask(std::chrono::seconds(10)));
      dispatcher->Run();

      return nlohmann::json::parse(response.get().Body)["data"];
    };

    const auto withToken = ask(token);
    const auto withoutToken = ask("");
    server.Stop();

    CPPUNIT_ASSERT_EQUAL(std::string("test client"), withToken["handler"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("test client"), withToken["gui"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("REST client"), withoutToken["handler"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("REST client"), withoutToken["gui"].get<std::string>());
    CPPUNIT_ASSERT(mitk::ActionOriginScope::GetCurrent().empty());
  }

  void CallInGuiThreadPassesResultsAndRunsInTheDispatchThread()
  {
    auto dispatcher = BusyDispatcher::New();

    mitk::RestServer server;
    server.SetDispatcher(dispatcher);

    // The dispatch thread is the one that runs the dispatcher's tasks, here the test's.
    std::thread::id worker;
    auto threadCall = std::async(std::launch::async, [&]() {
      worker = std::this_thread::get_id();
      return mitk::CallInGuiThread(server, []() { return std::this_thread::get_id(); });
    });
    RunDispatched(dispatcher);

    const auto ranIn = threadCall.get();
    CPPUNIT_ASSERT(ranIn == std::this_thread::get_id());
    CPPUNIT_ASSERT(ranIn != worker);

    // Results of any movable type.
    auto textCall = std::async(std::launch::async, [&server]() {
      return mitk::CallInGuiThread(server, []() { return std::string("answer"); });
    });
    RunDispatched(dispatcher);
    CPPUNIT_ASSERT_EQUAL(std::string("answer"), textCall.get());

    auto ownedCall = std::async(std::launch::async, [&server]() {
      return mitk::CallInGuiThread(server, []() { return std::make_unique<int>(42); });
    });
    RunDispatched(dispatcher);
    const auto owned = ownedCall.get();
    CPPUNIT_ASSERT(owned != nullptr);
    CPPUNIT_ASSERT_EQUAL(42, *owned);

    // A function without a result runs, too, and the call returns when it has run.
    std::thread::id voidRanIn;
    bool voidRan = false;
    auto voidCall = std::async(std::launch::async, [&]() {
      mitk::CallInGuiThread(server, [&]() {
        voidRanIn = std::this_thread::get_id();
        voidRan = true;
      });
    });
    RunDispatched(dispatcher);
    voidCall.get();

    CPPUNIT_ASSERT(voidRan);
    CPPUNIT_ASSERT(voidRanIn == std::this_thread::get_id());
  }

  void CallInGuiThreadPassesExceptionsOn()
  {
    auto dispatcher = BusyDispatcher::New();

    mitk::RestServer server;
    server.SetDispatcher(dispatcher);

    const auto call = [&server, &dispatcher](auto function) {
      auto future = std::async(std::launch::async, [&server, function]() { return mitk::CallInGuiThread(server, function); });
      RunDispatched(dispatcher);
      return future;
    };

    // The exception is thrown again in the calling thread, with its type and message; the dispatch thread sees none.
    auto failing = call([]() -> int { throw std::runtime_error("no luck"); });

    try
    {
      failing.get();
      CPPUNIT_FAIL("No exception");
    }
    catch (const std::runtime_error& e)
    {
      CPPUNIT_ASSERT_EQUAL(std::string("no luck"), std::string(e.what()));
    }

    auto notFound = call([]() { throw mitk::RestException(mitk::ErrorResponse::NodeNotFound("node_9", "/api/v1/x")); });
    const auto error = GetErrorOf([&notFound]() { notFound.get(); });
    CPPUNIT_ASSERT_EQUAL(404, error.first);
    CPPUNIT_ASSERT_EQUAL(std::string("NODE_NOT_FOUND"), error.second);

    // Later calls are not affected.
    auto later = call([]() { return 7; });
    CPPUNIT_ASSERT_EQUAL(7, later.get());
  }

  void CallInGuiThreadWithoutDispatcherRunsInTheCallingThread()
  {
    mitk::RestServer server;

    CPPUNIT_ASSERT(mitk::CallInGuiThread(server, []() { return std::this_thread::get_id(); }) == std::this_thread::get_id());
    CPPUNIT_ASSERT_THROW(mitk::CallInGuiThread(server, []() -> int { throw std::runtime_error("no luck"); }), std::runtime_error);
  }

  void JobsEndingDuringTheWaitAreAnsweredAtOnce()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto failing = server.CreateJob("Fails");
    const auto canceled = server.CreateJob("Is canceled");
    canceled->SetCancelHandler([]() {});

    CPPUNIT_ASSERT(server.Start());

    const auto wait = [&server](const std::shared_ptr<mitk::RestJob>& job) {
      return std::async(std::launch::async, [&server, id = job->GetId()]() {
        return server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/" + id + "?wait=20"));
      });
    };

    const auto start = std::chrono::steady_clock::now();
    auto failedAnswer = wait(failing);
    auto canceledAnswer = wait(canceled);

    failing->Fail("It broke.");
    CPPUNIT_ASSERT(canceled->Cancel());

    const auto failedResponse = failedAnswer.get();
    const auto canceledResponse = canceledAnswer.get();
    const auto duration = std::chrono::steady_clock::now() - start;
    server.Stop();

    // The wait lasts up to 20 seconds, the answers do not wait for them to pass.
    CPPUNIT_ASSERT(duration < std::chrono::seconds(10));

    CPPUNIT_ASSERT_EQUAL(200, failedResponse.Status);
    CPPUNIT_ASSERT(failedResponse.Headers.count("Location") == 0);
    const auto failedJob = nlohmann::json::parse(failedResponse.Body)["data"];
    CPPUNIT_ASSERT_EQUAL(std::string("failed"), failedJob["state"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("It broke."), failedJob["error"].get<std::string>());
    CPPUNIT_ASSERT(!failedJob.contains("result"));

    CPPUNIT_ASSERT_EQUAL(200, canceledResponse.Status);
    CPPUNIT_ASSERT(canceledResponse.Headers.count("Location") == 0);
    const auto canceledJob = nlohmann::json::parse(canceledResponse.Body)["data"];
    CPPUNIT_ASSERT_EQUAL(std::string("canceled"), canceledJob["state"].get<std::string>());
    CPPUNIT_ASSERT(!canceledJob.contains("error"));
    CPPUNIT_ASSERT(!canceledJob.contains("result"));
  }

  void WaitIsANumberOfSeconds()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto job = server.CreateJob("Keeps running");

    CPPUNIT_ASSERT(server.Start());

    const auto target = "/api/v1/jobs/" + job->GetId();

    for (const std::string value : { "abc", "", "inf", "-inf", "1s", "1,5" })
    {
      const auto response = server.HandleRequest(MakeRequest("GET", target + "?wait=" + value));

      CPPUNIT_ASSERT_EQUAL_MESSAGE("wait=" + value, 400, response.Status);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("wait=" + value, std::string("INVALID_REQUEST"), GetErrorCode(response.Body));
    }

    // No time to wait, whatever the sign: the job runs, and the answer of the endpoint that started it says where to
    // follow it. (The job's own resource answers 200 either way.)
    for (const std::string value : { "0", "-5", "0.0" })
    {
      const auto response = mitk::RestJob::Respond(MakeRequest("POST", "/api/v1/work?wait=" + value), job);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("wait=" + value, 202, response.Status);
      CPPUNIT_ASSERT_EQUAL_MESSAGE("wait=" + value, target, response.Headers.at("Location"));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("wait=" + value, std::string("running"),
        nlohmann::json::parse(response.Body)["data"]["state"].get<std::string>());
    }

    server.Stop();
  }

  void StopEndsBuiltInRequestsThatWaitForTheDispatchThread()
  {
    auto dispatcher = BusyDispatcher::New();
    auto dataStorage = mitk::StandaloneDataStorage::New();

    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.SetDataStorage(dataStorage);
    server.SetDispatcher(dispatcher);

    CPPUNIT_ASSERT(this->StopDuringRequest(server, dispatcher, "/api/v1/datastorage/nodes") >= 500);
  }

private:
  /**
   * Stops the server while a request waits for the dispatch thread, as the
   * thread that stops the server is that thread in the Workbench.
   *
   * \return The status of the request.
   */
  int StopDuringRequest(mitk::RestServer& server, BusyDispatcher* dispatcher, const std::string& path)
  {
    CPPUNIT_ASSERT(server.Start());

    auto waiting = std::async(std::launch::async, [path, port = server.GetRunningConfig()->port]() {
      httplib::Client client("127.0.0.1", port);
      return client.Get(path);
    });

    const bool dispatched = dispatcher->WaitForTask(std::chrono::seconds(10));

    auto stopping = std::async(std::launch::async, [&server]() { server.Stop(); });
    const bool stopped = stopping.wait_for(std::chrono::seconds(10)) == std::future_status::ready;

    // Runs what the dispatcher still holds: nothing that does anything, if the request has ended.
    dispatcher->Run();
    stopping.wait();

    const auto res = waiting.get();

    CPPUNIT_ASSERT(dispatched);
    CPPUNIT_ASSERT(stopped);
    CPPUNIT_ASSERT(res);

    return res->status;
  }

public:

  void RestColorsAreParsedAndFormatted()
  {
    const auto color = mitk::ParseRestColor("#FF8800");

    CPPUNIT_ASSERT(color.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("#ff8800"), mitk::ToRestColor(*color));

    for (const auto* text : { "", "ff8800", "#ff880", "#ff88000", "#gg8800", "red" })
      CPPUNIT_ASSERT(!mitk::ParseRestColor(text).has_value());

    CPPUNIT_ASSERT_THROW(mitk::ThrowRestError("CODE", "Title", "Detail", 400, "/path"), mitk::RestException);
  }

  void WorldPointsAreParsed()
  {
    const auto points = mitk::GetRestWorldPoints({ { "points", { { 1.5, 2, -3 }, { 0, 0, 0 } } } }, "/test", 2);

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), points.size());
    CPPUNIT_ASSERT_EQUAL(-3.0, points[0][2]);
    CPPUNIT_ASSERT_EQUAL(std::string("[1.5,2.0,-3.0]"), mitk::ToRestJson(points[0]).dump());

    CPPUNIT_ASSERT_THROW(mitk::GetRestWorldPoints({ { "points", { { 1, 2 } } } }, "/test"), mitk::RestException);
    CPPUNIT_ASSERT_THROW(mitk::GetRestWorldPoints({ { "points", { { 1, 2, 3 } } } }, "/test", 2), mitk::RestException);
    CPPUNIT_ASSERT_THROW(mitk::GetRestWorldPoints({ { "pixels", { { 1, 2 } } } }, "/test"), mitk::RestException);
    CPPUNIT_ASSERT_THROW(mitk::GetRestWorldPoints(nlohmann::json::object(), "/test"), mitk::RestException);
  }

  void PixelsNeedTheViewStateOfTheirScreenshot()
  {
    // Sized, as a window reports no size before it is shown.
    auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    renderWindow->SetSize(300, 300);
    auto renderer = mitk::VtkPropRenderer::New("stdmulti.widget0", renderWindow);
    mitk::BaseRenderer::AddInstance(renderWindow, renderer);

    struct Registration
    {
      ~Registration() { mitk::BaseRenderer::RemoveInstance(Window); }
      vtkRenderWindow* Window;
    } registration{ renderWindow };

    const auto state = mitk::GetRestViewState(renderer);

    CPPUNIT_ASSERT(!state.empty());
    CPPUNIT_ASSERT_EQUAL(state, mitk::GetRestViewState(renderer));

    auto camera = renderer->GetVtkRenderer()->GetActiveCamera();
    camera->SetParallelScale(camera->GetParallelScale() + 1.0);
    CPPUNIT_ASSERT(state != mitk::GetRestViewState(renderer));

    nlohmann::json body = { { "window", "axial" }, { "pixels", { { 10, 10 } } },
      { "image_size", { renderer->GetSizeX(), renderer->GetSizeY() } } };

    const auto statusOf = [&body]() {
      try
      {
        mitk::GetRestWorldPoints(body, "/test");
      }
      catch (const mitk::RestException& e)
      {
        return e.GetError()["error"]["status"].get<int>();
      }

      return 0;
    };

    CPPUNIT_ASSERT_EQUAL(400, statusOf());

    body["view_state"] = state;
    CPPUNIT_ASSERT_EQUAL(409, statusOf());
  }

  void PixelsMapOntoTheWindowOfTheirScreenshot()
  {
    const TestWindow window("stdmulti.widget0", 400, 200);

    const auto expectPoint = [](const mitk::Point3D& point, double x, double y) {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(x, point[0], 1e-3);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(y, point[1], 1e-3);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, point[2], 1e-3);
    };

    // A screenshot in the size of the window: the origin of pixels is the top left corner, the one of the world the middle,
    // and the y axis of the world points up.
    auto points = mitk::GetRestWorldPoints(
      window.MakeBody("axial", { { 0, 0 }, { 400, 200 }, { 200, 100 }, { 100, 50 }, { 200, 0 } }, { 400, 200 }), "/test");

    CPPUNIT_ASSERT_EQUAL(std::size_t(5), points.size());
    expectPoint(points[0], -200.0, 100.0);
    expectPoint(points[1], 200.0, -100.0);
    expectPoint(points[2], 0.0, 0.0);
    expectPoint(points[3], -100.0, 50.0);
    expectPoint(points[4], 0.0, 100.0);

    // A screenshot that is half as large: its pixels are twice as far apart.
    points = mitk::GetRestWorldPoints(window.MakeBody("axial", { { 50, 25 }, { 200, 100 } }, { 200, 100 }), "/test");

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), points.size());
    expectPoint(points[0], -100.0, 50.0);
    expectPoint(points[1], 200.0, -100.0);

    // The size of a screenshot is rounded to whole pixels, which the proportions tolerate.
    points = mitk::GetRestWorldPoints(window.MakeBody("axial", { { 0, 0 }, { 201, 100 } }, { 201, 100 }), "/test");

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), points.size());
    expectPoint(points[0], -200.0, 100.0);
    expectPoint(points[1], 200.0, -100.0);

    // The least number of points counts for pixels as well.
    const auto onePixel = window.MakeBody("axial", { { 10, 10 } }, { 400, 200 });
    CPPUNIT_ASSERT_NO_THROW(mitk::GetRestWorldPoints(onePixel, "/test", 1));
    CPPUNIT_ASSERT_EQUAL(400, GetErrorOf([&onePixel]() { mitk::GetRestWorldPoints(onePixel, "/test", 2); }).first);
  }

  void PixelsAreRefusedWhereTheyDoNotMapOntoThePlane()
  {
    const TestWindow axial("stdmulti.widget0", 400, 200);
    const TestWindow threeD("stdmulti.widget3", 400, 200);
    threeD.GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard3D);

    const auto statusOf = [](const nlohmann::json& body) {
      return GetErrorOf([&body]() { mitk::GetRestWorldPoints(body, "/test"); });
    };

    const auto valid = axial.MakeBody("axial", { { 10, 10 } }, { 400, 200 });
    CPPUNIT_ASSERT_EQUAL(0, statusOf(valid).first);

    // Parts of the body that are missing or of the wrong type.
    const std::vector<std::pair<std::string, nlohmann::json>> changes = {
      { "pixels", "10,10" },
      { "pixels", nlohmann::json::array() },
      { "pixels", { { 10, 10, 10 } } },
      // An array, not an object, although JSON initializers read a list of pairs of a string and a value as one.
      { "pixels", nlohmann::json::array({ nlohmann::json::array({ "10", 10 }) }) },
      { "pixels", { 10 } },
      { "window", 0 },
      { "image_size", { 0, 200 } },
      { "image_size", { 400, -200 } },
      { "image_size", { 400 } },
      { "image_size", { "400", "200" } },
      { "image_size", "400x200" },
      { "view_state", 7 },
      // Pixels that lie outside of the screenshot; its edges lie within.
      { "pixels", { { 401, 10 } } },
      { "pixels", { { -1, 10 } } },
      { "pixels", { { 10, 201 } } },
      { "pixels", { { 10, -0.5 } } },
      // A screenshot that is cropped, or otherwise not of the proportions of the window.
      { "image_size", { 390, 200 } },
      { "image_size", { 400, 100 } },
      { "image_size", { 200, 200 } }
    };

    for (const auto& [key, value] : changes)
    {
      auto body = valid;
      body[key] = value;

      CPPUNIT_ASSERT_EQUAL_MESSAGE(key + ": " + value.dump(), 400, statusOf(body).first);
    }

    for (const std::string key : { "window", "image_size", "view_state" })
    {
      auto body = valid;
      body.erase(key);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("without " + key, 400, statusOf(body).first);
    }

    // The edges of the screenshot are valid.
    CPPUNIT_ASSERT_EQUAL(0, statusOf(axial.MakeBody("axial", { { 0, 0 }, { 400, 200 } }, { 400, 200 })).first);

    // Windows that do not exist, or that no renderer stands for.
    auto body = valid;
    body["window"] = "lateral";
    CPPUNIT_ASSERT(statusOf(body) == std::make_pair(404, std::string("RENDER_WINDOW_NOT_FOUND")));

    body["window"] = "sagittal";
    CPPUNIT_ASSERT(statusOf(body) == std::make_pair(503, std::string("RENDER_WINDOW_NOT_AVAILABLE")));

    // The 3D window has no plane to map onto.
    CPPUNIT_ASSERT_EQUAL(400, statusOf(threeD.MakeBody("3d", { { 10, 10 } }, { 400, 200 })).first);

    // The view of the screenshot is gone once the camera has moved.
    auto* camera = axial.GetRenderer()->GetVtkRenderer()->GetActiveCamera();
    camera->SetFocalPoint(5.0, 0.0, 0.0);
    CPPUNIT_ASSERT(statusOf(valid) == std::make_pair(409, std::string("VIEW_CHANGED")));
    CPPUNIT_ASSERT_EQUAL(0, statusOf(axial.MakeBody("axial", { { 10, 10 } }, { 400, 200 })).first);
  }

  /** As the Agent view starts it: a free port, only created tokens, no files. */
  void ServerForOneClient()
  {
    auto dataStorage = mitk::StandaloneDataStorage::New();

    mitk::RestServer server;
    server.SetDataStorage(dataStorage);

    const auto pending = server.GetPendingConfig();

    auto config = MakeConfig();
    config.port = 0;
    config.requireAuth = true;
    config.apiToken.clear();
    config.fileAccessMode = mitk::FileAccessMode::AllowedDirectories;
    config.allowedFileDirectories.clear();

    // Even with file access, the server allows no files.
    mitk::RestTokenRights rights;
    rights.FileAccess = true;
    const auto token = server.CreateToken("Agent view", rights);

    CPPUNIT_ASSERT(server.Start(config, "Agent view"));

    const auto running = server.GetRunningConfig();
    const auto startedBy = server.GetStartedBy();
    const auto tokens = server.GetTokens();

    CPPUNIT_ASSERT(running.has_value());

    httplib::Client client("127.0.0.1", running->port);
    const auto withoutToken = client.Get("/api/v1/datastorage/nodes");
    const auto withEmptyToken = client.Get("/api/v1/datastorage/nodes", httplib::Headers{ { "Authorization", "Bearer " } });

    auto load = MakeRequest("POST", "/api/v1/datastorage/nodes", token);
    load.Headers["content-type"] = "application/json";
    load.Body = nlohmann::json{ { "name", "loaded" }, { "transfer", { { "file_path", "does/not/exist.nrrd" } } } }.dump();
    const auto loaded = server.HandleRequest(load);
    const auto listed = server.HandleRequest(MakeRequest("GET", "/api/v1/datastorage/nodes", token));

    server.Stop();

    CPPUNIT_ASSERT(running->port > 0);
    CPPUNIT_ASSERT_EQUAL(std::string("Agent view"), startedBy);
    CPPUNIT_ASSERT(server.GetStartedBy().empty());

    // The pending configuration stays the user's.
    CPPUNIT_ASSERT_EQUAL(pending.port, server.GetPendingConfig().port);
    CPPUNIT_ASSERT_EQUAL(pending.requireAuth, server.GetPendingConfig().requireAuth);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), tokens.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Agent view"), tokens.front().Client);

    CPPUNIT_ASSERT(withoutToken != nullptr && withEmptyToken != nullptr);
    CPPUNIT_ASSERT_EQUAL(401, withoutToken->status);
    CPPUNIT_ASSERT_EQUAL(401, withEmptyToken->status);

    CPPUNIT_ASSERT_EQUAL(403, loaded.Status);
    CPPUNIT_ASSERT_EQUAL(200, listed.Status);
  }

  /**
   * A handler resolves its nodes in its own thread, as CallInGuiThread() asks.
   * The user removes one meanwhile in the dispatch thread, here the test's.
   */
  void RemovedNodeHeldByAHandlerDiesInTheDispatchThread()
  {
    auto dataStorage = mitk::StandaloneDataStorage::New();
    auto dispatcher = BusyDispatcher::New();

    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.SetDataStorage(dataStorage);

    std::mutex mutex;
    std::thread::id deletedIn;
    std::thread::id observedIn;

    auto node = mitk::DataNode::New();
    node->SetName("held");
    node->AddObserver(itk::DeleteEvent(), [&](const itk::EventObject&) {
      std::lock_guard<std::mutex> lock(mutex);
      deletedIn = std::this_thread::get_id();
    });

    mitk::WeakPointer<mitk::DataNode> observer(node);
    observer.SetDeleteEventCallback([&]() {
      std::lock_guard<std::mutex> lock(mutex);
      observedIn = std::this_thread::get_id();
    });

    dataStorage->Add(node);

    std::promise<void> held;
    std::promise<void> removed;
    auto removedFuture = removed.get_future().share();

    mitk::RestRoute route;
    route.Method = "GET";
    route.Path = "/held/:uid";
    route.Handler = [&](const mitk::RestRequest& request) {
      auto heldNode = server.FindNode(request.PathParams.at("uid"));
      held.set_value();
      removedFuture.wait();

      return mitk::RestResponse::FromData(heldNode != nullptr ? heldNode->GetName() : std::string("none"));
    };
    server.AddRoute(route);

    // Before the dispatcher, which runs tasks only when the test says so.
    const auto uid = server.GetNodeUid(node);
    CPPUNIT_ASSERT(!uid.empty());

    server.SetDispatcher(dispatcher);
    CPPUNIT_ASSERT(server.Start());

    auto response = std::async(std::launch::async, [&server, uid]() {
      return server.HandleRequest(MakeRequest("GET", "/api/v1/held/" + uid));
    });

    const bool reached = held.get_future().wait_for(std::chrono::seconds(10)) == std::future_status::ready;

    // What the Data Manager does: the storage lets go, and nobody else holds the node.
    dataStorage->Remove(node);
    node = nullptr;
    removed.set_value();

    const auto answer = response.get();

    // What the handler left to the dispatch thread.
    dispatcher->Run();
    server.Stop();

    CPPUNIT_ASSERT(reached);
    CPPUNIT_ASSERT_EQUAL(200, answer.Status);

    std::lock_guard<std::mutex> lock(mutex);
    CPPUNIT_ASSERT(deletedIn == std::this_thread::get_id());
    CPPUNIT_ASSERT(observedIn == std::this_thread::get_id());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestRoute)
