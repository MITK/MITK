/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <httplib.h>

#include <mitkMcpEndpoint.h>
#include <mitkMcpRestApiTools.h>
#include <mitkRestServer.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <stdexcept>

namespace
{
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
  MITK_TEST(TestUnknownMethod);
  MITK_TEST(TestNotificationGetsNoAnswer);
  MITK_TEST(TestDuplicateTool);
  MITK_TEST(TestParseError);
  MITK_TEST(TestOfferedOnRestServer);
  MITK_TEST(TestRequiredToken);
  MITK_TEST(TestRestApiTools);
  MITK_TEST(TestRestApiToolsFlagFailedJobs);
  MITK_TEST(TestRestApiToolsUseTheCallersToken);
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
};

MITK_TEST_SUITE_REGISTRATION(mitkMcpEndpoint)
