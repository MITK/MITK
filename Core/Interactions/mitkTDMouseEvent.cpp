/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkTDMouseEvent.h"
#include "mitkInteractionConst.h"
#include "mitkVector.h"

mitk::TDMouseEvent::TDMouseEvent(int buttonState, const Vector3D& translation, const Vector3D& rotation, const ScalarType& angle)
: Event(NULL, Type_TDMouseInput, BS_NoButton, buttonState, Key_none), m_Translation(translation), m_Rotation(rotation), m_Angle(angle)
{
}
