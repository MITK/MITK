/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSpaceNavigatorEvent.h"
#include "mitkInteractionConst.h"
#include "mitkVector.h"

mitk::SpaceNavigatorEvent::SpaceNavigatorEvent(int buttonState, const Vector3D& translation, const Vector3D& rotation, const ScalarType& angle)
: Event(NULL, mitk::Type_SpaceNavigatorInput, BS_NoButton, buttonState, Key_none), m_Translation(translation), m_Rotation(rotation), m_Angle(angle)
{
}
