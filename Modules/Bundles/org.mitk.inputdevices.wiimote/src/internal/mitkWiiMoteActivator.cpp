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
#include "mitkWiiMoteAddOn.h"

#include "mitkGlobalInteraction.h"

#include "mitkCoreExtConstants.h"

mitk::WiiMoteActivator::WiiMoteActivator() 
: m_IsRegistered(false)
, m_Controller(mitk::WiiMoteVtkCameraController::New())
{
 m_PrefService = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);  
}

mitk::WiiMoteActivator::~WiiMoteActivator()
{
}

bool mitk::WiiMoteActivator::RegisterInputDevice()
{
  if(!this->m_IsRegistered)
  {
    mitk::WiiMoteAddOn::GetInstance()->ActivateWiiMotes();
    // mitk::GlobalInteraction::GetInstance()->AddListener(m_Controller);
    mitk::EventMapper* eventMapper(mitk::GlobalInteraction::GetInstance()->GetEventMapper());
    if (eventMapper != NULL)
    {
      eventMapper->AddEventMapperAddOn(mitk::WiiMoteAddOn::GetInstance());
      this->m_IsRegistered = true;
    }
    else
    {
      MITK_ERROR << "Eventmapper is not initialized!";
      return false;
    }
  }

  // get the current preferences
  m_WiiMotePreferencesNode = 
    m_PrefService->GetSystemPreferences()->Node(CoreExtConstants::INPUTDEVICE_PREFERENCES);

  // modus change between Surface Interaction and VR Headtracking
  if(m_WiiMotePreferencesNode->GetBool(CoreExtConstants::WIIMOTE_SURFACEINTERACTION,false))
  {
    //if(mitk::GlobalInteraction::GetInstance()->ListenerRegistered(m_Controller))
    //{
      mitk::GlobalInteraction::GetInstance()->RemoveListener(m_Controller);
    //}

    // TODO: Add Surface Interactor
  }
  else
  {
    // TODO: check for Surface Interactor and remove
    // check if listener is registered
    mitk::GlobalInteraction::GetInstance()->AddListener(m_Controller);
  }

  return true;
}

bool mitk::WiiMoteActivator::UnRegisterInputDevice()
{
  if(this->m_IsRegistered)
  {
    mitk::WiiMoteAddOn::GetInstance()->DeactivateWiiMotes();

    //if(mitk::GlobalInteraction::GetInstance()->ListenerRegistered(m_Controller))
    //{
      mitk::GlobalInteraction::GetInstance()->RemoveListener(m_Controller);
    //}

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
  }

  return true;
}
