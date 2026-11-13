/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMcpRestApiTools.h>

#include <mitkIRestServerService.h>
#include <mitkMcpEndpoint.h>

#include <algorithm>
#include <array>
#include <stdexcept>

namespace
{
  // Claude Code cuts tool results off at about 25000 tokens.
  constexpr std::size_t MaxTextLength = 60000;

  // Images reach the model base64-encoded, which grows them by a third, and
  // the model takes at most 5 MB per image.
  constexpr std::size_t MaxImageBytes = 3700000;

  const std::array<std::string, 4> ChangingMethods = { "POST", "PUT", "PATCH", "DELETE" };

  bool IsText(const std::string& contentType)
  {
    return contentType.find("json") != std::string::npos ||
           contentType.find("xml") != std::string::npos ||
           0 == contentType.compare(0, 5, "text/");
  }

  mitk::RestRequest MakeRequest(const std::string& method, const std::string& path, const mitk::RestRequest& call)
  {
    if (0 != path.compare(0, 5, "/api/"))
      throw std::invalid_argument("The path must start with /api/, like /api/v1/endpoints.");

    mitk::RestRequest request;
    request.Method = method;
    request.SetTarget(path);
    request.ClientAddress = call.ClientAddress;

    if (const auto authorization = call.GetHeader("Authorization"); authorization.has_value())
      request.Headers["authorization"] = *authorization;

    return request;
  }

  mitk::McpToolResult ToToolResult(const mitk::RestResponse& response)
  {
    const auto contentType = response.ContentType.substr(0, response.ContentType.find(';'));

    if (0 == contentType.compare(0, 6, "image/") && response.Status < 400)
    {
      if (response.Body.size() > MaxImageBytes)
      {
        return mitk::McpToolResult::FromError("The image has " + std::to_string(response.Body.size()) +
          " bytes, too many to show. Ask for a smaller one with the query parameters of the endpoint, "
          "for example max_size, or width and height.");
      }

      mitk::McpToolResult result;
      result.Content.push_back(mitk::McpContent::FromImage(response.Body, contentType));

      // Endpoints describe images in headers of their own, like the size of a
      // screenshot that pixels refer to.
      for (const auto& [name, value] : response.Headers)
      {
        if (0 == name.compare(0, 7, "X-MITK-"))
          result.Content.push_back(mitk::McpContent::FromText(name + ": " + value));
      }

      return result;
    }

    std::string text;

    if (response.Status != 200)
      text = "HTTP " + std::to_string(response.Status) + "\n";

    if (response.Body.empty())
    {
      text += response.Status < 400 ? "Done, no content." : "No details.";
    }
    else if (IsText(contentType))
    {
      if (response.Body.size() > MaxTextLength)
      {
        text += response.Body.substr(0, MaxTextLength) + "\n... cut off after " + std::to_string(MaxTextLength) +
          " of " + std::to_string(response.Body.size()) +
          " characters. Ask for less, for example with query parameters like limit, offset or fields.";
      }
      else
      {
        text += response.Body;
      }
    }
    else
    {
      text += "Content of type " + contentType + ", " + std::to_string(response.Body.size()) + " bytes, not shown.";
    }

    auto result = mitk::McpToolResult::FromText(text);
    result.IsError = response.Status >= 400;
    return result;
  }
}

void mitk::AddRestApiTools(McpEndpoint& endpoint, IRestServerService* server)
{
  const nlohmann::json pathSchema = {
    { "type", "string" },
    { "description", "The path with its query, starting with /api/, like /api/v1/datastorage/nodes?limit=20" }
  };

  McpTool get;
  get.Name = "api_get";
  get.Description =
    "Read from the Workbench through its REST API with a GET request: the data in the Data Manager, "
    "properties, the render windows and their screenshots, the user manual, and more. "
    "GET /api/v1/endpoints lists all endpoints; ?path=<start of the paths>&details=true says what they "
    "take and return. JSON comes back as text, images as images.";
  get.InputSchema = {
    { "type", "object" },
    { "properties", { { "path", pathSchema } } },
    { "required", { "path" } }
  };
  get.ReadOnly = true;
  get.Handler = [server](const nlohmann::json& arguments, const RestRequest& call) {
    const auto path = arguments.value("path", std::string());
    return ToToolResult(server->HandleRequest(MakeRequest("GET", path, call)));
  };
  endpoint.AddTool(get);

  McpTool call;
  call.Name = "api_call";
  call.Description =
    "Change the Workbench through its REST API with a POST, PUT, PATCH or DELETE request, for example to "
    "set properties of data or to use a segmentation tool. Look the endpoint up first with api_get "
    "/api/v1/endpoints?path=<start of the paths>&details=true. The body is JSON.";
  call.InputSchema = {
    { "type", "object" },
    { "properties", {
      { "method", { { "type", "string" }, { "enum", ChangingMethods } } },
      { "path", pathSchema },
      { "body", { { "description", "The JSON body, if the endpoint takes one" } } }
    } },
    { "required", { "method", "path" } }
  };
  call.Handler = [server](const nlohmann::json& arguments, const RestRequest& callRequest) {
    const auto method = arguments.value("method", std::string());

    if (std::find(ChangingMethods.begin(), ChangingMethods.end(), method) == ChangingMethods.end())
      throw std::invalid_argument("The method must be POST, PUT, PATCH or DELETE. Use api_get to read.");

    auto request = MakeRequest(method, arguments.value("path", std::string()), callRequest);

    if (arguments.contains("body") && !arguments["body"].is_null())
    {
      const auto& body = arguments["body"];

      // Models sometimes pass the JSON as a string.
      request.Body = body.is_string() ? body.get<std::string>() : body.dump();
      request.Headers["content-type"] = "application/json";
    }

    return ToToolResult(server->HandleRequest(request));
  };
  endpoint.AddTool(call);
}
