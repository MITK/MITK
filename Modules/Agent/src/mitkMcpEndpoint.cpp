/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMcpEndpoint.h>

#include <mitkErrorResponse.h>
#include <mitkExceptionMacro.h>
#include <mitkIRestServerService.h>

#include <algorithm>
#include <array>

namespace
{
  const std::array<std::string, 3> SupportedProtocolVersions = { "2025-11-25", "2025-06-18", "2025-03-26" };

  // JSON-RPC 2.0 error codes
  constexpr int ParseError = -32700;
  constexpr int InvalidRequest = -32600;
  constexpr int MethodNotFound = -32601;
  constexpr int InvalidParams = -32602;

  nlohmann::json MakeError(const nlohmann::json& id, int code, const std::string& message)
  {
    return { { "jsonrpc", "2.0" }, { "id", id }, { "error", { { "code", code }, { "message", message } } } };
  }

  nlohmann::json MakeResult(const nlohmann::json& id, nlohmann::json result)
  {
    return { { "jsonrpc", "2.0" }, { "id", id }, { "result", std::move(result) } };
  }

  std::string EncodeBase64(const std::string& data)
  {
    static constexpr char Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded;
    encoded.reserve((data.size() + 2) / 3 * 4);

    std::size_t i = 0;

    for (; i + 2 < data.size(); i += 3)
    {
      const auto n = (static_cast<unsigned char>(data[i]) << 16) |
                     (static_cast<unsigned char>(data[i + 1]) << 8) |
                     static_cast<unsigned char>(data[i + 2]);

      encoded += Alphabet[(n >> 18) & 63];
      encoded += Alphabet[(n >> 12) & 63];
      encoded += Alphabet[(n >> 6) & 63];
      encoded += Alphabet[n & 63];
    }

    if (i + 1 == data.size())
    {
      const auto n = static_cast<unsigned char>(data[i]) << 16;
      encoded += Alphabet[(n >> 18) & 63];
      encoded += Alphabet[(n >> 12) & 63];
      encoded += "==";
    }
    else if (i + 2 == data.size())
    {
      const auto n = (static_cast<unsigned char>(data[i]) << 16) | (static_cast<unsigned char>(data[i + 1]) << 8);
      encoded += Alphabet[(n >> 18) & 63];
      encoded += Alphabet[(n >> 12) & 63];
      encoded += Alphabet[(n >> 6) & 63];
      encoded += '=';
    }

    return encoded;
  }

  nlohmann::json ToJson(const mitk::McpToolResult& result)
  {
    auto content = nlohmann::json::array();

    for (const auto& item : result.Content)
    {
      if (item.ContentType == mitk::McpContent::Type::Image)
      {
        content.push_back({ { "type", "image" }, { "data", EncodeBase64(item.Data) }, { "mimeType", item.MimeType } });
      }
      else
      {
        content.push_back({ { "type", "text" }, { "text", item.Text } });
      }
    }

    return { { "content", std::move(content) }, { "isError", result.IsError } };
  }

  mitk::RestResponse MakeJsonResponse(const nlohmann::json& body, int status = 200)
  {
    mitk::RestResponse response;
    response.Status = status;
    response.ContentType = "application/json";

    // The tool results may contain arbitrary bytes from widget texts, so
    // replace invalid UTF-8 instead of throwing.
    response.Body = body.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    return response;
  }

  /** Counts a call for GetRunningCallCount() while it runs. */
  class RunningCall
  {
  public:
    explicit RunningCall(std::atomic<int>& count)
      : m_Count(count)
    {
      ++m_Count;
    }

    ~RunningCall()
    {
      --m_Count;
    }

    RunningCall(const RunningCall&) = delete;
    RunningCall& operator=(const RunningCall&) = delete;

  private:
    std::atomic<int>& m_Count;
  };
}

mitk::McpContent mitk::McpContent::FromText(const std::string& text)
{
  McpContent content;
  content.ContentType = Type::Text;
  content.Text = text;
  return content;
}

mitk::McpContent mitk::McpContent::FromImage(const std::string& data, const std::string& mimeType)
{
  McpContent content;
  content.ContentType = Type::Image;
  content.Data = data;
  content.MimeType = mimeType;
  return content;
}

