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


#include "mitkTDMouseEvent.h"
#include "mitkInteractionConst.h"
#include "mitkVector.h"

mitk::TDMouseEvent::TDMouseEvent(int buttonState, const Vector3D& translation, const Vector3D& rotation, const ScalarType& angle)
: Event(NULL, Type_TDMouseInput, BS_NoButton, buttonState, Key_none), m_Translation(translation), m_Rotation(rotation), m_Angle(angle)
{
}
