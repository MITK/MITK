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
  * @brief Generates a control point from a given data node.
  *        The date is extracted from the data node by using the 'DICOMHelper::GetDICOMDateFromDataNode'-function.
  *        'GenerateControlPoint(const SemanticTypes::TimePoint&)' is used to generate a new control point from this extracted date.
  *
  * @par datanode   A data node pointer, whose date should be included in the newly generated control point.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint GenerateControlPoint(const mitk::DataNode* datanode);
  /*
  * @brief Returns a string that displays the given control point in the format "YYYY-MM-DD".
  *        This function is used in the GUI to display the control point as header in the "information-type - control-point"-matrix.
  *
  * @par controlPoint   The control point to convert into a string.
  */
  MITKSEMANTICRELATIONS_EXPORT std::string GetControlPointAsString(const SemanticTypes::ControlPoint& controlPoint);
  /**
  * @brief Returns an already existing control point from the given vector of control points. This existing control point has the
  *        the same date (year, month, day) as the given single control point.
  *        If no existing control point can be found an empty control point is returned.
  *
  * @par controlPoint       The control point to check for existence.
  * @par allControlPoints   The vector of already existing control points.
  */
  MITKSEMANTICRELATIONS_EXPORT SemanticTypes::ControlPoint FindExistingControlPoint(const SemanticTypes::ControlPoint& controlPoint, std::vector<SemanticTypes::ControlPoint>& allControlPoints);
} // namespace mitk

#endif // MITKCONTROLPOINTMANAGER_H
