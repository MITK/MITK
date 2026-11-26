/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRestRouter.h"

#include <mitkExceptionMacro.h>

#include <algorithm>
#include <array>
#include <mutex>

namespace
{
  const std::array<std::string, 5> Methods = { "GET", "POST", "PUT", "PATCH", "DELETE" };
  const std::string GetMethod = "GET";

  /** Splits after the leading slash and keeps empty segments, so "/docs/" and "/docs" differ. */
  std::vector<std::string> Split(const std::string& path)
  {
    std::vector<std::string> segments;
    std::size_t start = 1;

    while (true)
    {
      const auto end = path.find('/', start);
      segments.push_back(path.substr(start, end == std::string::npos ? std::string::npos : end - start));

      if (end == std::string::npos)
        break;

      start = end + 1;
    }

    return segments;
  }

  bool IsParameter(const std::string& segment)
  {
    return segment.size() > 1 && segment.front() == ':';
  }

  bool HaveSameShape(const std::vector<std::string>& a, const std::vector<std::string>& b)
  {
    if (a.size() != b.size())
      return false;

    for (std::size_t i = 0; i < a.size(); ++i)
    {
      if (IsParameter(a[i]) != IsParameter(b[i]))
        return false;

      if (!IsParameter(a[i]) && a[i] != b[i])
        return false;
    }

    return true;
  }

  /** Whether \a a has the literal segment at the first position where one path has a literal and the other a parameter. */
  bool HasEarlierLiteral(const std::vector<std::string>& a, const std::vector<std::string>& b)
  {
    for (std::size_t i = 0; i < a.size() && i < b.size(); ++i)
    {
      if (IsParameter(a[i]) != IsParameter(b[i]))
        return IsParameter(b[i]);
    }

    return false;
  }

  bool HasRepeatedParameter(const std::vector<std::string>& segments)
  {
    for (auto segment = segments.begin(); segment != segments.end(); ++segment)
    {
      if (IsParameter(*segment) && std::find(segment + 1, segments.end(), *segment) != segments.end())
        return true;
    }

    return false;
  }
}

mitk::RestRouteId mitk::RestRouter::Add(RestRoute route)
{
  if (std::find(Methods.begin(), Methods.end(), route.Method) == Methods.end())
    mitkThrow() << "Unknown HTTP method \"" << route.Method << "\" for the route " << route.Path;

  if (route.Path.empty() || route.Path.front() != '/')
    mitkThrow() << "The path of a route must start with a slash: \"" << route.Path << "\"";

  if (!route.Handler)
    mitkThrow() << "The route " << route.Method << " " << route.Path << " has no handler";

  Entry entry;
  entry.Segments = Split(route.Path);

  if (HasRepeatedParameter(entry.Segments))
    mitkThrow() << "The route " << route.Method << " " << route.Path << " names a parameter twice";

  entry.LiteralSegmentCount = static_cast<int>(std::count_if(entry.Segments.begin(), entry.Segments.end(),
    [](const std::string& segment) { return !IsParameter(segment); }));

  std::unique_lock lock(m_Mutex);

  for (const auto& existing : m_Entries)
  {
    if (existing.Route.Method == route.Method && HaveSameShape(existing.Segments, entry.Segments))
      mitkThrow() << "There already is a route " << route.Method << " " << existing.Route.Path;
  }

  entry.Id = m_NextId++;
  entry.Route = std::move(route);
  m_Entries.push_back(std::move(entry));

  return m_Entries.back().Id;
}

void mitk::RestRouter::Remove(RestRouteId id)
{
  std::unique_lock lock(m_Mutex);

  m_Entries.erase(std::remove_if(m_Entries.begin(), m_Entries.end(),
    [id](const Entry& entry) { return entry.Id == id; }), m_Entries.end());
}

std::optional<mitk::RestRouter::Match> mitk::RestRouter::Find(const std::string& method, const std::string& path) const
{
  if (path.empty() || path.front() != '/')
    return std::nullopt;

  const auto segments = Split(path);

  // httplib hands HEAD requests to the GET handlers and drops the body.
  const std::string& routeMethod = method == "HEAD" ? GetMethod : method;

  std::shared_lock lock(m_Mutex);

  const Entry* best = nullptr;

  for (const auto& entry : m_Entries)
  {
    if (entry.Route.Method != routeMethod || entry.Segments.size() != segments.size())
      continue;

    bool matches = true;

    for (std::size_t i = 0; i < segments.size() && matches; ++i)
    {
      matches = IsParameter(entry.Segments[i])
        ? !segments[i].empty()
        : entry.Segments[i] == segments[i];
    }

    if (!matches)
      continue;

    if (best == nullptr || entry.LiteralSegmentCount > best->LiteralSegmentCount ||
        (entry.LiteralSegmentCount == best->LiteralSegmentCount && HasEarlierLiteral(entry.Segments, best->Segments)))
    {
      best = &entry;
    }
  }

  if (best == nullptr)
    return std::nullopt;

  Match match;
  match.Handler = best->Route.Handler;

  for (std::size_t i = 0; i < segments.size(); ++i)
  {
    if (IsParameter(best->Segments[i]))
      match.PathParams[best->Segments[i].substr(1)] = segments[i];
  }

  return match;
}

std::vector<mitk::RestRoute> mitk::RestRouter::GetRoutes(const std::string& prefix) const
{
  std::vector<RestRoute> routes;

  std::shared_lock lock(m_Mutex);

  for (const auto& entry : m_Entries)
  {
    if (0 == entry.Route.Path.compare(0, prefix.size(), prefix))
      routes.push_back(entry.Route);
  }

  return routes;
}
