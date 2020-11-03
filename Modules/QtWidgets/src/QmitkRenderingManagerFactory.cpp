/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderingManagerFactory.h"
#include "QmitkRenderingManager.h"

QmitkRenderingManagerFactory::QmitkRenderingManagerFactory()
{
  mitk::RenderingManager::SetFactory(this);
}

QmitkRenderingManagerFactory::~QmitkRenderingManagerFactory()
{
}

mitk::RenderingManager::Pointer QmitkRenderingManagerFactory::CreateRenderingManager() const
{
  QmitkRenderingManager::Pointer specificSmartPtr = QmitkRenderingManager::New();
  mitk::RenderingManager::Pointer smartPtr = specificSmartPtr.GetPointer();
  return smartPtr;
}
