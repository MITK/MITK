/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIMapper3D.h>

mitk::ROIMapper3D::LocalStorage::LocalStorage()
  : m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New())
{
}

mitk::ROIMapper3D::LocalStorage::~LocalStorage()
{
}

vtkPropAssembly* mitk::ROIMapper3D::LocalStorage::GetPropAssembly() const
{
  return m_PropAssembly;
}

void mitk::ROIMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override)
{
  Superclass::SetDefaultProperties(node, renderer, override);
}

mitk::ROIMapper3D::ROIMapper3D()
{
}

mitk::ROIMapper3D::~ROIMapper3D()
{
}

vtkProp* mitk::ROIMapper3D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->GetPropAssembly();
}
