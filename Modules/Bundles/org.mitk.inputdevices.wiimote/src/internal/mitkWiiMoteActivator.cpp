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

#include "mitkWiiMoteActivator.h"

//#include "mitkWiiMoteAddOn.h"

#include "mitkGlobalInteraction.h"

mitk::WiiMoteActivator::WiiMoteActivator() 
: m_IsRegistered(false)
//, m_Controller(mitk::WiiMoteVtkCameraController::New())
{
}

mitk::WiiMoteActivator::~WiiMoteActivator()
{
}

bool mitk::WiiMoteActivator::RegisterInputDevice()
{
  //if(!this->m_IsRegistered)
  //{
  //  mitk::WiiMoteAddOn::GetInstance()->ActivateWiiMotes();
  //  mitk::GlobalInteraction::GetInstance()->AddListener(m_Controller);
  //  mitk::EventMapper* eventMapper(mitk::GlobalInteraction::GetInstance()->GetEventMapper());
  //  if (eventMapper != NULL)
  //  {
  //    eventMapper->AddEventMapperAddOn(mitk::WiiMoteAddOn::GetInstance());
  //    this->m_IsRegistered = true;
  //  }
  //  else
  //  {
  //    MITK_ERROR << "Eventmapper is not initialized!";
  //    return false;
  //  }
  //}
  //return true;
}

bool mitk::WiiMoteActivator::UnRegisterInputDevice()
{
 /* if(this->m_IsRegistered)
  {
    mitk::WiiMoteAddOn::GetInstance()->DeactivateWiiMotes();
    mitk::GlobalInteraction::GetInstance()->RemoveListener(m_Controller);
    mitk::EventMapper* eventMapper(mitk::GlobalInteraction::GetInstance()->GetEventMapper());

    if(eventMapper != NULL)
    {
      eventMapper->RemoveEventMapperAddOn(mitk::WiiMoteAddOn::GetInstance());
      this->m_IsRegistered = false;
    }
    else
    {
      MITK_ERROR << "Eventmapper is not initialized!";
      return false;
    }
  }*/

  return true;
}
