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

#include "mitkNodePredicateGeometry.h"
#include "mitkDataNode.h"
#include "mitkImage.h"

mitk::NodePredicateGeometry::NodePredicateGeometry(const BaseGeometry* refGeometry, TimePointType relevantTimePoint)
  : m_RefGeometry(refGeometry), m_RefTimeGeometry(nullptr), m_TimePoint(relevantTimePoint), m_UseTimePoint(true), m_CheckPrecision(mitk::eps)
{
  if (m_RefGeometry.IsNull()) mitkThrow() << "Invalid constructor initialization. Reference base geometry instance is nullptr pointer.";
}

mitk::NodePredicateGeometry::NodePredicateGeometry(const BaseGeometry* refGeometry)
  : m_RefGeometry(refGeometry), m_RefTimeGeometry(nullptr), m_TimePoint(0), m_UseTimePoint(false), m_CheckPrecision(mitk::eps)
{
  if (m_RefGeometry.IsNull()) mitkThrow() << "Invalid constructor initialization. Reference base geometry instance is nullptr pointer.";
}

mitk::NodePredicateGeometry::NodePredicateGeometry(const TimeGeometry* refGeometry)
  : m_RefGeometry(nullptr), m_RefTimeGeometry(refGeometry), m_TimePoint(0), m_UseTimePoint(false), m_CheckPrecision(mitk::eps)
{
  if (m_RefTimeGeometry.IsNull()) mitkThrow() << "Invalid constructor initialization. Reference base geometry instance is nullptr pointer.";
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
        return Equal(*testGeometry, *m_RefGeometry, this->m_CheckPrecision, false);
      }
    }
    else
    { //check whole time geometry
      return Equal(*data->GetTimeGeometry(), *m_RefTimeGeometry, this->m_CheckPrecision, false);
    }
  }

  return false;
}
