/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestRendering.h>

#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include <mitkDataNode.h>
#include <mitkErrorResponse.h>
#include <mitkImage.h>
#include <mitkImagePixelReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkRestHelpers.h>
#include <mitkRestTime.h>
#include <mitkSliceNavigationHelper.h>

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkRenderer.h>

#include <algorithm>
#include <charconv>
#include <cmath>

int mitk::ConvertRestSliceNumber(BaseRenderer* renderer, int count, int number)
{
  return SliceNavigationHelper::IsDisplayedSliceInverted(renderer) ? count - 1 - number : number;
}

bool mitk::IsRestRenderer2D(const BaseRenderer* renderer)
{
  return renderer->GetMapperID() == BaseRenderer::Standard2D;
}

nlohmann::json mitk::DescribeRestWindow(const std::string& id, BaseRenderer* renderer)
{
  const bool is2D = IsRestRenderer2D(renderer);

  nlohmann::json json = {
    { "id", id },
    { "kind", is2D ? "2d" : "3d" },
    { "size", { renderer->GetSizeX(), renderer->GetSizeY() } },
    { "timepoint_ms", renderer->GetTime() }
  };

  if (is2D)
  {
    auto* navigation = renderer->GetSliceNavigationController();
    const auto* stepper = navigation->GetStepper();
    const auto count = static_cast<int>(stepper->GetSteps());
    const auto position = static_cast<int>(stepper->GetPos());

    json["view_direction"] = navigation->GetViewDirectionAsString();

    if (count > 0)
    {
      json["slice"] = {
        { "index", ConvertRestSliceNumber(renderer, count, position) },
        { "count", count }
      };
    }
  }

  return json;
}

nlohmann::json mitk::DescribeRestCrosshair(
  const std::array<const DataNode*, 3>& planes, const BaseRenderer* window2D, const BaseRenderer* window3D)
{
  const auto isShownIn = [&planes](const BaseRenderer* renderer) {
    for (const auto* plane : planes)
    {
      if (plane == nullptr || !plane->IsVisible(renderer))
        return false;
    }

    return true;
  };

  return {
    { "visible", isShownIn(window2D) },
    { "visible_3d", isShownIn(window3D) }
  };
}

bool mitk::GetRestQueryFlag(const RestRequest& request, const std::string& name, bool defaultValue)
{
  const auto value = request.GetQueryParam(name);

  if (!value.has_value())
    return defaultValue;

  if (*value != "true" && *value != "false")
    throw RestException(ErrorResponse::InvalidRequest(name + " must be true or false.", request.Path));

  return *value == "true";
}

int mitk::ParseRestPixelCount(const std::string& name, const std::string& value, const std::string& path)
{
  // Digits only, with a minus at most: no plus sign, no blanks.
  const auto* end = value.data() + value.size();
  int count = 0;
  const auto [last, error] = std::from_chars(value.data(), end, count);

  if (value.empty() || error != std::errc() || last != end)
    throw RestException(ErrorResponse::InvalidRequest(name + " must be a number of pixels.", path));

  return count;
}

mitk::RestScreenshotOptions mitk::ParseRestScreenshotOptions(const RestRequest& request)
{
  RestScreenshotOptions options;

  if (const auto value = request.GetQueryParam("max_size"); value.has_value())
    options.MaxSize = static_cast<unsigned int>(std::clamp(ParseRestPixelCount("max_size", *value, request.Path), 100, 4000));

  if (const auto background = request.GetQueryParam("background"); background.has_value())
  {
    // A '#' in a query value has to be encoded, which a client library that
    // encodes values itself does a second time, so the digits alone count, too.
    const auto color = ParseRestColor(background->size() == 6 ? '#' + *background : *background);

    if (*background == "transparent")
    {
      options.BackgroundMode = RestScreenshotOptions::Background::Transparent;
    }
    else if (color.has_value())
    {
      options.BackgroundMode = RestScreenshotOptions::Background::Color;
      options.BackgroundColor = *color;
    }
    else
    {
      throw RestException(ErrorResponse::InvalidRequest(
        "background must be transparent or a color like ffffff (six hex digits).", request.Path));
    }
  }

  options.Decorations = GetRestQueryFlag(request, "decorations", options.Decorations);

  if (const auto scale = request.GetQueryParam("scale"); scale.has_value())
  {
    if (*scale != "1" && *scale != "2" && *scale != "3" && *scale != "4")
      throw RestException(ErrorResponse::InvalidRequest("scale must be 1, 2, 3 or 4.", request.Path));

    options.Magnification = static_cast<unsigned int>(std::stoi(*scale));
  }

  options.CropToContent = GetRestQueryFlag(request, "crop", false);

  if (const auto padding = request.GetQueryParam("padding"); padding.has_value())
  {
    if (!options.CropToContent)
      throw RestException(ErrorResponse::InvalidRequest("padding needs crop=true.", request.Path));

    const auto value = ParseRestPixelCount("padding", *padding, request.Path);

    if (value < 0 || 2LL * value >= options.MaxSize)
      throw RestException(ErrorResponse::InvalidRequest("padding must be 0 or more and less than half of max_size.", request.Path));

    options.Padding = static_cast<unsigned int>(value);
  }

  return options;
}

bool mitk::RestGridLine::operator==(const RestGridLine& other) const
{
  return this->Position == other.Position && this->Labeled == other.Labeled;
}

std::vector<mitk::RestGridLine> mitk::GetRestGridLines(int length)
{
  std::vector<RestGridLine> lines;

  for (int position = 50; position < length; position += 50)
    lines.push_back({ position, position % 100 == 0 });

  return lines;
}

