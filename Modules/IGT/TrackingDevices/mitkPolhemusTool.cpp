/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
