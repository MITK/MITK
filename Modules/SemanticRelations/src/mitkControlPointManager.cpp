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

double CalculateDistanceInDays(mitk::SemanticTypes::ControlPoint leftControlPoint, mitk::SemanticTypes::ControlPoint rightControlPoint);

mitk::SemanticTypes::ControlPoint mitk::GenerateControlPoint(const mitk::DataNode* datanode)
{
  SemanticTypes::ControlPoint controlPoint = GetDICOMDateFromDataNode(datanode);
  controlPoint.UID = UIDGeneratorBoost::GenerateUID();

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

mitk::SemanticTypes::ControlPoint mitk::FindExistingControlPoint(const SemanticTypes::ControlPoint& controlPoint, std::vector<SemanticTypes::ControlPoint>& allControlPoints)
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

double CalculateDistanceInDays(mitk::SemanticTypes::ControlPoint leftControlPoint, mitk::SemanticTypes::ControlPoint rightControlPoint)
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
