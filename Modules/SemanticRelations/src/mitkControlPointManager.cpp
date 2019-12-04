/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations module
#include "mitkControlPointManager.h"
#include "mitkDICOMHelper.h"
#include "mitkRelationStorage.h"
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
    mitkReThrow(e) << "Cannot generate a control point from the DICOM tag of the given data node.";
  }

  controlPoint.UID = UIDGeneratorBoost::GenerateUID();
  return controlPoint;
}

mitk::SemanticTypes::ControlPoint mitk::GetControlPointByUID(const SemanticTypes::CaseID& caseID, const SemanticTypes::ID& controlPointUID)
{
  auto lambda = [&controlPointUID](const SemanticTypes::ControlPoint& currentControlPoint)
  {
    return currentControlPoint.UID == controlPointUID;
  };

  SemanticTypes::ControlPointVector allControlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);
  const auto existingControlPoint = std::find_if(allControlPoints.begin(), allControlPoints.end(), lambda);

  mitk::SemanticTypes::ControlPoint controlPoint;
  if (existingControlPoint != allControlPoints.end())
  {
    controlPoint = *existingControlPoint;
  }

  return controlPoint;
}

mitk::SemanticTypes::ControlPoint mitk::FindExistingControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::ControlPointVector allControlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);
  for (const auto& currentControlPoint : allControlPoints)
  {
    if (controlPoint.date == currentControlPoint.date)
    {
      return currentControlPoint;
    }
  }

  return SemanticTypes::ControlPoint();
}

mitk::SemanticTypes::ControlPoint mitk::FindClosestControlPoint(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::ControlPointVector allControlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);
  if (allControlPoints.empty())
  {
    return SemanticTypes::ControlPoint();
  }

  // sort the vector of control points for easier lookup
  std::sort(allControlPoints.begin(), allControlPoints.end());
  // new control point does not match an existing control point
  // check if the control point is close to an already existing control point
  SemanticTypes::ControlPointVector::const_iterator it;
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

mitk::SemanticTypes::ExaminationPeriod mitk::FindContainingExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::ExaminationPeriodVector allExaminationPeriods = RelationStorage::GetAllExaminationPeriodsOfCase(caseID);
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

mitk::SemanticTypes::ExaminationPeriod mitk::FindFittingExaminationPeriod(const SemanticTypes::CaseID& caseID, const SemanticTypes::ControlPoint& controlPoint)
{
  SemanticTypes::ExaminationPeriod specificExaminationPeriod;
  SemanticTypes::ControlPoint specificControlPoint;
  // find the closest control point
  SemanticTypes::ControlPoint existingControlPoint = FindExistingControlPoint(caseID, controlPoint);
  if (!existingControlPoint.UID.empty())
  {
    specificControlPoint = existingControlPoint;
  }
  else
  {
    auto closestControlPoint = FindClosestControlPoint(caseID, controlPoint);
    if (!closestControlPoint.UID.empty())
    {
      specificControlPoint = closestControlPoint;
    }
  }

  // find the containing examination period
  return FindContainingExaminationPeriod(caseID, specificControlPoint);
}

mitk::SemanticTypes::ExaminationPeriod mitk::FindFittingExaminationPeriod(const DataNode* dataNode)
{
  SemanticTypes::CaseID caseID = "";
  SemanticTypes::ControlPoint controlPoint;
  try
  {
    caseID = GetCaseIDFromDataNode(dataNode);
    controlPoint = GetDICOMDateFromDataNode(dataNode);
  }
  catch (SemanticRelationException& e)
  {
    mitkReThrow(e) << "Cannot find an examination period.";
  }

  return FindFittingExaminationPeriod(caseID, controlPoint);
}

void mitk::SortAllExaminationPeriods(const SemanticTypes::CaseID& caseID, SemanticTypes::ExaminationPeriodVector& allExaminationPeriods)
{
  SemanticTypes::ControlPointVector controlPoints = RelationStorage::GetAllControlPointsOfCase(caseID);
  // sort the vector of control points for the timeline
  std::sort(controlPoints.begin(), controlPoints.end());

  auto lambda = [&caseID](const SemanticTypes::ExaminationPeriod& leftExaminationPeriod, const SemanticTypes::ExaminationPeriod& rightExaminationPeriod)
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
    const auto& leftControlPoint = GetControlPointByUID(caseID, leftUID);
    const auto& rightControlPoint = GetControlPointByUID(caseID, rightUID);

    return leftControlPoint.date < rightControlPoint.date;
  };

  std::sort(allExaminationPeriods.begin(), allExaminationPeriods.end(), lambda);
}
