/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestHelpers.h>

#include <mitkErrorResponse.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleRegistry.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

void mitk::ThrowRestError(const std::string& code, const std::string& title, const std::string& detail, int status, const std::string& path)
{
  throw RestException(ErrorResponse::Create(code, title, detail, status, path));
}

mitk::DataNodeHandle mitk::FindRestNode(const IRestServerService& server, const std::string& uid, const std::string& path)
{
  auto node = server.FindNode(uid);

  if (node == nullptr)
    throw RestException(ErrorResponse::NodeNotFound(uid, path));

  return node;
}

mitk::DataNodeHandle mitk::FindRestNode(const IRestServerService& server, const nlohmann::json& uid, const std::string& path)
{
  if (!uid.is_string())
    throw RestException(ErrorResponse::InvalidRequest("A node is given by its uid, a string.", path));

  return FindRestNode(server, uid.get<std::string>(), path);
}

std::optional<mitk::Color> mitk::ParseRestColor(const std::string& text)
{
  if (text.size() != 7 || text[0] != '#' ||
      !std::all_of(text.begin() + 1, text.end(), [](unsigned char c) { return std::isxdigit(c) != 0; }))
  {
    return std::nullopt;
  }

  Color color;

  for (int i = 0; i < 3; ++i)
    color[i] = std::stoi(text.substr(1 + 2 * i, 2), nullptr, 16) / 255.0f;

  return color;
}

std::string mitk::ToRestColor(const Color& color)
{
  char text[8];
  std::snprintf(text, sizeof(text), "#%02x%02x%02x",
    static_cast<int>(std::lround(color[0] * 255)),
    static_cast<int>(std::lround(color[1] * 255)),
    static_cast<int>(std::lround(color[2] * 255)));

  return text;
}

mitk::RestRouteRegistration::RestRouteRegistration()
{
  auto* context = us::ModuleRegistry::GetModule(1)->GetModuleContext();
  const auto reference = context->GetServiceReference<IRestServerService>();
  m_Server = reference ? context->GetService(reference) : nullptr;
}

mitk::RestRouteRegistration::~RestRouteRegistration()
{
  if (m_Server == nullptr)
    return;

  for (const auto id : m_RouteIds)
    m_Server->RemoveRoute(id);
}

mitk::IRestServerService* mitk::RestRouteRegistration::GetServer() const
{
  return m_Server;
}

void mitk::RestRouteRegistration::Add(const std::vector<RestRoute>& routes)
{
  if (m_Server == nullptr)
    return;

  for (const auto& route : routes)
    m_RouteIds.push_back(m_Server->AddRoute(route));
}
