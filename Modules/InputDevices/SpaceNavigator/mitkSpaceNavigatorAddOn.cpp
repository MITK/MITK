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

#include <mitkGlobalInteraction.h>
#include <mitkInteractionConst.h>
#include <mitkNumericTypes.h>

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





