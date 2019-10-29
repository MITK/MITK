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

mitk::PolhemusTool::PolhemusTool()
  : TrackingTool(),
    m_ToolPort(-1),
    m_DistortionLevel(DistortionLevel::UNDEFINED)
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

void mitk::PolhemusTool::SetDistortionLevel(const int level)
{
  if (level == 0)
  {
    m_DistortionLevel = DistortionLevel::NO_DISTORTION;
  }
  else if (level == 1)
  {
    m_DistortionLevel = DistortionLevel::MINOR_DISTORTION;
  }
  else if (level == 2)
  {
    m_DistortionLevel = DistortionLevel::SIGNIFICANT_DISTORTION;
  }
  else
  {
    m_DistortionLevel = DistortionLevel::UNDEFINED;
  }
}

mitk::PolhemusTool::DistortionLevel mitk::PolhemusTool::GetDistortionLevel() const
{
  return m_DistortionLevel;
}
