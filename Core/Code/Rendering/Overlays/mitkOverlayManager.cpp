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

#include "mitkOverlayManager.h"
#include "usGetModuleContext.h"

const std::string mitk::OverlayManager::PROP_ID = "org.mitk.services.OverlayManager.ID";

mitk::OverlayManager::OverlayManager()
{
  this->RegisterAsMicroservice();
}


mitk::OverlayManager::~OverlayManager()
{
}

void mitk::OverlayManager::AddOverlay(mitk::Overlay::Pointer overlay)
{
  m_OverlayList.push_back(overlay);
}

void mitk::OverlayManager::UpdateOverlays(mitk::BaseRenderer* baseRenderer)
{
  for(int i=0 ; i<m_OverlayList.size() ; i++)
  {
    mitk::Overlay::Pointer overlay = m_OverlayList[i];
//    overlay->Render(); //TODO
    overlay->UpdateOverlay(baseRenderer);
  }
}


//bool mitk::OverlayManager::LocalStorage::IsGenerateDataRequired(mitk::BaseRenderer *renderer, OverlayManager *overlaymanager)
//{
//  if( m_LastGenerateDataTime < overlaymanager -> GetMTime () )
//    return true;

//  if( renderer && m_LastGenerateDataTime < renderer -> GetTimeStepUpdateTime ( ) )
//    return true;

//  return false;
//}


mitk::OverlayManager::LocalStorage::~LocalStorage()
{
}

mitk::OverlayManager::LocalStorage::LocalStorage()
{

}


void mitk::OverlayManager::RegisterAsMicroservice()
{
  mitk::ServiceProperties properties;
  static int ID = 0;
  properties[mitk::OverlayManager::PROP_ID] = ID;
  mitk::ModuleContext* moduleContext = mitk::GetModuleContext();
  m_Registration = moduleContext->RegisterService<mitk::OverlayManager>(this,properties);
  ID++;
}

void mitk::OverlayManager::UnregisterAsMicroservice()
{
  if(m_Registration =! NULL)
  {
    m_Registration.Unregister();
  }
}
