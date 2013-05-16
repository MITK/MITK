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

#include "mitkOverlay.h"

mitk::Overlay::Overlay()
{
}


mitk::Overlay::~Overlay()
{
}

bool mitk::Overlay::BaseLocalStorage::IsGenerateDataRequired(
    mitk::BaseRenderer *renderer,
    mitk::Overlay *overlay)
{
  if( m_LastGenerateDataTime < overlay -> GetMTime () )
    return true;

  if( renderer && m_LastGenerateDataTime < renderer -> GetTimeStepUpdateTime ( ) )
    return true;

  return false;
}
