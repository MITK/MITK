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

#include <mitkGlobalInteraction.h>
#include <mitkInteractionConst.h>
#include <mitkVector.h>

#include <mitkSpaceNavigatorAddOn.h>
#include <mitkSpaceNavigatorDriver.h>
#include <mitkSpaceNavigatorEvent.h>


mitk::SpaceNavigatorAddOn* mitk::SpaceNavigatorAddOn::GetInstance()
{
  // only needed for initializiation
  SpaceNavigatorDriver* spaceNavigatorDriver = SpaceNavigatorDriver::GetInstance();

  static SpaceNavigatorAddOn instance;
  return &instance;
}


void mitk::SpaceNavigatorAddOn::DeviceChange (long device, long keys, long programmableKeys)
{
}

void mitk::SpaceNavigatorAddOn::KeyDown (int keyCode)
{
  mitk::Event* e = new mitk::Event(NULL, mitk::Type_SpaceNavigatorKeyDown, mitk::BS_LeftButton, keyCode, mitk::Key_none);
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDSPACENAVIGATORKEYDOWN, e);
  this->ForwardEvent(se);
}

void mitk::SpaceNavigatorAddOn::KeyUp (int keyCode)
{
}

void mitk::SpaceNavigatorAddOn::SensorInput( mitk::Vector3D translation, mitk::Vector3D rotation, mitk::ScalarType angle)
{
  mitk::SpaceNavigatorEvent* e = new mitk::SpaceNavigatorEvent(mitk::BS_NoButton, translation, rotation, angle);
  mitk::StateEvent* se = new mitk::StateEvent(mitk::EIDSPACENAVIGATORINPUT, e);
  this->ForwardEvent(se);
}





