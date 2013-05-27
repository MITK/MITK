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

mitk::OverlayManager::OverlayManager()
{
}


mitk::OverlayManager::~OverlayManager()
{
}

void mitk::OverlayManager::AddOverlay(mitk::Overlay::Pointer overlay)
{
  m_OverlayList.push_back(overlay);
}

void mitk::OverlayManager::PrepareOverlays(mitk::BaseRenderer* baseRenderer)
{
  for(int i=0 ; i<m_OverlayList.size() ; i++)
  {
    mitk::Overlay::Pointer overlay = m_OverlayList[i];
//    overlay->Render(); //TODO
    overlay->MitkRender(baseRenderer);
  }
}


bool mitk::OverlayManager::LocalStorage::IsGenerateDataRequired(mitk::BaseRenderer *renderer, OverlayManager *overlaymanager)
{
  if( m_LastGenerateDataTime < overlaymanager -> GetMTime () )
    return true;

  if( renderer && m_LastGenerateDataTime < renderer -> GetTimeStepUpdateTime ( ) )
    return true;

  return false;
}


mitk::OverlayManager::LocalStorage::~LocalStorage()
{
}

mitk::OverlayManager::LocalStorage::LocalStorage()
{

}
