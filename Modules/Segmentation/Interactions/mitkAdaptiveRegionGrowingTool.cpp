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

#include "mitkAdaptiveRegionGrowing.h"

#include "mitkToolManager.h"


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, AdaptiveRegionGrowing, "AdaptiveRegionGrowing");
}

mitk::AdaptiveRegionGrowing::AdaptiveRegionGrowing()
{
}

mitk::AdaptiveRegionGrowing::~AdaptiveRegionGrowing()
{
}

const char** mitk::AdaptiveRegionGrowing::GetXPM() const
{
  return NULL;
}

const char* mitk::AdaptiveRegionGrowing::GetName() const
{
  return "RegionGrowing";
}

std::string mitk::AdaptiveRegionGrowing::GetIconPath() const
{
  return ":/Segmentation/Add_48x48.png";
}

void mitk::AdaptiveRegionGrowing::Activated()
{

}

void mitk::AdaptiveRegionGrowing::Deactivated()
{
}
