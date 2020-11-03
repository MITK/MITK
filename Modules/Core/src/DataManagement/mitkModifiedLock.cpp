/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkModifiedLock.h>

mitk::ModifiedLock::ModifiedLock(BaseGeometry *baseGeo)
{
  m_baseGeometry = baseGeo;
  m_baseGeometry->m_ModifiedLockFlag = true;
  m_baseGeometry->m_ModifiedCalledFlag = false;
}

mitk::ModifiedLock::~ModifiedLock()
{
  m_baseGeometry->m_ModifiedLockFlag = false;

  if (m_baseGeometry->m_ModifiedCalledFlag)
    m_baseGeometry->Modified();

  m_baseGeometry->m_ModifiedCalledFlag = false;
}
