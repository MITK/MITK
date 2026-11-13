/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestPoints.h>

#include <mitkBaseRenderer.h>
#include <mitkErrorResponse.h>
#include <mitkRestRoute.h>

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
  const std::array<std::string, 4> WindowIds = { "axial", "sagittal", "coronal", "3d" };

  [[noreturn]] void ThrowInvalid(const std::string& detail, const std::string& path)
  {
    throw mitk::RestException(mitk::ErrorResponse::InvalidRequest(detail, path));
  }

  bool IsNumberArray(const nlohmann::json& json, std::size_t size)
  {
    if (!json.is_array() || json.size() != size)
      return false;

    for (const auto& value : json)
    {
      if (!value.is_number())
        return false;
    }

    return true;
  }
}

mitk::BaseRenderer* mitk::GetRestRenderer(const std::string& windowId, const std::string& path)
{
  const auto id = std::find(WindowIds.begin(), WindowIds.end(), windowId);

  if (id == WindowIds.end())
  {
    throw RestException(ErrorResponse::RenderWindowNotFound(windowId +
      " (the windows are axial, sagittal, coronal and 3d)", path));
  }

  // The standard display names its renderers after the order of WindowIds.
  auto* renderer = BaseRenderer::GetByName("stdmulti.widget" + std::to_string(id - WindowIds.begin()));

  if (renderer == nullptr)
    throw RestException(ErrorResponse::RenderWindowNotAvailable(path));

  return renderer;
}

std::vector<mitk::Point3D> mitk::GetRestWorldPoints(const nlohmann::json& body, const std::string& path, std::size_t minCount)
{
  std::vector<Point3D> points;

  if (body.contains("points"))
  {
    if (!body["points"].is_array())
      ThrowInvalid("\"points\" must be a list of [x, y, z] in millimeters.", path);

    for (const auto& item : body["points"])
    {
      if (!IsNumberArray(item, 3))
        ThrowInvalid("Each point must be [x, y, z] in millimeters.", path);

      Point3D point;

      for (int i = 0; i < 3; ++i)
        point[i] = item[i].get<double>();

      points.push_back(point);
    }
  }
  else if (body.contains("pixels"))
  {
    if (!body["pixels"].is_array())
      ThrowInvalid("\"pixels\" must be a list of [x, y] in a screenshot.", path);

    if (!body.contains("window") || !body["window"].is_string())
      ThrowInvalid("Pixels need \"window\", the id of the render window of the screenshot.", path);

    if (!body.contains("image_size") || !IsNumberArray(body["image_size"], 2) ||
        body["image_size"][0].get<double>() <= 0 || body["image_size"][1].get<double>() <= 0)
    {
      ThrowInvalid("Pixels need \"image_size\": [width, height] of the screenshot.", path);
    }

    const auto* renderer = GetRestRenderer(body["window"].get<std::string>(), path);

    if (renderer->GetMapperID() != BaseRenderer::Standard2D)
      ThrowInvalid("Pixels map onto the plane of a 2D window only, not of the 3D window.", path);

    const double imageWidth = body["image_size"][0].get<double>();
    const double imageHeight = body["image_size"][1].get<double>();
    const double windowWidth = renderer->GetSizeX();
    const double windowHeight = renderer->GetSizeY();

    if (windowWidth <= 0 || windowHeight <= 0)
      throw RestException(ErrorResponse::RenderWindowNotAvailable(path));

    // Screenshots shrink the window evenly, rounded to whole pixels; a cropped one does not.
    if (std::abs(imageWidth * windowHeight - imageHeight * windowWidth) > 2 * std::max(windowWidth, windowHeight))
    {
      ThrowInvalid("\"image_size\" does not have the proportions of the window. Pixels map only from an uncropped "
        "screenshot taken since the window last changed its size.", path);
    }

    const double scaleX = windowWidth / imageWidth;
    const double scaleY = windowHeight / imageHeight;

    for (const auto& item : body["pixels"])
    {
      if (!IsNumberArray(item, 2))
        ThrowInvalid("Each pixel must be [x, y] in the screenshot.", path);

      const double x = item[0].get<double>();
      const double y = item[1].get<double>();

      if (x < 0 || x > imageWidth || y < 0 || y > imageHeight)
        ThrowInvalid("Each pixel must lie within \"image_size\".", path);

      // Display coordinates start at the bottom left, screenshots at the top left.
      Point2D display;
      display[0] = x * scaleX;
      display[1] = renderer->GetSizeY() - y * scaleY;

      Point3D point;
      renderer->DisplayToWorld(display, point);
      points.push_back(point);
    }
  }
  else
  {
    ThrowInvalid("The body needs \"points\" in millimeters or \"pixels\" of a screenshot.", path);
  }

  if (points.size() < minCount)
    ThrowInvalid("The body needs at least " + std::to_string(minCount) + " points.", path);

  return points;
}

nlohmann::json mitk::ToRestJson(const Point3D& point)
{
  auto json = nlohmann::json::array();

  for (int i = 0; i < 3; ++i)
    json.push_back(std::round(point[i] * 1000.0) / 1000.0);

  return json;
}
