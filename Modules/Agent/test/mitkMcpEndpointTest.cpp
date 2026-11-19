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

#include <stdexcept>
#include <string>
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
  MITK_TEST(TestRestApiToolsImageAnswer);
  MITK_TEST(TestRestApiToolsImageLimit);
  MITK_TEST(TestRestApiToolsEncodeBinaryAnswers);
  MITK_TEST(TestOfferedServerAllowsNoFiles);
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
};

MITK_TEST_SUITE_REGISTRATION(mitkMcpEndpoint)