mitk::McpToolResult mitk::McpToolResult::FromText(const std::string& text)
{
  McpToolResult result;
  result.Content.push_back(McpContent::FromText(text));
  return result;
}

mitk::McpToolResult mitk::McpToolResult::FromError(const std::string& message)
{
  auto result = FromText(message);
  result.IsError = true;
  return result;
}

mitk::McpEndpoint::McpEndpoint(const std::string& name, const std::string& version)
  : m_Name(name),
    m_Version(version),
    m_Server(nullptr),
    m_RunningCalls(0)
{
}

mitk::McpEndpoint::~McpEndpoint()
{
  this->Withdraw();
}

void mitk::McpEndpoint::AddTool(McpTool tool)
{
  std::lock_guard<std::mutex> lock(m_Mutex);

  if (m_Tools.count(tool.Name) != 0)
    mitkThrow() << "The MCP endpoint already has a tool named \"" << tool.Name << "\"";

  auto name = tool.Name;
  m_Tools.emplace(std::move(name), std::move(tool));
}

void mitk::McpEndpoint::Offer(IRestServerService* server, const std::string& path)
{
  if (server == nullptr)
    mitkThrow() << "No REST server to offer the MCP endpoint at";

  if (m_Server != nullptr)
    mitkThrow() << "The MCP endpoint is offered already";

  RestRoute post;
  post.Method = "POST";
  post.Path = path;
  post.Summary = "MCP endpoint for AI agents";
  post.Description =
    "The request-response part of the MCP Streamable HTTP transport: POST JSON-RPC messages, get the "
    "answers as JSON. Its tools act with the rights of the token the request carries.";
  post.Handler = [this](const RestRequest& request) { return this->HandlePost(request); };

  // No server-initiated stream and no sessions to end.
  const auto notAllowed = [](const RestRequest&) {
    RestResponse response;
    response.Status = 405;
    response.Headers["Allow"] = "POST";
    return response;
  };

  RestRoute get;
  get.Method = "GET";
  get.Path = path;
  get.Summary = "Not offered: the MCP endpoint has no server-initiated stream";
  get.Handler = notAllowed;

  RestRoute del;
  del.Method = "DELETE";
  del.Path = path;
  del.Summary = "Not offered: the MCP endpoint has no sessions to end";
  del.Handler = notAllowed;

  std::vector<RestRouteId> routes;

  try
  {
    for (auto& route : { post, get, del })
      routes.push_back(server->AddRoute(route));
  }
  catch (...)
  {
    for (const auto id : routes)
      server->RemoveRoute(id);

    throw;
  }

  m_Server = server;
  m_Routes = std::move(routes);
}

void mitk::McpEndpoint::Withdraw()
{
  if (m_Server == nullptr)
    return;

  for (const auto id : m_Routes)
    m_Server->RemoveRoute(id);

  m_Routes.clear();
  m_Server = nullptr;
}

int mitk::McpEndpoint::GetRunningCallCount() const
{
  return m_RunningCalls;
}

void mitk::McpEndpoint::RefuseCalls(const std::string& reason)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Refusal = reason;
}

void mitk::McpEndpoint::AcceptCalls()
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Refusal.reset();
}

void mitk::McpEndpoint::RequireToken(const std::string& token)
{
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_RequiredToken = token;
}

mitk::RestResponse mitk::McpEndpoint::HandlePost(const RestRequest& request)
{
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (!m_RequiredToken.empty())
    {
      const auto token = request.GetBearerToken();

      if (!token.has_value() || *token != m_RequiredToken)
      {
        auto response = RestResponse::FromError(ErrorResponse::Unauthorized("This endpoint is not open to you.", request.Path));
        response.Headers["WWW-Authenticate"] = "Bearer";
        return response;
      }
    }
  }

  nlohmann::json message;

  try
  {
    message = nlohmann::json::parse(request.Body);
  }
  catch (const nlohmann::json::exception& e)
  {
    return MakeJsonResponse(MakeError(nullptr, ParseError, e.what()), 400);
  }

  const auto answer = this->HandleMessage(message, request);

  if (answer.is_null())
  {
    RestResponse accepted;
    accepted.Status = 202;
    return accepted;
  }

  return MakeJsonResponse(answer);
}

