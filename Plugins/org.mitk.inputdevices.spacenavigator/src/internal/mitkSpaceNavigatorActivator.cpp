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

#include "mitkSpaceNavigatorActivator.h"

#include <mitkGlobalinteraction.h>
#include <mitkEventMapper.h>

#include <mitkSpaceNavigatorAddOn.h>

mitk::SpaceNavigatorActivator::SpaceNavigatorActivator()
: m_IsRegistered(false), m_Controller(mitk::SpaceNavigatorVtkCameraController::New())
{
}

bool mitk::SpaceNavigatorActivator::RegisterInputDevice()
{
  if(!this->m_IsRegistered)
  {
    mitk::GlobalInteraction::GetInstance()->AddListener(m_Controller);
    mitk::EventMapper* eventMapper(mitk::GlobalInteraction::GetInstance()->GetEventMapper());
    if (eventMapper != NULL)
    {
      eventMapper->AddEventMapperAddOn(mitk::SpaceNavigatorAddOn::GetInstance());
      this->m_IsRegistered = true;
    }
    else
    {
      return false;
    }
  }
  return true;
}

bool mitk::SpaceNavigatorActivator::UnRegisterInputDevice()
{
  if(this->m_IsRegistered)
  {
    mitk::GlobalInteraction::GetInstance()->RemoveListener(m_Controller);
    mitk::EventMapper* eventMapper(mitk::GlobalInteraction::GetInstance()->GetEventMapper());

    if(eventMapper != NULL)
    {
      eventMapper->RemoveEventMapperAddOn(mitk::SpaceNavigatorAddOn::GetInstance());
      this->m_IsRegistered = false;
    }
    else
    {
      return false;
    }
  }
  return true;
}
