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

// semantic relations module
#include "mitkLesionData.h"

mitk::LesionData::LesionData(const SemanticTypes::Lesion& lesion/* = SemanticTypes::Lesion()*/)
{
  m_Lesion = lesion;
}

mitk::LesionData::~LesionData()
{
  // nothing here
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
