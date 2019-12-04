/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations module
#include "mitkLesionData.h"

mitk::LesionData::LesionData(const SemanticTypes::Lesion& lesion/* = SemanticTypes::Lesion()*/)
{
  m_Lesion = lesion;
}

void mitk::LesionData::SetLesion(const SemanticTypes::Lesion& lesion)
{
  m_Lesion = lesion;
}

void mitk::LesionData::SetLesionPresence(const std::vector<bool>& lesionPresence)
{
  m_LesionPresence = lesionPresence;
}

void mitk::LesionData::SetLesionVolume(const std::vector<double>& lesionVolume)
{
  m_LesionVolume = lesionVolume;
}
