/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIMapper2D.h>

mitk::ROIMapper2D::LocalStorage::LocalStorage()
  : m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New())
{
}

mitk::ROIMapper2D::LocalStorage::~LocalStorage()
{
}

vtkPropAssembly* mitk::ROIMapper2D::LocalStorage::GetPropAssembly() const
{
  return m_PropAssembly;
}

void mitk::ROIMapper2D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override)
{
  Superclass::SetDefaultProperties(node, renderer, override);
}

mitk::ROIMapper2D::ROIMapper2D()
{
}

mitk::ROIMapper2D::~ROIMapper2D()
{
}

vtkProp* mitk::ROIMapper2D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->GetPropAssembly();
}