nlohmann::json mitk::McpEndpoint::HandleMessage(const nlohmann::json& message, const RestRequest& request)
{
  if (message.is_array())
  {
    if (message.empty())
      return MakeError(nullptr, InvalidRequest, "Empty batch");

    auto responses = nlohmann::json::array();

    for (const auto& item : message)
    {
      auto response = this->HandleMessage(item, request);

      if (!response.is_null())
        responses.push_back(std::move(response));
    }

    return responses.empty() ? nlohmann::json() : responses;
  }

  if (!message.is_object())
    return MakeError(nullptr, InvalidRequest, "Expected a JSON-RPC object");

  // Notifications and responses carry no id and get no answer.
  if (!message.contains("id") || !message.contains("method"))
    return message.contains("method") || message.contains("result") || message.contains("error")
      ? nlohmann::json()
      : MakeError(message.value("id", nlohmann::json()), InvalidRequest, "Not a JSON-RPC message");

  return this->HandleRequest(message, request);
}

nlohmann::json mitk::McpEndpoint::HandleRequest(const nlohmann::json& message, const RestRequest& request)
{
  const auto id = message.value("id", nlohmann::json());

  if (!message.contains("method") || !message["method"].is_string())
    return MakeError(id, InvalidRequest, "Missing method");

  const auto method = message["method"].get<std::string>();
  const auto params = message.value("params", nlohmann::json::object());

  if (method == "initialize")
    return MakeResult(id, this->Initialize(params));

  if (method == "ping")
    return MakeResult(id, nlohmann::json::object());

  if (method == "tools/list")
    return MakeResult(id, this->ListTools());

  if (method == "tools/call")
  {
    if (!params.is_object() || !params.contains("name") || !params["name"].is_string())
      return MakeError(id, InvalidParams, "Expected the name of a tool");

    const auto name = params["name"].get<std::string>();
    McpTool tool;
    std::optional<RunningCall> runningCall;

    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      const auto entry = m_Tools.find(name);

      if (entry == m_Tools.end())
        return MakeError(id, InvalidParams, "Unknown tool: " + name);

      if (m_Refusal.has_value())
        return MakeResult(id, ToJson(McpToolResult::FromError(*m_Refusal)));

      tool = entry->second;

      // Under the lock, so that once RefuseCalls() returns, every call it
      // let through is counted.
      runningCall.emplace(m_RunningCalls);
    }

    const auto arguments = params.value("arguments", nlohmann::json::object());

    McpToolResult result;

    try
    {
      result = tool.Handler(arguments, request);
    }
    catch (const std::exception& e)
    {
      result = McpToolResult::FromError(e.what());
    }

    return MakeResult(id, ToJson(result));
  }

  return MakeError(id, MethodNotFound, "Unknown method: " + method);
}

nlohmann::json mitk::McpEndpoint::Initialize(const nlohmann::json& params) const
{
  std::string protocolVersion = SupportedProtocolVersions.front();

  if (params.contains("protocolVersion") && params["protocolVersion"].is_string())
  {
    const auto requested = params["protocolVersion"].get<std::string>();

    if (std::find(SupportedProtocolVersions.begin(), SupportedProtocolVersions.end(), requested) != SupportedProtocolVersions.end())
      protocolVersion = requested;
  }

  return {
    { "protocolVersion", protocolVersion },
    { "capabilities", { { "tools", { { "listChanged", false } } } } },
    { "serverInfo", { { "name", m_Name }, { "version", m_Version } } }
  };
}

nlohmann::json mitk::McpEndpoint::ListTools() const
{
  auto tools = nlohmann::json::array();

  std::lock_guard<std::mutex> lock(m_Mutex);

  for (const auto& [name, tool] : m_Tools)
  {
    nlohmann::json entry = {
      { "name", tool.Name },
      { "description", tool.Description },
      { "inputSchema", tool.InputSchema.is_null() ? nlohmann::json{ { "type", "object" } } : tool.InputSchema }
    };

    if (tool.ReadOnly)
      entry["annotations"] = { { "readOnlyHint", true } };

    tools.push_back(std::move(entry));
  }

  return { { "tools", std::move(tools) } };
}
