/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkException.h>
#include <mitkRestPoints.h>
#include <mitkRestServer.h>
#include <mitkStandaloneDataStorage.h>
#include "mitkRestRouter.h"

#include <httplib.h>

#include <nlohmann/json.hpp>

#include <thread>

namespace
{
  constexpr int TestPort = 59101;

  mitk::RestServerConfig MakeConfig()
  {
    mitk::RestServerConfig config;
    config.enabled = true;
    config.host = "127.0.0.1";
    config.port = TestPort;
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
  MITK_TEST(CreatedTokenIsAcceptedWhenAuthIsRequired);
  MITK_TEST(TokenWithoutFileAccessCannotLoadFiles);
  MITK_TEST(EndpointsListsBuiltInAndAddedRoutes);
  MITK_TEST(JobsCanBeAwaited);
  MITK_TEST(JobsCanBeCancelled);
  MITK_TEST(WorldPointsAreParsed);
  MITK_TEST(ServerForOneClient);
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
  }

  void RouterRejectsSameShape()
  {
    mitk::RestRouter router;
    router.Add(MakeRoute("GET", "/api/v1/labels/:value", "a"));

    CPPUNIT_ASSERT_THROW(router.Add(MakeRoute("GET", "/api/v1/labels/:id", "b")), mitk::Exception);
    CPPUNIT_ASSERT_THROW(router.Add(MakeRoute("FETCH", "/api/v1/other", "c")), mitk::Exception);
    CPPUNIT_ASSERT_THROW(router.Add(MakeRoute("GET", "api/v1/other", "d")), mitk::Exception);
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

    httplib::Client client("127.0.0.1", TestPort);
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

    httplib::Client client("127.0.0.1", TestPort);
    const auto res = client.Post("/api/v1/things", httplib::Headers{ { "Origin", "https://example.org" } }, "{}", "application/json");
    server.Stop();

    CPPUNIT_ASSERT(res != nullptr);
    CPPUNIT_ASSERT_EQUAL(403, res->status);
    CPPUNIT_ASSERT_EQUAL(std::string("ACCESS_DENIED"), GetErrorCode(res->body));
  }

  void UnknownPathsAreAnsweredWithJson()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    CPPUNIT_ASSERT(server.Start());

    httplib::Client client("127.0.0.1", TestPort);
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

    httplib::Client client("127.0.0.1", TestPort);
    const auto withoutToken = client.Get("/api/v1/things");
    const auto withCreatedToken = client.Get("/api/v1/things", httplib::Headers{ { "Authorization", "Bearer " + token } });
    const auto withMainToken = client.Get("/api/v1/datastorage/nodes", httplib::Headers{ { "Authorization", "Bearer main-token" } });

    const auto inProcessWithout = server.HandleRequest(MakeRequest("GET", "/api/v1/things"));
    const auto inProcessWith = server.HandleRequest(MakeRequest("GET", "/api/v1/things", token));

    server.RemoveToken(token);
    const auto withRemovedToken = client.Get("/api/v1/things", httplib::Headers{ { "Authorization", "Bearer " + token } });
    server.Stop();

    CPPUNIT_ASSERT(withoutToken != nullptr && withCreatedToken != nullptr && withMainToken != nullptr && withRemovedToken != nullptr);
    CPPUNIT_ASSERT_EQUAL(401, withoutToken->status);
    CPPUNIT_ASSERT_EQUAL(200, withCreatedToken->status);
    CPPUNIT_ASSERT(withMainToken->status != 401);
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

    std::vector<std::thread> workers;

    mitk::RestRoute work;
    work.Method = "POST";
    work.Path = "/work";
    work.Handler = [&server, &workers](const mitk::RestRequest& request) {
      auto job = server.CreateJob("Work a little");

      workers.emplace_back([job]() {
        job->SetProgress(0.5, "halfway");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        job->Succeed({ { "answer", 42 } });
      });

      return mitk::RestJob::Respond(request, job);
    };
    server.AddRoute(work);

    CPPUNIT_ASSERT(server.Start());

    const auto started = server.HandleRequest(MakeRequest("POST", "/api/v1/work"));
    const auto startedJob = nlohmann::json::parse(started.Body)["data"];
    const auto running = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/" + startedJob["id"].get<std::string>()));
    const auto awaited = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/" + startedJob["id"].get<std::string>() + "?wait=10"));
    const auto direct = server.HandleRequest(MakeRequest("POST", "/api/v1/work?wait=10"));
    const auto list = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs"));
    const auto missing = server.HandleRequest(MakeRequest("GET", "/api/v1/jobs/job_999"));

    for (auto& worker : workers)
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
  }

  void JobsCanBeCancelled()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto cancellable = server.CreateJob("Wait for cancellation");
    bool stopped = false;
    cancellable->SetCancelHandler([&stopped]() { stopped = true; });

    const auto stubborn = server.CreateJob("Cannot be stopped");

    CPPUNIT_ASSERT(server.Start());

    const auto cancelled = server.HandleRequest(MakeRequest("DELETE", "/api/v1/jobs/" + cancellable->GetId()));
    const auto again = server.HandleRequest(MakeRequest("DELETE", "/api/v1/jobs/" + cancellable->GetId()));
    const auto refused = server.HandleRequest(MakeRequest("DELETE", "/api/v1/jobs/" + stubborn->GetId()));

    // Work that ends after the cancellation does not change the outcome.
    cancellable->Succeed();

    server.Stop();

    CPPUNIT_ASSERT_EQUAL(200, cancelled.Status);
    CPPUNIT_ASSERT(stopped);
    CPPUNIT_ASSERT(cancellable->GetState() == mitk::RestJob::State::Cancelled);
    CPPUNIT_ASSERT_EQUAL(409, again.Status);
    CPPUNIT_ASSERT_EQUAL(409, refused.Status);
    CPPUNIT_ASSERT(!stubborn->HasEnded());
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

    httplib::Client client("127.0.0.1", running->port);
    const auto withoutToken = client.Get("/api/v1/datastorage/nodes");
    const auto withEmptyToken = client.Get("/api/v1/datastorage/nodes", httplib::Headers{ { "Authorization", "Bearer " } });

    auto load = MakeRequest("POST", "/api/v1/datastorage/nodes", token);
    load.Headers["content-type"] = "application/json";
    load.Body = nlohmann::json{ { "name", "loaded" }, { "transfer", { { "file_path", "does/not/exist.nrrd" } } } }.dump();
    const auto loaded = server.HandleRequest(load);
    const auto listed = server.HandleRequest(MakeRequest("GET", "/api/v1/datastorage/nodes", token));

    server.Stop();

    CPPUNIT_ASSERT(running.has_value());
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
};

MITK_TEST_SUITE_REGISTRATION(mitkRestRoute)
