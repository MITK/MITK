/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRTStructMapper2D.h"

mitk::RTStructMapper2D::LocalStorage::LocalStorage()
  : m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New())
{
}

mitk::RTStructMapper2D::LocalStorage::~LocalStorage()
{
}

vtkPropAssembly* mitk::RTStructMapper2D::LocalStorage::GetPropAssembly() const
{
  return m_PropAssembly;
}

void mitk::RTStructMapper2D::LocalStorage::SetPropAssembly(vtkPropAssembly* propAssembly)
{
  m_PropAssembly = propAssembly;
}

void mitk::RTStructMapper2D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool override)
{
  Superclass::SetDefaultProperties(node, renderer, override);
}

mitk::RTStructMapper2D::RTStructMapper2D()
{
}

mitk::RTStructMapper2D::~RTStructMapper2D()
{
}

void mitk::RTStructMapper2D::GenerateDataForRenderer(BaseRenderer* renderer)
{
}

vtkProp* mitk::RTStructMapper2D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->GetPropAssembly();
}
