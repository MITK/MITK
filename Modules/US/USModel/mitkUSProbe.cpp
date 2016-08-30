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

#include "mitkUSProbe.h"
#include <string>

mitk::USProbe::USProbe() : itk::Object()
{
}

mitk::USProbe::USProbe(std::string identifier)
  : m_Name(identifier)
{
}

mitk::USProbe::~USProbe()
{
}

bool mitk::USProbe::IsEqualToProbe(mitk::USProbe::Pointer probe)
{
  if (m_Name.compare(probe->GetName()) == 0) return true;
  else return false;
}

void mitk::USProbe::SetDepthAndSpacing(int depth, mitk::Vector3D spacing)
{
  m_DepthsAndSpacings.insert(std::pair<int, mitk::Vector3D>(depth, spacing));
}

std::map<int, mitk::Vector3D> mitk::USProbe::GetDepthsAndSpacing()
{
  return m_DepthsAndSpacings;
}

void mitk::USProbe::SetDepth(int depth)
{
  mitk::Vector3D defaultSpacing;
  defaultSpacing[0] = 1;
  defaultSpacing[1] = 1;
  defaultSpacing[2] = 0;

  m_DepthsAndSpacings.insert(std::pair<int, mitk::Vector3D>(depth, defaultSpacing));
}

void mitk::USProbe::RemoveDepth(int depthToRemove)
{
  m_DepthsAndSpacings.erase(depthToRemove);
}

void mitk::USProbe::SetSpacingForGivenDepth(int givenDepth, Vector3D spacing)
{
  m_DepthsAndSpacings[givenDepth][0] = spacing[0];
  m_DepthsAndSpacings[givenDepth][1] = spacing[1];
  m_DepthsAndSpacings[givenDepth][2] = spacing[2];
}

mitk::Vector3D mitk::USProbe::GetSpacingForGivenDepth(int givenDepth)
{
  mitk::Vector3D spacing;
  std::map<int, mitk::Vector3D>::iterator it = m_DepthsAndSpacings.find(givenDepth);
  if (it != m_DepthsAndSpacings.end()) //check if given depth really exists
  {
    spacing[0] = it->second[0];
    spacing[1] = it->second[1];
    spacing[2] = it->second[2];
  }
  else
  { //spacing does not exist, so set default spacing (1,1,0)
    spacing[0] = 1;
    spacing[1] = 1;
    spacing[2] = 0;
  }
  return spacing;
}
