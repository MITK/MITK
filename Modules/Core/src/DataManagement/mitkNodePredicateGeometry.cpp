/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNodePredicateGeometry.h"
#include "mitkDataNode.h"
#include "mitkImage.h"

mitk::NodePredicateGeometry::NodePredicateGeometry(const BaseGeometry* refGeometry, TimePointType relevantTimePoint)
  : m_RefGeometry(refGeometry), m_RefTimeGeometry(nullptr), m_TimePoint(relevantTimePoint), m_UseTimePoint(true), m_CheckCoordinatePrecision(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION), m_CheckDirectionPrecision(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION)
{
  if (m_RefGeometry.IsNull()) mitkThrow() << "Invalid constructor initialization. Reference base geometry instance is nullptr pointer.";
}

mitk::NodePredicateGeometry::NodePredicateGeometry(const BaseGeometry* refGeometry)
  : m_RefGeometry(refGeometry), m_RefTimeGeometry(nullptr), m_TimePoint(0), m_UseTimePoint(false), m_CheckCoordinatePrecision(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION), m_CheckDirectionPrecision(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION)
{
  if (m_RefGeometry.IsNull()) mitkThrow() << "Invalid constructor initialization. Reference base geometry instance is nullptr pointer.";
}

mitk::NodePredicateGeometry::NodePredicateGeometry(const TimeGeometry* refGeometry)
  : m_RefGeometry(nullptr), m_RefTimeGeometry(refGeometry), m_TimePoint(0), m_UseTimePoint(false), m_CheckCoordinatePrecision(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION), m_CheckDirectionPrecision(NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_DIRECTION_PRECISION)
{
  if (m_RefTimeGeometry.IsNull()) mitkThrow() << "Invalid constructor initialization. Reference base geometry instance is nullptr pointer.";
}

void mitk::NodePredicateGeometry::SetCheckPrecision(ScalarType precision)
{
  SetCheckCoordinatePrecision(precision);
  SetCheckDirectionPrecision(precision);
}

mitk::NodePredicateGeometry::~NodePredicateGeometry()
{
}

bool mitk::NodePredicateGeometry::CheckNode(const mitk::DataNode *node) const
{
  if (node == nullptr)
    mitkThrow() << "NodePredicateGeometry: invalid node";

  mitk::BaseData *data = node->GetData();
  if (data)
  {
    if (m_RefGeometry.IsNotNull())
    { //check only one time point.
      mitk::BaseGeometry::Pointer testGeometry = data->GetGeometry();
      if (this->m_UseTimePoint)
      {
        testGeometry = data->GetTimeGeometry()->GetGeometryForTimePoint(m_TimePoint);
      }

      if (testGeometry)
      {
        return Equal(*testGeometry, *m_RefGeometry, this->m_CheckCoordinatePrecision, this->m_CheckDirectionPrecision, false);
      }
    }
    else
    { //check whole time geometry
      return Equal(*data->GetTimeGeometry(), *m_RefTimeGeometry, this->m_CheckCoordinatePrecision, this->m_CheckDirectionPrecision, false);
    }
  }

  return false;
}
