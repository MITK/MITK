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

// semantic relations UI
#include "mitkLesionItemData.h"

mitk::LesionItemData::LesionItemData()
{
  // nothing here
}


mitk::LesionItemData::~LesionItemData()
{
  // nothing here
}

void mitk::LesionItemData::SetLesion(const mitk::SemanticTypes::Lesion& lesion)
{
  m_Lesion = lesion;
}
