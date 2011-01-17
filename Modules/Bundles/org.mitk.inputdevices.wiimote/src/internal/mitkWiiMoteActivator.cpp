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

//mitk
#include "mitkGlobalInteraction.h"
#include "mitkCoreExtConstants.h"
#include <mitkDataNodeFactory.h>
#include <mitkBaseRenderer.h>
#include <mitkIDataStorageService.h>

mitk::WiiMoteActivator::WiiMoteActivator() 
: m_IsRegistered(false)
, m_Controller(mitk::WiiMoteVtkCameraController::New())
, m_Interactor(NULL)
, m_Node(NULL)
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
    if(mitk::GlobalInteraction::GetInstance()->ListenerRegistered(m_Controller))
    {
      mitk::GlobalInteraction::GetInstance()->RemoveListener(m_Controller);
    }

    this->AddSurfaceInteractor();
  }
  else
  {
    // Headtracking listener already registered
    if(!(mitk::GlobalInteraction::GetInstance()->ListenerRegistered(m_Controller)))
    {
      mitk::GlobalInteraction::GetInstance()->AddListener(m_Controller);
    }

    this->RemoveSurfaceInteractor();
  }

  return true;
}

bool mitk::WiiMoteActivator::UnRegisterInputDevice()
{
  if(this->m_IsRegistered)
  {
    mitk::WiiMoteAddOn::GetInstance()->DeactivateWiiMotes();

    // remove Headtracking listener
    if(mitk::GlobalInteraction::GetInstance()->ListenerRegistered(m_Controller))
    {
      mitk::GlobalInteraction::GetInstance()->RemoveListener(m_Controller);
    }

    this->RemoveSurfaceInteractor();

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

mitk::DataStorage::Pointer mitk::WiiMoteActivator::GetDataStorage()
{
  mitk::IDataStorageService::Pointer service = 
    berry::Platform::GetServiceRegistry().
    GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

  if (service.IsNotNull())
  {
    return service->GetActiveDataStorage()->GetDataStorage();
  }

  return 0;
}

void mitk::WiiMoteActivator::AddSurfaceInteractor()
{
    mitk::DataNodeFactory::Pointer nodeReader = mitk::DataNodeFactory::New();

    // model was designed by Patrick Grubb
    std::string fileName = MITK_ROOT;
    fileName += "Modules/InputDevices/WiiMote/WiiMoteModel.obj"; 

    try
    {
      nodeReader->SetFileName(fileName.c_str());
      nodeReader->Update();
      m_Node = nodeReader->GetOutput();

      if(m_Interactor.IsNull())
      {
        m_Interactor = mitk::WiiMoteInteractor::New("WiiMoteSurfaceInteraction",m_Node);
      }

      m_Node->SetInteractor(m_Interactor);

      this->GetDataStorage()->Add(m_Node);

      if(!(mitk::GlobalInteraction::GetInstance()->InteractorRegistered(m_Interactor)))
      {
        mitk::GlobalInteraction::GetInstance()->AddInteractor(m_Interactor);
      }

      mitk::WiiMoteAddOn::GetInstance()->SetWiiMoteSurfaceIModus(true);
    }
    catch(...)
    {
      MITK_ERROR << "Wiimote Surface Interaction could not be initialized";
    }
}

void mitk::WiiMoteActivator::RemoveSurfaceInteractor()
{
  try
  {
    if(m_Node.IsNotNull())
    {
      this->GetDataStorage()->Remove(m_Node);
    }

    if(m_Interactor.IsNotNull() 
      && mitk::GlobalInteraction::GetInstance()->InteractorRegistered(m_Interactor))
    {
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_Interactor);
    }

    mitk::WiiMoteAddOn::GetInstance()->SetWiiMoteSurfaceIModus(false);
  }
  catch(...)
  {
    MITK_ERROR << "Wiimote Surface Interactor could not be properly removed";
  }
}