mitk::TimeStepType mitk::GetRestTimeStep(const DataNode* node, TimePointType timePoint, const std::string& path)
{
  RestTimeSelection selection;
  selection.TimePoint = timePoint;
  const auto timeSteps = selection.GetTimeSteps(node->GetData());

  if (timeSteps.empty())
  {
    throw RestException(ErrorResponse::Create("NO_DATA_AT_TIME_POINT", "Unprocessable Content",
      "\"" + node->GetName() + "\" has no data at the selected time point, " + std::to_string(selection.TimePoint) + " ms.", 422, path));
  }

  return timeSteps.front();
}

mitk::Point3D mitk::GetRestCenter(const std::vector<Point3D>& points)
{
  Point3D lower = points.front();
  Point3D upper = points.front();

  for (const auto& point : points)
  {
    for (int axis = 0; axis < 3; ++axis)
    {
      lower[axis] = std::min(lower[axis], point[axis]);
      upper[axis] = std::max(upper[axis], point[axis]);
    }
  }

  Point3D center;

  for (int axis = 0; axis < 3; ++axis)
    center[axis] = (lower[axis] + upper[axis]) / 2.0;

  return center;
}

void mitk::FocusRestRenderer2D(BaseRenderer* renderer, const std::vector<Point3D>& corners, double margin)
{
  renderer->GetSliceNavigationController()->SelectSliceByPoint(GetRestCenter(corners));

  const auto* plane = renderer->GetCurrentWorldPlaneGeometry();

  Point2D lower;
  plane->Map(corners.front(), lower);
  Point2D upper = lower;

  for (const auto& corner : corners)
  {
    Point2D point;
    plane->Map(corner, point);

    for (int axis = 0; axis < 2; ++axis)
    {
      lower[axis] = std::min(lower[axis], point[axis]);
      upper[axis] = std::max(upper[axis], point[axis]);
    }
  }

  const auto viewport = renderer->GetViewportSize();
  const auto millimetersPerPixel = (1.0 + margin) *
    std::max((upper[0] - lower[0]) / std::max(viewport[0], 1), (upper[1] - lower[1]) / std::max(viewport[1], 1));

  Point2D center;
  center[0] = (lower[0] + upper[0]) / 2.0;
  center[1] = (lower[1] + upper[1]) / 2.0;

  // Corners that coincide in the plane, like those of a line that runs through it, give no
  // zoom: a scale of zero would blank the window and leave its mouse zoom without effect.
  if (millimetersPerPixel > 0.0)
    renderer->GetCameraController()->SetScaleFactorInMMPerDisplayUnit(millimetersPerPixel);

  renderer->GetCameraController()->MoveCameraToPoint(center);
}

void mitk::FocusRestRenderer3D(BaseRenderer* baseRenderer, const std::vector<Point3D>& corners, double margin)
{
  auto* renderer = baseRenderer->GetVtkRenderer();
  auto* camera = renderer->GetActiveCamera();
  const auto center = GetRestCenter(corners);

  double direction[3];
  camera->GetDirectionOfProjection(direction);

  double up[3];
  camera->GetViewUp(up);

  double right[3];
  vtkMath::Cross(direction, up, right);
  vtkMath::Normalize(right);
  vtkMath::Cross(right, direction, up);
  vtkMath::Normalize(up);

  const auto* size = renderer->GetSize();
  const auto aspect = size[1] > 0 ? static_cast<double>(size[0]) / size[1] : 1.0;

  // Lateral extents of the corners and their depth toward the camera, around the center.
  double extent = 0.0;
  double distance = 0.0;

  const auto halfAngle = std::tan(vtkMath::RadiansFromDegrees(camera->GetViewAngle()) / 2.0);
  const auto tanVertical = camera->GetUseHorizontalViewAngle() ? halfAngle / aspect : halfAngle;
  const auto tanHorizontal = tanVertical * aspect;

  for (const auto& corner : corners)
  {
    const double offset[3] = { corner[0] - center[0], corner[1] - center[1], corner[2] - center[2] };
    const auto x = std::abs(vtkMath::Dot(offset, right)) * (1.0 + margin);
    const auto y = std::abs(vtkMath::Dot(offset, up)) * (1.0 + margin);
    const auto depth = -vtkMath::Dot(offset, direction);

    extent = std::max({ extent, y, x / aspect });
    distance = std::max({ distance, depth + y / tanVertical, depth + x / tanHorizontal });
  }

  // Corners that coincide give neither a scale nor a distance, so the camera keeps its own.
  if (camera->GetParallelProjection() && extent > 0.0)
    camera->SetParallelScale(extent);

  if (camera->GetParallelProjection() || distance <= 0.0)
    distance = camera->GetDistance();

  camera->SetFocalPoint(center[0], center[1], center[2]);
  camera->SetPosition(center[0] - direction[0] * distance, center[1] - direction[1] * distance, center[2] - direction[2] * distance);
  camera->SetViewUp(up);
  renderer->ResetCameraClippingRange();
}

std::optional<double> mitk::GetRestImageValue(const Image* image, const Point3D& point, TimePointType timePoint)
{
  const auto* timeGeometry = image->GetTimeGeometry();

  if (!timeGeometry->IsValidTimePoint(timePoint) || image->GetDimension() < 3)
    return std::nullopt;

  const auto timeStep = timeGeometry->TimePointToTimeStep(timePoint);
  const auto* geometry = image->GetGeometry(timeStep);

  if (!geometry->IsInside(point))
    return std::nullopt;

  itk::Index<3> index;
  geometry->WorldToIndex(point, index);

  ScalarType value = 0;
  mitkPixelTypeMultiplex5(mitk::FastSinglePixelAccess, image->GetChannelDescriptor().GetPixelType(),
    mitk::Image::ConstPointer(image), image->GetVolumeData(timeStep), index, value, 0);

  return value;
}
