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

namespace mitk
{
  /*
  * @brief Provides helper functions that are needed to work with control points.
  *
  *   These functions help to generate new control points, check for overlapping / containing control points or provide functionality
  *   to find a fitting control point or even extend an already existing control point.
  */
  /*
  * @brief Generates a control point from a given vector of data nodes.
  *        The start point and the end point for the newly generated control point are determined by the
  *        earliest and the latest date of all the given data nodes, using 'GetStartPoint' and 'GetEndPoint'.
  *
  * @par dataNodes   A vector of data node pointers that contain all the dates, that should be included in the newly generated control point.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GenerateControlPoint(const std::vector<mitk::DataNode::Pointer>& dataNodes);
  /*
  * @brief Generates a control point from a given data node.
  *        The date is extracted from the data node by using the 'DICOMHelper::GetDateFromDataNode'-function. Inside the helper-function
  *        the randomly generated UID is assigned to the UID of the date.
  *        'GenerateControlPoint(const SemanticTypes::Date&)' is used to generate a new control point from this extracted date.
  *
  * @par datanode   A data node pointer, whose date should be included in the newly generated control point.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GenerateControlPoint(const mitk::DataNode* datanode);
  /*
  * @brief Generates a control point from a given date.
  *        Providing a single date, this date is assigned to both the start point and the end point of the newly created control point.
  *        In order to reference both dates as individual instances, a new randomly generated UID is assigned to the UID of the end point.
  *
  * @par date     A single date that is used to define both the start point and the end point of the newly created control point.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GenerateControlPoint(const SemanticTypes::Date& date);
  /*
  * @brief Returns a string that displays the start point and the end point of the given control point in the format "YYYY-MM-DD to YYYY-MM-DD".
  *        If the end point is the same as the start point, only one date is returned in the format "YYYY-MM-DD".
  *        This function is used in the GUI to display the dates of a control point as header in the "information-type - control-point"-matrix.
  *
  * @par controlPoint   The control point to convert into a string.
  */
  MITKSEMANTICRELATIONS_EXPORT std::string GetControlPointAsString(const SemanticTypes::ControlPoint& controlPoint);
  /*
  * @brief Checks if the date that is extracted from the given data node lies inside the given control point.
  *        To lie inside the given control point,
  *         - the extracted date has to be greater or equal to the start point of the control point
  *         AND
  *         - the date has to be smaller or equal to the end point of the control point
  *
  * @par dataNode       The data node whose extracted date has to be checked.
  * @par controlPoint   The control point in which the extracted date of the data node may lie.
  *
  * @return             True, if the extracted date lies inside the given control point; false otherwise.
  */
  MITKSEMANTICRELATIONS_EXPORT bool InsideControlPoint(const mitk::DataNode* dataNode, const SemanticTypes::ControlPoint& controlPoint);
  /*
  * @brief Checks if the given date lies inside the given control point.
  *        To lie inside the given control point,
  *         - the date has to be greater or equal to the start point of the control point
  *         AND
  *         - the date has to be smaller or equal to the end point of the control point
  *
  * @par date           The date to be checked.
  * @par controlPoint   The control point in which the date may lie.
  *
  * @return             True, if the date lies inside the given control point; false otherwise.
  */
  MITKSEMANTICRELATIONS_EXPORT bool InsideControlPoint(const SemanticTypes::Date& date, const SemanticTypes::ControlPoint& controlPoint);
  /*
  * @brief Checks if a given control point lies inside another given control point.
  *        To lie inside another control point,
  *         - the start point of the first control point has to be greater or equal to the start point of the second control point
  *         AND
  *         - the end point of the first control point has to be smaller or equal to the end point of the second control point
  *
  * @par containedControlPoint    The control point that may be contained in the second control point.
  * @par containingControlPoint   The control point in which the first control point may lie.
  *
  * @return             True, if the first control point lies inside the second control point; false otherwise.
  */
  MITKSEMANTICRELATIONS_EXPORT bool InsideControlPoint(const SemanticTypes::ControlPoint& containedControlPoint, const SemanticTypes::ControlPoint& containingControlPoint);
  /*
  * @brief Checks if a given control point overlaps with a neighboring control point.
  *        To overlap,
  *         - the start point of the first control point has to be smaller than the end point of the second control point
  *         AND
  *         - the end point of the first control point has to be greater than the start point of the second control point
  *
  * @par controlPoint             The control point to be checked.
  * @par neighboringControlPoint  The neighboring control point to be checked.
  *
  * @return             True, if both the control points overlap with eatch other; false otherwise.
  */
  MITKSEMANTICRELATIONS_EXPORT bool CheckForOverlap(const SemanticTypes::ControlPoint& controlPoint, const SemanticTypes::ControlPoint& neighboringControlPoint);
  /*
  * @brief Extracts a date from the given data node using the 'DICOMHelper::GetDateFromDataNode'-function.
  *        If a real date can be extracted, the second 'FindFittingControlPoint'-function is used to find a control point that contains the given date.
  *        If no date can be extracted from the data node, the function prints an info and returns an empty control point.
  *
  * @par dataNode           A data node pointer, whose extracted date may be found in the given control points.
  * @par allControlPoints   A vector of control points that may contain the extracted date.
  *
  * @return                 The return value of the second 'FindFittingControlPoint'-function.
  *                         An empty control point, if no date can be extracted from the data node.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint FindFittingControlPoint(const mitk::DataNode* dataNode, std::vector<SemanticTypes::ControlPoint>& allControlPoints);
  /*
  * @brief Tries to find a control point in all the given control points that contains the given date.
  *        The correct control point may be found by using the 'InsideControlPoint' function with the date, evaluated on each control point.
  *
  * @par date               The date that may be found in the given control points.
  * @par allControlPoints   A vector of control points that may contain the given date.
  *
  * @return                 The control point that contains the given date. An empty control point, if the date is not contained in any of the given control points.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint FindFittingControlPoint(const SemanticTypes::Date& date, std::vector<SemanticTypes::ControlPoint>& allControlPoints);
  /*
  * @brief Extracts a date from the given data node using the 'DICOMHelper::GetDateFromDataNode'-function.
  *        If a real date can be extracted, the second 'ExtendClosestControlPoint'-function is used to try to extend a close control point.
  *        If no date can be extracted from the data node, the function prints an info and returns an empty control point.
  *
  * @par dataNode           A data node pointer, whose extracted date may be used to extend a close control points.
  * @par allControlPoints   A vector of control points of which the closest control point may be extended by the extracted date.
  *
  * @return                 The return value of the second 'ExtendClosestControlPoint'-function.
  *                         An empty control point, if no date can be extracted from the data node.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint ExtendClosestControlPoint(const mitk::DataNode* dataNode, std::vector<SemanticTypes::ControlPoint>& allControlPoints);
  /*
  * @brief Tries to find the closest control point to the given date in all the given control points. This is done by sorting the given vector of all control points and
  *        finding the first control point that is newer than the given date. Then the distances to the neighboring control points (older and newer, if existent) is calculated,
  *        using 'CalculateDistanceInDays'. If the smaller distance is smaller than a certain threshold, the corresponding control point is extended and returned, using the
  *        'ExtendControlPoint'-function. If not, an empty control point is returned.
  *
  * @return               The return value of the 'ExtendControlPoint'-function.
  *                       An empty control point, if the distance between the date and the closest control point exceeds a certain threshold.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint ExtendClosestControlPoint(const SemanticTypes::Date& date, std::vector<SemanticTypes::ControlPoint>& allControlPoints);
} // namespace mitk

#endif // MITKCONTROLPOINTMANAGER_H
