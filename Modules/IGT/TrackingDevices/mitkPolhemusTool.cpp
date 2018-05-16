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

#include "mitkPolhemusTool.h"

mitk::PolhemusTool::PolhemusTool() : TrackingTool()
{
}

mitk::PolhemusTool::~PolhemusTool(void)
{
}

void mitk::PolhemusTool::SetToolPort(int _ToolPort)
{
  this->m_ToolPort = _ToolPort;
}

int mitk::PolhemusTool::GetToolPort()
{
  return this->m_ToolPort;
}
