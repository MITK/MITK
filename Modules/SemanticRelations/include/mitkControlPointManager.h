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

#ifndef MITKCONTROLPOINTMANAGER_H
#define MITKCONTROLPOINTMANAGER_H

// semantic relations module
#include "mitkSemanticTypes.h"
#include <MitkSemanticRelationsExports.h>

// mitk core
#include <mitkDataNode.h>

namespace ControlPointManager
{
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GenerateControlPoint(const std::vector<mitk::DataNode::Pointer>& dataOfControlPoint);

  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GenerateControlPoint(const mitk::DataNode* datanode);

  SemanticTypes::Date GetStartPoint(const std::vector<mitk::DataNode::Pointer>& dataOfControlPoint);

  SemanticTypes::Date GetEndPoint(const std::vector<mitk::DataNode::Pointer>& dataOfControlPoint);

  SemanticTypes::Date GetDateFromDataNode(const mitk::DataNode* dataNode);

  SemanticTypes::Date GetDateFromString(std::string& dateAsString);

  MITKSEMANTICRELATIONS_EXPORT bool InsideControlPoint(const mitk::DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint);

  MITKSEMANTICRELATIONS_EXPORT bool InsideControlPoint(const SemanticTypes::Date& date, const SemanticTypes::ControlPoint& controlPoint);

  MITKSEMANTICRELATIONS_EXPORT bool CheckForOverlap(const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ControlPoint& neighboringControlPoint);

  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint FindControlPoint(const mitk::DataNode* datanode, std::vector<SemanticTypes::ControlPoint>& allControlPoints);

  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint ExtendClosestControlPoint(const mitk::DataNode* dataNode, std::vector<SemanticTypes::ControlPoint>& allControlPoints);

  void ExtendControlPoint(SemanticTypes::ControlPoint& controlPoint, SemanticTypes::Date date);

  double CalculateDistanceInDays(SemanticTypes::Date neighboringDicomDate, SemanticTypes::Date newDicomDate);

} // namespace ControlPointManager

#endif // MITKCONTROLPOINTMANAGER_H
