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

#include "mitkToolManagerProvider.h"



mitk::ToolManagerProvider::ToolManagerProvider()
{
  this->m_ToolManager = mitk::ToolManager::New(NULL);
}

mitk::ToolManagerProvider::~ToolManagerProvider()
{
  this->m_ToolManager = NULL;
}

mitk::ToolManager* mitk::ToolManagerProvider::GetToolManager()
{
  return this->m_ToolManager;
}

mitk::ToolManagerProvider* mitk::ToolManagerProvider::GetInstance()
{
  return NULL;
}
