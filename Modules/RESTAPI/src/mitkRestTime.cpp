/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestTime.h>

#include <mitkBaseData.h>
#include <mitkErrorResponse.h>
#include <mitkRenderingManager.h>
#include <mitkRestHelpers.h>
#include <mitkRestRoute.h>
#include <mitkTimeNavigationController.h>

#include <cmath>
#include <limits>

namespace
{
  /** A bound of a time step, null where it is none: data of a single time step reaches to the limits of the type. */
  nlohmann::json ToBound(mitk::TimePointType timePoint)
  {
    constexpr auto limit = std::numeric_limits<mitk::TimePointType>::max() / 2;

    if (!std::isfinite(timePoint) || std::abs(timePoint) >= limit)
      return nullptr;

    return timePoint;
  }
}

mitk::RestTimeSelection mitk::RestTimeSelection::FromRequest(const RestRequest& request)
{
  RestTimeSelection selection;
  const auto value = request.GetQueryParam("timepoint_ms");

  if (!value.has_value())
  {
    selection.TimePoint = RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
    return selection;
  }

  if (*value == "all")
  {
    selection.All = true;
    return selection;
  }

  if (const auto timePoint = ParseRestDecimal(*value); timePoint.has_value())
  {
    selection.TimePoint = *timePoint;
    return selection;
  }

  throw RestException(ErrorResponse::InvalidRequest("timepoint_ms must be a time point in milliseconds or all.", request.Path));
}

std::vector<mitk::TimeStepType> mitk::RestTimeSelection::GetTimeSteps(const BaseData* data) const
{
  std::vector<TimeStepType> timeSteps;
  const auto* timeGeometry = data->GetTimeGeometry();

  if (this->All)
  {
    for (TimeStepType timeStep = 0; timeStep < timeGeometry->CountTimeSteps(); ++timeStep)
      timeSteps.push_back(timeStep);
  }
  else if (timeGeometry->IsValidTimePoint(this->TimePoint))
  {
    // TimePointToTimeStep() does not clamp, hence only for valid time points.
    timeSteps.push_back(timeGeometry->TimePointToTimeStep(this->TimePoint));
  }

  return timeSteps;
}

nlohmann::json mitk::RestTimeSelection::ToJson() const
{
  return this->All ? nlohmann::json("all") : nlohmann::json(this->TimePoint);
}

nlohmann::json mitk::DescribeRestTimeStep(const BaseData* data, TimeStepType timeStep)
{
  const auto* timeGeometry = data->GetTimeGeometry();

  return {
    { "min_timepoint_ms", ToBound(timeGeometry->GetMinimumTimePoint(timeStep)) },
    { "max_timepoint_ms", ToBound(timeGeometry->GetMaximumTimePoint(timeStep)) }
  };
}

nlohmann::json mitk::DescribeRestTimeSteps(const BaseData* data)
{
  auto timeSteps = nlohmann::json::array();

  for (TimeStepType timeStep = 0; timeStep < data->GetTimeGeometry()->CountTimeSteps(); ++timeStep)
    timeSteps.push_back(DescribeRestTimeStep(data, timeStep));

  return timeSteps;
}
