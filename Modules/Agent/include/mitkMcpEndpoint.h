/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMcpEndpoint_h
#define mitkMcpEndpoint_h

#include <MitkAgentExports.h>

#include <mitkRestRoute.h>

#include <nlohmann/json.hpp>

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace mitk
{
  struct IRestServerService;

  /** \brief One piece of a tool result: text, or an encoded image. */
  struct MITKAGENT_EXPORT McpContent
  {
    enum class Type
    {
      Text,
      Image
    };

    static McpContent FromText(const std::string& text);

    /**
     * \param data The encoded image file, for example a PNG.
     * \param mimeType The type of \p data, for example "image/png".
     */
    static McpContent FromImage(const std::string& data, const std::string& mimeType);

    Type ContentType = Type::Text;
    std::string Text;
    std::string Data;
    std::string MimeType;
  };

  /**
   * \brief What a tool returns to the model.
   *
   * IsError marks a failure the model should see and react to, like a
   * widget that does not exist. It is not a protocol error.
   */
  struct MITKAGENT_EXPORT McpToolResult
  {
    static McpToolResult FromText(const std::string& text);
    static McpToolResult FromError(const std::string& message);

    std::vector<McpContent> Content;
    bool IsError = false;
  };

  /**
   * \brief A tool offered to MCP clients.
   *
   * The handler runs on a thread of the REST server and may block. A handler
   * that throws produces an error result with the exception's message.
   */
  struct MITKAGENT_EXPORT McpTool
  {
    std::string Name;
    std::string Description;

    /** \brief JSON schema of the arguments, an object schema. */
    nlohmann::json InputSchema;

    /** \brief Whether the tool only inspects, so clients may run it concurrently. */
    bool ReadOnly = false;

    /**
     * \param arguments The arguments of the call.
     * \param request The HTTP request that carried the call, for example to
     *        act with the rights of the client's token.
     */
    std::function<McpToolResult(const nlohmann::json& arguments, const RestRequest& request)> Handler;
  };

  /**
   * \brief Offers tools to MCP clients as an endpoint of the REST API.
   *
   * Implements the request-response part of the MCP Streamable HTTP
   * transport: clients POST JSON-RPC messages and get the answer as a JSON
   * body. There is no server-initiated stream. Authentication and keeping
   * browsers out are left to the REST server.
   */
  class MITKAGENT_EXPORT McpEndpoint
  {
  public:
    McpEndpoint(const std::string& name, const std::string& version);

    /** \brief Withdraws the endpoint if it is offered. */
    ~McpEndpoint();

    McpEndpoint(const McpEndpoint&) = delete;
    McpEndpoint& operator=(const McpEndpoint&) = delete;

    /**
     * \brief Add a tool.
     *
     * \throw mitk::Exception if a tool of that name exists.
     */
    void AddTool(McpTool tool);

    /**
     * \brief Offer the endpoint at a path below /api/v1 of a REST server.
     *
     * \param server Must outlive the offer.
     * \param path Like "/mcp".
     * \throw mitk::Exception if the endpoint is offered already or the path is taken.
     */
    void Offer(IRestServerService* server, const std::string& path);

    /**
     * \brief Remove the endpoint from the server.
     *
     * Calls that run already go on; GetRunningCallCount() tells when they ended.
     */
    void Withdraw();

    /** \brief The number of tool calls that run at the moment. */
    int GetRunningCallCount() const;

    /**
     * \brief Answer tool calls with an error result that says \p reason,
     *        instead of running them, until AcceptCalls().
     *
     * Calls that run already go on. Once this returns, GetRunningCallCount()
     * counts only those.
     */
    void RefuseCalls(const std::string& reason);

    /** \brief Run tool calls again after RefuseCalls(). */
    void AcceptCalls();

    /** \brief Answer a POST of JSON-RPC messages as the endpoint does. */
    RestResponse HandlePost(const RestRequest& request);

    /**
     * \brief Answer one JSON-RPC message, or a batch of them.
     *
     * \param request The HTTP request that carried the message, passed on to
     *        the tool handlers.
     * \return The response, or null for a notification or a response, which
     *         get no answer.
     */
    nlohmann::json HandleMessage(const nlohmann::json& message, const RestRequest& request);

  private:
    nlohmann::json HandleRequest(const nlohmann::json& message, const RestRequest& request);
    nlohmann::json Initialize(const nlohmann::json& params) const;
    nlohmann::json ListTools() const;

    std::string m_Name;
    std::string m_Version;

    mutable std::mutex m_Mutex;
    std::map<std::string, McpTool> m_Tools;
    std::optional<std::string> m_Refusal;

    IRestServerService* m_Server;
    std::vector<RestRouteId> m_Routes;

    std::atomic<int> m_RunningCalls;
  };
}

#endif
