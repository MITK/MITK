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

#include <mitkModifiedLock.h>

mitk::ModifiedLock::ModifiedLock(BaseGeometry* baseGeo){
  m_baseGeometry = baseGeo;
  m_baseGeometry->m_ModifiedLockFlag = true;
  m_baseGeometry->m_ModifiedCalledFlag = false;
}

mitk::ModifiedLock::~ModifiedLock(){
  m_baseGeometry->m_ModifiedLockFlag = false;

  if(m_baseGeometry->m_ModifiedCalledFlag)
    m_baseGeometry->Modified();

  m_baseGeometry->m_ModifiedCalledFlag = false;
}
