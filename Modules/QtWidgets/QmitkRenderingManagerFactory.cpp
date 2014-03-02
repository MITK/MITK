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


#include "QmitkRenderingManagerFactory.h"
#include "QmitkRenderingManager.h"

QmitkRenderingManagerFactory
::QmitkRenderingManagerFactory()
{
  mitk::RenderingManager::SetFactory( this );

}

QmitkRenderingManagerFactory
::~QmitkRenderingManagerFactory()
{
}

mitk::RenderingManager::Pointer
QmitkRenderingManagerFactory
::CreateRenderingManager() const
{
  QmitkRenderingManager::Pointer specificSmartPtr = QmitkRenderingManager::New();
  mitk::RenderingManager::Pointer smartPtr = specificSmartPtr.GetPointer();
  return smartPtr;
}
