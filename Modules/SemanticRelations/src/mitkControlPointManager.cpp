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
#include "mitkSemanticRelationException.h"
#include "mitkUIDGeneratorBoost.h"

// mitk core
#include <mitkPropertyNameHelper.h>

mitk::SemanticTypes::ControlPoint mitk::GenerateControlPoint(const DataNode* datanode)
{
  SemanticTypes::ControlPoint controlPoint;
  try
  {
    controlPoint = GetDICOMDateFromDataNode(datanode);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot generate a control point from the DICOM tag of the given data node";
  }

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

mitk::SemanticTypes::ControlPoint mitk::FindExistingControlPoint(const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ControlPointVector& allControlPoints)
{
  for (const auto& currentControlPoint : allControlPoints)
  {
    if (controlPoint.date == currentControlPoint.date)
    {
      return currentControlPoint;
    }
  }

  return SemanticTypes::ControlPoint();
}

mitk::SemanticTypes::ControlPoint mitk::FindClosestControlPoint(const SemanticTypes::ControlPoint& controlPoint, SemanticTypes::ControlPointVector& allControlPoints)
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
    if (controlPoint.date < it->date)
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
  double distanceToNextExaminationPeriod = nextControlPoint.DistanceInDays(controlPoint);
  double distanceToPreviousExaminationPeriod = previousControlPoint.DistanceInDays(controlPoint);

  SemanticTypes::ControlPoint closestControlPoint;
  int closestDistanceInDays = 0;
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

  int THRESHOLD_DISTANCE_IN_DAYS = 30;
  if (closestDistanceInDays <= THRESHOLD_DISTANCE_IN_DAYS)
  {
    return closestControlPoint;
  }

  return SemanticTypes::ControlPoint();
}

mitk::SemanticTypes::ExaminationPeriod mitk::FindExaminationPeriod(const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ExaminationPeriodVector& allExaminationPeriods)
{
  for (const auto& examinationPeriod : allExaminationPeriods)
  {
    for (const auto& UID : examinationPeriod.controlPointUIDs)
    {
      if (controlPoint.UID == UID)
      {
        return examinationPeriod;
      }
    }
  }

  return SemanticTypes::ExaminationPeriod();
}

void mitk::SortExaminationPeriods(SemanticTypes::ExaminationPeriodVector& allExaminationPeriods, const SemanticTypes::ControlPointVector& allControlPoints)
{
  auto lambda = [allControlPoints](const SemanticTypes::ExaminationPeriod& leftExaminationPeriod, const SemanticTypes::ExaminationPeriod& rightExaminationPeriod)
  {
    if (leftExaminationPeriod.controlPointUIDs.empty())
    {
      return true;
    }

    if (rightExaminationPeriod.controlPointUIDs.empty())
    {
      return false;
    }
    const auto leftUID = leftExaminationPeriod.controlPointUIDs.front();
    const auto rightUID = rightExaminationPeriod.controlPointUIDs.front();
    const auto& leftControlPoint = GetControlPointByUID(leftUID, allControlPoints);
    const auto& rightControlPoint = GetControlPointByUID(rightUID, allControlPoints);

    return leftControlPoint.date < rightControlPoint.date;
  };

  std::sort(allExaminationPeriods.begin(), allExaminationPeriods.end(), lambda);
}
