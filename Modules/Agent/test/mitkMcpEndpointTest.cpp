/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <httplib.h>

#include <QmitkAgentMemory.h>
#include <QmitkAgentToolServer.h>
#include <mitkMcpEndpoint.h>
#include <mitkMcpRestApiTools.h>
#include <mitkRestServer.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>

#include <QFile>
#include <QTemporaryDir>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{
  /** A REST server that QmitkAgentToolServer finds as the service, ahead of any other. */
  class RegisteredRestServer
  {
  public:
    RegisteredRestServer()
    {
      us::ServiceProperties properties;
      properties[us::ServiceConstants::SERVICE_RANKING()] = 1000;

      m_Registration = us::GetModuleContext()->RegisterService<mitk::IRestServerService>(&Server, properties);
    }

    ~RegisteredRestServer()
    {
      m_Registration.Unregister();
    }

    RegisteredRestServer(const RegisteredRestServer&) = delete;
    RegisteredRestServer& operator=(const RegisteredRestServer&) = delete;

    mitk::RestServer Server;

  private:
    us::ServiceRegistration<mitk::IRestServerService> m_Registration;
  };

  /** A REST server that keeps the routes it is given, as a request keeps the handler of its route while it runs. */
  class RouteKeepingRestServer : public mitk::RestServer
  {
  public:
    mitk::RestRouteId AddRoute(mitk::RestRoute route) override
    {
      Routes.push_back(route);
      return mitk::RestServer::AddRoute(std::move(route));
    }

    std::vector<mitk::RestRoute> Routes;
  };

  void AddResponseRoute(mitk::RestServer& server, const std::string& path, const mitk::RestResponse& response)
  {
    mitk::RestRoute route;
    route.Method = "GET";
    route.Path = path;
    route.Handler = [response](const mitk::RestRequest&) { return response; };
    server.AddRoute(route);
  }

  nlohmann::json Request(int id, const std::string& method, const nlohmann::json& params = nlohmann::json::object())
  {
    return { { "jsonrpc", "2.0" }, { "id", id }, { "method", method }, { "params", params } };
  }

  nlohmann::json Call(const std::string& tool, const nlohmann::json& arguments = nlohmann::json::object())
  {
    return Request(1, "tools/call", { { "name", tool }, { "arguments", arguments } });
  }

  std::unique_ptr<mitk::McpEndpoint> MakeEndpoint()
  {
    auto endpoint = std::make_unique<mitk::McpEndpoint>("test", "1.0");

    mitk::McpTool echo;
    echo.Name = "echo";
    echo.Description = "Returns its text.";
    echo.InputSchema = { { "type", "object" }, { "properties", { { "text", { { "type", "string" } } } } } };
    echo.ReadOnly = true;
    echo.Handler = [](const nlohmann::json& arguments, const mitk::RestRequest&) {
      return mitk::McpToolResult::FromText(arguments.value("text", std::string()));
    };
    endpoint->AddTool(echo);

    mitk::McpTool image;
    image.Name = "image";
    image.Description = "Returns an image.";
    image.Handler = [](const nlohmann::json&, const mitk::RestRequest&) {
      mitk::McpToolResult result;
      result.Content.push_back(mitk::McpContent::FromImage("abcd", "image/png"));
      return result;
    };
    endpoint->AddTool(image);

    mitk::McpTool failing;
    failing.Name = "failing";
    failing.Description = "Throws.";
    failing.Handler = [](const nlohmann::json&, const mitk::RestRequest&) -> mitk::McpToolResult {
      throw std::runtime_error("broken");
    };
    endpoint->AddTool(failing);

    return endpoint;
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

  std::string GetText(const nlohmann::json& response)
  {
    return response["result"]["content"][0]["text"].get<std::string>();
  }

  /** What became of an api_call that waited for a job when the endpoint ended its wait. */
  struct InterruptedWait
  {
    std::chrono::steady_clock::duration Duration;
    nlohmann::json Answer;
    mitk::RestJob::State JobState = mitk::RestJob::State::Running;
    bool CancelHandlerRan = false;
  };

  /** How the call waits for a job, as the agent does. */
  enum class WaitingCall
  {
    /** api_call POST of a route that starts the job, with ?wait. */
    StartsJob,

    /** api_get GET /api/v1/jobs/{id} of a job that runs, with ?wait. */
    FollowsJob,

    /** As StartsJob, for a job whose cancel discards more than its work. */
    StartsJobWhoseCancelDiscardsMore
  };

  /**
   * Lets a call wait up to 120 s for a job that never ends by itself, and
   * calls \p interrupt once the job exists.
   */
  InterruptedWait InterruptWaitingCall(WaitingCall waitingCall, const std::function<void(mitk::McpEndpoint&)>& interrupt)
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    std::promise<std::shared_ptr<mitk::RestJob>> started;
    std::atomic<bool> cancelHandlerRan = false;

    const auto createJob = [&server, &cancelHandlerRan, waitingCall]() {
      const auto job = server.CreateJob("Never ends by itself");
      job->SetCancelHandler([&cancelHandlerRan]() { cancelHandlerRan = true; });

      if (waitingCall == WaitingCall::StartsJobWhoseCancelDiscardsMore)
        job->SetCancelAlsoDiscards("the session");

      return job;
    };

    mitk::RestRoute route;
    route.Method = "POST";
    route.Path = "/test/work";
    route.Handler = [&started, &createJob](const mitk::RestRequest& request) {
      const auto job = createJob();
      started.set_value(job);
      return mitk::RestJob::Respond(request, job);
    };
    server.AddRoute(route);

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());

    std::shared_ptr<mitk::RestJob> runningJob;
    nlohmann::json call;

    if (waitingCall != WaitingCall::FollowsJob)
    {
      call = Call("api_call", { { "method", "POST" }, { "path", "/api/v1/test/work?wait=120" } });
    }
    else
    {
      runningJob = createJob();
      call = Call("api_get", { { "path", "/api/v1/jobs/" + runningJob->GetId() + "?wait=120" } });
    }

    const auto start = std::chrono::steady_clock::now();

    auto answer = std::async(std::launch::async, [&endpoint, &call]() {
      return endpoint.HandleMessage(call, mitk::RestRequest());
    });

    const auto job = waitingCall != WaitingCall::FollowsJob
      ? started.get_future().get()
      : runningJob;

    // Interrupts a call that runs, not one that RefuseCalls() keeps out.
    while (endpoint.GetRunningCallCount() == 0 && answer.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
      std::this_thread::yield();

    interrupt(endpoint);

    InterruptedWait result;
    result.Answer = answer.get();
    result.Duration = std::chrono::steady_clock::now() - start;
    result.JobState = job->GetState();
    result.CancelHandlerRan = cancelHandlerRan;

    // Only now: stopping the server ends waits as well.
    server.Stop();

    return result;
  }
}

class mitkMcpEndpointTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMcpEndpointTestSuite);
  MITK_TEST(TestInitialize);
  MITK_TEST(TestUnsupportedProtocolVersion);
  MITK_TEST(TestListTools);
  MITK_TEST(TestCallTool);
  MITK_TEST(TestImageResult);
  MITK_TEST(TestToolException);
  MITK_TEST(TestUnknownTool);
  MITK_TEST(TestMalformedMessages);
  MITK_TEST(TestRefuseCalls);
  MITK_TEST(TestRefuseCallsEndsWaitsAndCancelsJobs);
  MITK_TEST(TestRefuseCallsLeavesJobsWhoseCancelDiscardsMore);
  MITK_TEST(TestWithdrawEndsWaitsAndCancelsJobs);
  MITK_TEST(TestAcceptCallsLetsCallsWaitAgain);
  MITK_TEST(TestUnknownMethod);
  MITK_TEST(TestNotificationGetsNoAnswer);
  MITK_TEST(TestDuplicateTool);
  MITK_TEST(TestParseError);
  MITK_TEST(TestOfferedOnRestServer);
  MITK_TEST(TestWithdrawnEndpointRefusesRequestsThatHoldItsRoute);
  MITK_TEST(TestRequiredToken);
  MITK_TEST(TestRestApiTools);
  MITK_TEST(TestRestApiToolsFlagFailedJobs);
  MITK_TEST(TestRestApiToolsUseTheCallersToken);
  MITK_TEST(TestRestApiToolsImageAnswer);
  MITK_TEST(TestRestApiToolsImageLimit);
  MITK_TEST(TestRestApiToolsEncodeBinaryAnswers);
  MITK_TEST(TestOfferedServerAllowsNoFiles);
  MITK_TEST(TestOfferNeedsTheAgentsAddressOnTheList);
  MITK_TEST(TestOfferNeedsACertificateThatNamesTheHost);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestInitialize()
  {
    const auto endpoint = MakeEndpoint();
    const auto response = endpoint->HandleMessage(Request(1, "initialize", { { "protocolVersion", "2025-06-18" } }), mitk::RestRequest());

    CPPUNIT_ASSERT_EQUAL(1, response["id"].get<int>());
    CPPUNIT_ASSERT_EQUAL(std::string("2025-06-18"), response["result"]["protocolVersion"].get<std::string>());
    CPPUNIT_ASSERT(response["result"]["capabilities"].contains("tools"));
    CPPUNIT_ASSERT_EQUAL(std::string("test"), response["result"]["serverInfo"]["name"].get<std::string>());
  }

  void TestUnsupportedProtocolVersion()
  {
    const auto endpoint = MakeEndpoint();
    const auto response = endpoint->HandleMessage(Request(1, "initialize", { { "protocolVersion", "1999-01-01" } }), mitk::RestRequest());

    CPPUNIT_ASSERT(response["result"]["protocolVersion"].get<std::string>() != "1999-01-01");
  }

  void TestListTools()
  {
    const auto endpoint = MakeEndpoint();
    const auto tools = endpoint->HandleMessage(Request(2, "tools/list"), mitk::RestRequest())["result"]["tools"];

    CPPUNIT_ASSERT_EQUAL(std::size_t(3), tools.size());

    for (const auto& tool : tools)
    {
      if (tool["name"] == "echo")
      {
        CPPUNIT_ASSERT_EQUAL(std::string("object"), tool["inputSchema"]["type"].get<std::string>());
        CPPUNIT_ASSERT(tool["annotations"]["readOnlyHint"].get<bool>());
      }
      else
      {
        CPPUNIT_ASSERT(!tool.contains("annotations"));
      }
    }
  }

  void TestCallTool()
  {
    const auto endpoint = MakeEndpoint();
    const auto result = endpoint->HandleMessage(Call("echo", { { "text", "hello" } }), mitk::RestRequest())["result"];

    CPPUNIT_ASSERT(!result["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("text"), result["content"][0]["type"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), result["content"][0]["text"].get<std::string>());
  }

  void TestImageResult()
  {
    const auto endpoint = MakeEndpoint();
    const auto content = endpoint->HandleMessage(Call("image"), mitk::RestRequest())["result"]["content"][0];

    CPPUNIT_ASSERT_EQUAL(std::string("image"), content["type"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("YWJjZA=="), content["data"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("image/png"), content["mimeType"].get<std::string>());
  }

  void TestToolException()
  {
    const auto endpoint = MakeEndpoint();
    const auto result = endpoint->HandleMessage(Call("failing"), mitk::RestRequest())["result"];

    CPPUNIT_ASSERT(result["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("broken"), result["content"][0]["text"].get<std::string>());
  }

  void TestUnknownTool()
  {
    const auto endpoint = MakeEndpoint();
    const auto response = endpoint->HandleMessage(Call("missing"), mitk::RestRequest());

    CPPUNIT_ASSERT_EQUAL(-32602, response["error"]["code"].get<int>());
  }

  void TestMalformedMessages()
  {
    const auto endpoint = MakeEndpoint();

    for (const auto& params : { nlohmann::json(), nlohmann::json::array({ "echo" }), nlohmann::json::object({ { "name", 5 } }) })
    {
      const auto response = endpoint->HandleMessage(Request(1, "tools/call", params), mitk::RestRequest());
      CPPUNIT_ASSERT_EQUAL(-32602, response["error"]["code"].get<int>());
    }

    const auto response = endpoint->HandleMessage(nlohmann::json::array(), mitk::RestRequest());
    CPPUNIT_ASSERT_EQUAL(-32600, response["error"]["code"].get<int>());
  }

  void TestRefuseCalls()
  {
    const auto endpoint = MakeEndpoint();
    endpoint->RefuseCalls("You were stopped.");

    auto response = endpoint->HandleMessage(Call("echo", { { "text", "hello" } }), mitk::RestRequest());

    CPPUNIT_ASSERT(response["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("You were stopped."), GetText(response));
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), endpoint->HandleMessage(Request(2, "tools/list"), mitk::RestRequest())["result"]["tools"].size());

    endpoint->AcceptCalls();
    response = endpoint->HandleMessage(Call("echo", { { "text", "hello" } }), mitk::RestRequest());

    CPPUNIT_ASSERT(!response["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), GetText(response));
  }

  void TestRefuseCallsEndsWaitsAndCancelsJobs()
  {
    for (const auto waitingCall : { WaitingCall::StartsJob, WaitingCall::FollowsJob })
    {
      const auto wait = InterruptWaitingCall(waitingCall, [](mitk::McpEndpoint& endpoint) { endpoint.RefuseCalls("You were stopped."); });

      CPPUNIT_ASSERT(wait.Duration < std::chrono::seconds(60));
      CPPUNIT_ASSERT(wait.JobState == mitk::RestJob::State::Canceled);
      CPPUNIT_ASSERT(wait.CancelHandlerRan);
      CPPUNIT_ASSERT(!wait.Answer["result"]["isError"].get<bool>());
      CPPUNIT_ASSERT(GetText(wait.Answer).find("\"canceled\"") != std::string::npos);
    }
  }

  void TestRefuseCallsLeavesJobsWhoseCancelDiscardsMore()
  {
    const auto wait = InterruptWaitingCall(WaitingCall::StartsJobWhoseCancelDiscardsMore, [](mitk::McpEndpoint& endpoint) { endpoint.RefuseCalls("You were stopped."); });

    CPPUNIT_ASSERT(wait.Duration < std::chrono::seconds(60));
    CPPUNIT_ASSERT(wait.JobState == mitk::RestJob::State::Running);
    CPPUNIT_ASSERT(!wait.CancelHandlerRan);
    CPPUNIT_ASSERT(GetText(wait.Answer).find("\"cancel_also_discards\"") != std::string::npos);
  }

  void TestWithdrawEndsWaitsAndCancelsJobs()
  {
    const auto wait = InterruptWaitingCall(WaitingCall::StartsJob, [](mitk::McpEndpoint& endpoint) { endpoint.Withdraw(); });

    CPPUNIT_ASSERT(wait.Duration < std::chrono::seconds(60));
    CPPUNIT_ASSERT(wait.JobState == mitk::RestJob::State::Canceled);
    CPPUNIT_ASSERT(wait.CancelHandlerRan);
  }

  void TestAcceptCallsLetsCallsWaitAgain()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto job = server.CreateJob("Never ends by itself");
    job->SetCancelHandler([]() {});

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());

    endpoint.RefuseCalls("You were stopped.");
    endpoint.AcceptCalls();

    const auto start = std::chrono::steady_clock::now();
    endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/jobs/" + job->GetId() + "?wait=1" } }), mitk::RestRequest());
    const auto duration = std::chrono::steady_clock::now() - start;

    server.Stop();

    CPPUNIT_ASSERT(duration >= std::chrono::milliseconds(900));
    CPPUNIT_ASSERT(job->GetState() == mitk::RestJob::State::Running);
  }

  void TestUnknownMethod()
  {
    const auto endpoint = MakeEndpoint();
    const auto response = endpoint->HandleMessage(Request(7, "resources/list"), mitk::RestRequest());

    CPPUNIT_ASSERT_EQUAL(-32601, response["error"]["code"].get<int>());
  }

  void TestNotificationGetsNoAnswer()
  {
    const auto endpoint = MakeEndpoint();
    const nlohmann::json notification = { { "jsonrpc", "2.0" }, { "method", "notifications/initialized" } };

    CPPUNIT_ASSERT(endpoint->HandleMessage(notification, mitk::RestRequest()).is_null());

    mitk::RestRequest post;
    post.Body = notification.dump();
    CPPUNIT_ASSERT_EQUAL(202, endpoint->HandlePost(post).Status);
  }

  void TestDuplicateTool()
  {
    const auto endpoint = MakeEndpoint();

    mitk::McpTool echo;
    echo.Name = "echo";

    CPPUNIT_ASSERT_THROW(endpoint->AddTool(echo), mitk::Exception);
  }

  void TestParseError()
  {
    const auto endpoint = MakeEndpoint();

    mitk::RestRequest post;
    post.Body = "{not json";
    const auto response = endpoint->HandlePost(post);

    CPPUNIT_ASSERT_EQUAL(400, response.Status);
    CPPUNIT_ASSERT_EQUAL(-32700, nlohmann::json::parse(response.Body)["error"]["code"].get<int>());
  }

  void TestOfferedOnRestServer()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    const auto endpoint = MakeEndpoint();
    endpoint->RequireToken("agent-token");
    endpoint->Offer(&server, "/mcp");

    CPPUNIT_ASSERT_THROW(endpoint->Offer(&server, "/mcp"), mitk::Exception);
    CPPUNIT_ASSERT(server.Start());

    httplib::Client client("127.0.0.1", server.GetRunningConfig()->port);
    const httplib::Headers agent = { { "Authorization", "Bearer agent-token" } };
    const auto response = client.Post("/api/v1/mcp", agent, Call("echo", { { "text", "over http" } }).dump(), "application/json");
    const auto withoutToken = client.Post("/api/v1/mcp", Call("echo", { { "text", "over http" } }).dump(), "application/json");
    const auto stream = client.Get("/api/v1/mcp");

    endpoint->Withdraw();
    const auto withdrawn = client.Post("/api/v1/mcp", agent, Call("echo").dump(), "application/json");
    server.Stop();

    CPPUNIT_ASSERT_MESSAGE(httplib::to_string(response.error()), response);
    CPPUNIT_ASSERT_EQUAL(200, response->status);
    CPPUNIT_ASSERT_EQUAL(std::string("over http"), GetText(nlohmann::json::parse(response->body)));

    CPPUNIT_ASSERT(withoutToken);
    CPPUNIT_ASSERT_EQUAL(401, withoutToken->status);

    CPPUNIT_ASSERT(stream);
    CPPUNIT_ASSERT_EQUAL(405, stream->status);

    CPPUNIT_ASSERT(withdrawn);
    CPPUNIT_ASSERT_EQUAL(404, withdrawn->status);
  }

  void TestWithdrawnEndpointRefusesRequestsThatHoldItsRoute()
  {
    RouteKeepingRestServer server;
    auto endpoint = MakeEndpoint();

    std::promise<void> called;
    std::promise<void> released;

    mitk::McpTool waiting;
    waiting.Name = "waiting";
    waiting.Description = "Returns when it is released.";
    waiting.Handler = [&called, release = released.get_future().share()](const nlohmann::json&, const mitk::RestRequest&) {
      called.set_value();
      release.wait();
      return mitk::McpToolResult::FromText("done");
    };
    endpoint->AddTool(waiting);
    endpoint->Offer(&server, "/mcp");

    const auto route = std::find_if(server.Routes.begin(), server.Routes.end(), [](const mitk::RestRoute& kept) {
      return kept.Method == "POST";
    });

    CPPUNIT_ASSERT(route != server.Routes.end());

    // What a request holds that found the route before it was removed.
    const auto handler = route->Handler;

    mitk::RestRequest echo;
    echo.Body = Call("echo", { { "text", "hello" } }).dump();

    mitk::RestRequest wait;
    wait.Body = Call("waiting").dump();

    const auto offered = handler(echo).Status;
    const auto idle = endpoint->GetRunningRequestCount();

    auto answer = std::async(std::launch::async, [&handler, &wait]() { return handler(wait); });
    called.get_future().wait();

    endpoint->Withdraw();

    const auto withdrawn = handler(echo).Status;
    const auto running = endpoint->GetRunningRequestCount();

    released.set_value();

    const auto finished = answer.get().Status;
    const auto ended = endpoint->GetRunningRequestCount();

    endpoint.reset();
    const auto destroyed = handler(echo).Status;

    CPPUNIT_ASSERT_EQUAL(200, offered);
    CPPUNIT_ASSERT_EQUAL(0, idle);
    CPPUNIT_ASSERT_EQUAL(404, withdrawn);
    CPPUNIT_ASSERT_EQUAL(1, running);
    CPPUNIT_ASSERT_EQUAL(200, finished);
    CPPUNIT_ASSERT_EQUAL(0, ended);
    CPPUNIT_ASSERT_EQUAL(404, destroyed);
  }

  void TestRequiredToken()
  {
    const auto endpoint = MakeEndpoint();
    endpoint->RequireToken("agent-token");

    mitk::RestRequest request;
    request.Body = Call("echo", { { "text", "hello" } }).dump();

    CPPUNIT_ASSERT_EQUAL(401, endpoint->HandlePost(request).Status);

    request.Headers["authorization"] = "Bearer other-token";
    const auto refused = endpoint->HandlePost(request);
    CPPUNIT_ASSERT_EQUAL(401, refused.Status);
    CPPUNIT_ASSERT(!nlohmann::json::parse(refused.Body).contains("result"));

    request.Headers["authorization"] = "Bearer agent-token";
    const auto accepted = endpoint->HandlePost(request);
    CPPUNIT_ASSERT_EQUAL(200, accepted.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("hello"), GetText(nlohmann::json::parse(accepted.Body)));
  }

  void TestRestApiTools()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());

    const auto health = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/health" } }), mitk::RestRequest());
    const auto missing = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/missing" } }), mitk::RestRequest());
    const auto outside = endpoint.HandleMessage(Call("api_get", { { "path", "/elsewhere" } }), mitk::RestRequest());
    const auto wrongMethod = endpoint.HandleMessage(Call("api_call", { { "method", "GET" }, { "path", "/api/v1/health" } }), mitk::RestRequest());

    server.Stop();

    CPPUNIT_ASSERT(!health["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT(nlohmann::json::parse(GetText(health)).contains("data"));

    CPPUNIT_ASSERT(missing["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("HTTP 404"), GetText(missing).substr(0, 8));

    CPPUNIT_ASSERT(outside["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT(wrongMethod["result"]["isError"].get<bool>());
  }

  void TestRestApiToolsFlagFailedJobs()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    for (const std::string state : { "failed", "canceled" })
    {
      mitk::RestRoute route;
      route.Method = "GET";
      route.Path = "/test/" + state;
      route.Handler = [&server, state](const mitk::RestRequest& request) {
        const auto job = server.CreateJob("test");

        if (state == "failed")
          job->Fail("broken");
        else
        {
          job->SetCancelHandler([]() {});
          job->Cancel();
        }

        return mitk::RestJob::Respond(request, job);
      };
      server.AddRoute(route);
    }

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());

    const auto failed = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/test/failed" } }), mitk::RestRequest());
    const auto canceled = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/test/canceled" } }), mitk::RestRequest());

    server.Stop();

    CPPUNIT_ASSERT(failed["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT(!canceled["result"]["isError"].get<bool>());
  }

  void TestRestApiToolsUseTheCallersToken()
  {
    auto dataStorage = mitk::StandaloneDataStorage::New();

    mitk::RestServer server;
    server.SetConfig(MakeConfig());
    server.SetDataStorage(dataStorage);

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());

    mitk::RestRequest call;
    call.Headers["authorization"] = "Bearer " + server.CreateToken("test", mitk::RestTokenRights());

    const nlohmann::json load = {
      { "method", "POST" },
      { "path", "/api/v1/datastorage/nodes" },
      { "body", { { "name", "loaded" }, { "transfer", { { "file_path", "does/not/exist.nrrd" } } } } }
    };

    const auto denied = endpoint.HandleMessage(Call("api_call", load), call);
    const auto created = endpoint.HandleMessage(Call("api_call", { { "method", "POST" }, { "path", "/api/v1/datastorage/nodes" }, { "body", { { "name", "empty" } } } }), call);

    server.Stop();

    CPPUNIT_ASSERT(denied["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT(GetText(denied).find("FILE_ACCESS_DENIED") != std::string::npos);

    CPPUNIT_ASSERT(!created["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT(dataStorage->GetNamedNode("empty") != nullptr);
  }

  void TestRestApiToolsImageAnswer()
  {
    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    auto screenshot = mitk::RestResponse::FromBytes("abcdef", "image/png");
    screenshot.Headers["X-MITK-Image-Size"] = "640x480";
    screenshot.Headers["X-Other"] = "not for the model";
    AddResponseRoute(server, "/test/screenshot", screenshot);

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());
    const auto answer = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/test/screenshot" } }), mitk::RestRequest());
    server.Stop();

    const auto& result = answer["result"];
    CPPUNIT_ASSERT(!result["isError"].get<bool>());

    // The image, then one text per header of the endpoint, and no other header.
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), result["content"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("image"), result["content"][0]["type"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("image/png"), result["content"][0]["mimeType"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("YWJjZGVm"), result["content"][0]["data"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("text"), result["content"][1]["type"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("X-MITK-Image-Size: 640x480"), result["content"][1]["text"].get<std::string>());
  }

  void TestRestApiToolsImageLimit()
  {
    // What the model takes after base64 encoding is at most 5 MB per image.
    constexpr std::size_t limit = 3700000;

    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    AddResponseRoute(server, "/test/atlimit", mitk::RestResponse::FromBytes(std::string(limit, 'x'), "image/png"));
    AddResponseRoute(server, "/test/overlimit", mitk::RestResponse::FromBytes(std::string(limit + 1, 'x'), "image/png"));

    // An error answer is no image, whatever its content type says.
    auto failed = mitk::RestResponse::FromBytes("x", "image/png");
    failed.Status = 404;
    AddResponseRoute(server, "/test/failed", failed);

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());
    const auto atLimit = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/test/atlimit" } }), mitk::RestRequest());
    const auto overLimit = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/test/overlimit" } }), mitk::RestRequest());
    const auto notFound = endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/test/failed" } }), mitk::RestRequest());
    server.Stop();

    CPPUNIT_ASSERT(!atLimit["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("image"), atLimit["result"]["content"][0]["type"].get<std::string>());

    CPPUNIT_ASSERT(overLimit["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("text"), overLimit["result"]["content"][0]["type"].get<std::string>());
    CPPUNIT_ASSERT(GetText(overLimit).find(std::to_string(limit + 1) + " bytes") != std::string::npos);

    CPPUNIT_ASSERT(notFound["result"]["isError"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("text"), notFound["result"]["content"][0]["type"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("HTTP 404"), GetText(notFound).substr(0, 8));
  }

  void TestRestApiToolsEncodeBinaryAnswers()
  {
    // Bytes and their base64 text: one and two bytes of tail, none, and bytes above 127.
    const std::vector<std::pair<std::string, std::string>> samples = {
      { "a", "YQ==" },
      { "ab", "YWI=" },
      { "abc", "YWJj" },
      { "abcd", "YWJjZA==" },
      { "abcde", "YWJjZGU=" },
      { std::string("\xFF\xFE\xFD", 3), "//79" },
      { std::string("\xFF\xFE\xFD\xFF", 4), "//79/w==" }
    };

    mitk::RestServer server;
    server.SetConfig(MakeConfig());

    for (std::size_t i = 0; i < samples.size(); ++i)
      AddResponseRoute(server, "/test/bytes" + std::to_string(i), mitk::RestResponse::FromBytes(samples[i].first, "image/png"));

    mitk::McpEndpoint endpoint("test", "1.0");
    mitk::AddRestApiTools(endpoint, &server);

    CPPUNIT_ASSERT(server.Start());

    std::vector<nlohmann::json> answers;

    for (std::size_t i = 0; i < samples.size(); ++i)
      answers.push_back(endpoint.HandleMessage(Call("api_get", { { "path", "/api/v1/test/bytes" + std::to_string(i) } }), mitk::RestRequest()));

    server.Stop();

    for (std::size_t i = 0; i < samples.size(); ++i)
      CPPUNIT_ASSERT_EQUAL(samples[i].second, answers[i]["result"]["content"][0]["data"].get<std::string>());
  }

  void TestOfferedServerAllowsNoFiles()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());

    // A file that exists: the refusal must not come from a missing file.
    const auto file = directory.filePath("image.nrrd");

    {
      QFile out(file);
      CPPUNIT_ASSERT(out.open(QIODevice::WriteOnly));
      out.write("NRRD");
    }

    auto dataStorage = mitk::StandaloneDataStorage::New();
    RegisteredRestServer registered;
    registered.Server.SetDataStorage(dataStorage);

    QmitkAgentMemory memory(directory.filePath("memory.json"));
    QmitkAgentToolServer tools(memory);

    CPPUNIT_ASSERT_EQUAL(std::string(), tools.Offer().toStdString());

    const auto config = registered.Server.GetRunningConfig();
    CPPUNIT_ASSERT(config.has_value());

    const auto agentTokens = registered.Server.GetTokens();

    // A token that may name files, to tell the server's own restriction from the token's.
    mitk::RestTokenRights rights;
    rights.FileAccess = true;

    mitk::RestRequest load;
    load.Method = "POST";
    load.SetTarget("/api/v1/datastorage/nodes");
    load.Headers["authorization"] = "Bearer " + registered.Server.CreateToken("test", rights);
    load.Headers["content-type"] = "application/json";
    load.Body = nlohmann::json{ { "name", "loaded" }, { "transfer", { { "file_path", file.toStdString() } } } }.dump();

    const auto response = registered.Server.HandleRequest(load);

    tools.Withdraw();

    CPPUNIT_ASSERT(config->clientAccessMode == mitk::ClientAccessMode::LocalhostOnly);
    CPPUNIT_ASSERT(config->requireAuth);
    CPPUNIT_ASSERT(config->apiToken.empty());
    CPPUNIT_ASSERT(!config->httpsEnabled);
    CPPUNIT_ASSERT(config->fileAccessMode == mitk::FileAccessMode::AllowedDirectories);
    CPPUNIT_ASSERT(config->allowedFileDirectories.empty());

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), agentTokens.size());
    CPPUNIT_ASSERT(!agentTokens[0].Rights.FileAccess);

    CPPUNIT_ASSERT_EQUAL(403, response.Status);
    CPPUNIT_ASSERT_MESSAGE(response.Body, response.Body.find("FILE_ACCESS_DENIED") != std::string::npos);
    CPPUNIT_ASSERT_MESSAGE(response.Body, response.Body.find("not within any allowed directory") != std::string::npos);
    CPPUNIT_ASSERT(dataStorage->GetNamedNode("loaded") == nullptr);
  }

  void TestOfferNeedsTheAgentsAddressOnTheList()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());
    QmitkAgentMemory memory(directory.filePath("memory.json"));

    // The server compares the exact string of the address the agent arrives
    // from. For localhost it listens on one loopback address, the first that
    // localhost resolves to, which depends on the system. On loopback only, as
    // a test should not make the firewall ask.
    const auto offer = [&memory](const std::vector<std::string>& allowed) {
      RegisteredRestServer registered;

      mitk::RestServerConfig config;
      config.enabled = true;
      config.host = "localhost";
      config.port = 0;
      config.clientAccessMode = mitk::ClientAccessMode::Whitelist;
      config.allowedClientIPs = allowed;
      registered.Server.SetConfig(config);
      CPPUNIT_ASSERT(registered.Server.Start());

      QmitkAgentToolServer tools(memory);
      const auto refusal = tools.Offer().toStdString();
      tools.Withdraw();
      registered.Server.Stop();

      return refusal;
    };

    const auto listening = mitk::RestServer::GetListeningAddress("localhost");
    const auto agent = listening.rfind("127.", 0) == 0 ? std::string("127.0.0.1") : listening;
    const auto other = agent == "::1" ? std::string("127.0.0.1") : std::string("::1");

    const auto refusal = offer({ other, "::ffff:127.0.0.1" });
    CPPUNIT_ASSERT_MESSAGE(refusal, refusal.find("arrive from " + agent + " there") != std::string::npos);

    // The other loopback address, on which nothing listens, is not needed.
    CPPUNIT_ASSERT_EQUAL(std::string(), offer({ agent }));
  }

  /**
   * A key and a certificate that names \p names, like "DNS:localhost", as PEM files in \p directory,
   * valid from \p validFrom to \p validUntil seconds from now.
   */
  static std::pair<std::string, std::string> WriteCertificate(const QTemporaryDir& directory, const std::string& names,
    long validFrom = 0, long validUntil = 3600)
  {
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> key(EVP_EC_gen("P-256"), &EVP_PKEY_free);
    std::unique_ptr<X509, decltype(&X509_free)> certificate(X509_new(), &X509_free);
    CPPUNIT_ASSERT(key != nullptr && certificate != nullptr);

    X509_set_version(certificate.get(), 2);
    ASN1_INTEGER_set(X509_get_serialNumber(certificate.get()), 1);
    X509_gmtime_adj(X509_getm_notBefore(certificate.get()), validFrom);
    X509_gmtime_adj(X509_getm_notAfter(certificate.get()), validUntil);
    X509_set_pubkey(certificate.get(), key.get());

    auto* name = X509_get_subject_name(certificate.get());
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, reinterpret_cast<const unsigned char*>("MITK test"), -1, -1, 0);
    X509_set_issuer_name(certificate.get(), name);

    X509V3_CTX context;
    X509V3_set_ctx_nodb(&context);
    X509V3_set_ctx(&context, certificate.get(), certificate.get(), nullptr, nullptr, 0);
    auto* alternativeNames = X509V3_EXT_conf_nid(nullptr, &context, NID_subject_alt_name, names.c_str());
    CPPUNIT_ASSERT(alternativeNames != nullptr);
    X509_add_ext(certificate.get(), alternativeNames, -1);
    X509_EXTENSION_free(alternativeNames);

    CPPUNIT_ASSERT(X509_sign(certificate.get(), key.get(), EVP_sha256()) > 0);

    const auto keyPath = directory.filePath("key.pem").toStdString();
    const auto certificatePath = directory.filePath("certificate.pem").toStdString();

    {
      std::unique_ptr<BIO, decltype(&BIO_free)> file(BIO_new_file(keyPath.c_str(), "w"), &BIO_free);
      CPPUNIT_ASSERT(file != nullptr && PEM_write_bio_PrivateKey(file.get(), key.get(), nullptr, nullptr, 0, nullptr, nullptr) == 1);
    }

    {
      std::unique_ptr<BIO, decltype(&BIO_free)> file(BIO_new_file(certificatePath.c_str(), "w"), &BIO_free);
      CPPUNIT_ASSERT(file != nullptr && PEM_write_bio_X509(file.get(), certificate.get()) == 1);
    }

    return { keyPath, certificatePath };
  }

  void TestOfferNeedsACertificateThatNamesTheHost()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());
    QmitkAgentMemory memory(directory.filePath("memory.json"));

    // The agent connects to 127.0.0.1, which Claude Code checks against the IP entries only.
    const auto offer = [&](const std::string& names, long validFrom = 0, long validUntil = 3600) {
      const auto [keyPath, certificatePath] = WriteCertificate(directory, names, validFrom, validUntil);
      RegisteredRestServer registered;

      mitk::RestServerConfig config;
      config.enabled = true;
      config.host = "127.0.0.1";
      config.port = 0;
      config.httpsEnabled = true;
      config.sslCertPath = certificatePath;
      config.sslKeyPath = keyPath;
      registered.Server.SetConfig(config);
      CPPUNIT_ASSERT(registered.Server.Start());

      QmitkAgentToolServer tools(memory);
      const auto refusal = tools.Offer().toStdString();
      tools.Withdraw();
      registered.Server.Stop();

      return refusal;
    };

    const auto refusal = offer("DNS:localhost");
    CPPUNIT_ASSERT_MESSAGE(refusal, refusal.find("certificate does not name 127.0.0.1") != std::string::npos);

    CPPUNIT_ASSERT_EQUAL(std::string(), offer("DNS:localhost,IP:127.0.0.1"));

    // Claude Code refuses a certificate outside of its validity, which the server serves anyway.
    const auto expired = offer("DNS:localhost,IP:127.0.0.1", -7200, -3600);
    CPPUNIT_ASSERT_MESSAGE(expired, expired.find("has expired") != std::string::npos);

    const auto early = offer("DNS:localhost,IP:127.0.0.1", 3600, 7200);
    CPPUNIT_ASSERT_MESSAGE(early, early.find("is not valid yet") != std::string::npos);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMcpEndpoint)
