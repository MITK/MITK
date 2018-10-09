/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// semantic relations module
#include "mitkControlPointManager.h"
#include "mitkDICOMHelper.h"
#include "mitkUIDGeneratorBoost.h"

// mitk core
#include <mitkPropertyNameHelper.h>

double CalculateDistanceInDays(const mitk::SemanticTypes::ControlPoint& leftControlPoint, const mitk::SemanticTypes::ControlPoint& rightControlPoint);

mitk::SemanticTypes::ControlPoint mitk::GenerateControlPoint(const DataNode* datanode)
{
  SemanticTypes::ControlPoint controlPoint = GetDICOMDateFromDataNode(datanode);
  controlPoint.UID = UIDGeneratorBoost::GenerateUID();

  return controlPoint;
}

mitk::SemanticTypes::ControlPoint mitk::GetControlPointByUID(const SemanticTypes::ID& controlPointUID, const std::vector<SemanticTypes::ControlPoint>& allControlPoints)
{
  auto lambda = [&controlPointUID](const SemanticTypes::ControlPoint& currentControlPoint)
  {
    return currentControlPoint.UID == controlPointUID;
  };

  const auto existingControlPoint = std::find_if(allControlPoints.begin(), allControlPoints.end(), lambda);

  mitk::SemanticTypes::ControlPoint controlPoint;
  if (existingControlPoint != allControlPoints.end())
  {
    controlPoint = *existingControlPoint;
  }

  return controlPoint;
}

std::string mitk::GetControlPointAsString(const SemanticTypes::ControlPoint& controlPoint)
{
  std::stringstream controlPointAsString;
  controlPointAsString << std::to_string(controlPoint.year) << "-"
    << std::setfill('0') << std::setw(2) << std::to_string(controlPoint.month) << "-"
    << std::setfill('0') << std::setw(2) << std::to_string(controlPoint.day);

  return controlPointAsString.str();
}

mitk::SemanticTypes::ControlPoint mitk::FindExistingControlPoint(const SemanticTypes::ControlPoint& controlPoint, const std::vector<SemanticTypes::ControlPoint>& allControlPoints)
{
  for (const auto& currentControlPoint : allControlPoints)
  {
    if (controlPoint == currentControlPoint)
    {
      return currentControlPoint;
    }
  }

  return SemanticTypes::ControlPoint();
}


mitk::SemanticTypes::ControlPoint mitk::FindClosestControlPoint(const SemanticTypes::ControlPoint& controlPoint, std::vector<SemanticTypes::ControlPoint>& allControlPoints)
{
  if (allControlPoints.empty())
  {
    return SemanticTypes::ControlPoint();
  }

  // sort the vector of control points for easier lookup
  std::sort(allControlPoints.begin(), allControlPoints.end());
  // new control point does not match an existing control point
  // check if the control point is close to an already existing control point
  std::vector<SemanticTypes::ControlPoint>::const_iterator it;
  for (it = allControlPoints.begin(); it != allControlPoints.end(); ++it)
  {
    if (controlPoint < *it)
    {
      break;
    }
  }

  SemanticTypes::ControlPoint nextControlPoint;
  SemanticTypes::ControlPoint previousControlPoint;
  if (it == allControlPoints.begin())
  {
    // new date is smaller ("older") than the smallest already existing control point
    nextControlPoint = *it;
  }
  else if (it != allControlPoints.end())
  {
    // new date is greater ("newer") than an already existing control point,
    // but smaller ("older") than another already existing control point
    nextControlPoint = *it;
    previousControlPoint = *(--it);
  }
  else
  {
    // new date is greater ("newer") than the greatest already existing control point
    previousControlPoint = *(--it);
  }

  // test distance to next and previous time period
  double distanceToNextExaminationPeriod = CalculateDistanceInDays(nextControlPoint, controlPoint);
  double distanceToPreviousExaminationPeriod = CalculateDistanceInDays(previousControlPoint, controlPoint);

  SemanticTypes::ControlPoint closestControlPoint;
  double closestDistanceInDays = 0.0;
  if (distanceToNextExaminationPeriod < distanceToPreviousExaminationPeriod)
  {
    // control point is closer to the next control point
    closestControlPoint = nextControlPoint;
    closestDistanceInDays = distanceToNextExaminationPeriod;
  }
  else
  {
    // control point is closer to the previous control point
    closestControlPoint = previousControlPoint;
    closestDistanceInDays = distanceToPreviousExaminationPeriod;
  }

  double THRESHOLD_DISTANCE_IN_DAYS = 30.0;
  if (std::abs(closestDistanceInDays) < THRESHOLD_DISTANCE_IN_DAYS)
  {
    return closestControlPoint;
  }

  return SemanticTypes::ControlPoint();
}

mitk::SemanticTypes::ExaminationPeriod mitk::FindExaminationPeriod(const SemanticTypes::ControlPoint& controlPoint, const std::vector<SemanticTypes::ExaminationPeriod>& allExaminationPeriods)
{
  for (const auto& examinationPeriod : allExaminationPeriods)
  {
    for (const auto& UID : examinationPeriod.controlPointIDs)
    {
      if (controlPoint.UID == UID)
      {
        return examinationPeriod;
      }
    }
  }

  return SemanticTypes::ExaminationPeriod();
}

double CalculateDistanceInDays(const mitk::SemanticTypes::ControlPoint& leftControlPoint, const mitk::SemanticTypes::ControlPoint& rightControlPoint)
{
  std::tm leftTimeStructure = { 0, 0, 0, leftControlPoint.day,  leftControlPoint.month - 1,   leftControlPoint.year - 1900 };
  std::tm rightTimeStructure = { 0, 0, 0, rightControlPoint.day, rightControlPoint.month - 1,  rightControlPoint.year - 1900 };

  time_t leftTime = mktime(&leftTimeStructure);
  time_t rightTime = mktime(&rightTimeStructure);

  if (leftTime == -1 || rightTime == -1)
  {
    // date is not initialized, no difference can be computed
    return std::numeric_limits<double>::max();
  }

  // compute distance here
  double secondsPerDay = 60 * 60 * 24;
  double timeDifferenceInDays = std::difftime(leftTime, rightTime) / secondsPerDay;

  return timeDifferenceInDays;
}
