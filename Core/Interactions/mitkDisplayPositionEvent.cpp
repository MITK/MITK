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


#include "mitkDisplayPositionEvent.h"
#include "mitkVector.h"

mitk::DisplayPositionEvent::DisplayPositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition)
: Event(sender, type, button, buttonState, key), m_DisplayPosition(displPosition)
{}

const mitk::Point2D& mitk::DisplayPositionEvent::GetDisplayPosition() const
{
	return m_DisplayPosition;
}


