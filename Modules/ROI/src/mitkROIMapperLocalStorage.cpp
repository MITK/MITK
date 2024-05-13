/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIMapperLocalStorage.h>

#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>

mitk::ROIMapperLocalStorage::ROIMapperLocalStorage()
  : m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New()),
    m_LastTimePoint(0.0)
{
}

mitk::ROIMapperLocalStorage::~ROIMapperLocalStorage()
{
}

vtkPropAssembly* mitk::ROIMapperLocalStorage::GetPropAssembly() const
{
  return m_PropAssembly;
}

void mitk::ROIMapperLocalStorage::SetPropAssembly(vtkPropAssembly* propAssembly)
{
  m_PropAssembly = propAssembly;
}

mitk::TimePointType mitk::ROIMapperLocalStorage::GetLastTimePoint() const
{
  return m_LastTimePoint;
}

void mitk::ROIMapperLocalStorage::SetLastTimePoint(TimePointType timePoint)
{
  m_LastTimePoint = timePoint;
}
