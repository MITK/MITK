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

#include "mitkChiliPluginImpl.h"
#include <mitkPropertyList.h>

QcPlugin* mitk::ChiliPluginImpl::s_PluginInstance = 0;

mitk::ChiliPluginImpl::ChiliPluginImpl()
{
  m_CurrentStudy.InstanceUID == "";
}

mitk::ChiliPluginImpl::~ChiliPluginImpl()
{
}

bool mitk::ChiliPluginImpl::IsPlugin()
{
  return true;
}

mitk::ChiliPlugin::StudyInformation mitk::ChiliPluginImpl::GetCurrentStudy()
{
  return m_CurrentStudy;
}

mitk::ChiliPlugin::SeriesList mitk::ChiliPluginImpl::GetCurrentSeries()
{
  return m_CurrentSeries;
}

unsigned int mitk::ChiliPluginImpl::GetLightBoxCount()
{
  return m_LightBoxCount;
}

int mitk::ChiliPluginImpl::GetConferenceID()
{
  return m_QmitkChiliPluginConferenceID;
}

QcPlugin* mitk::ChiliPluginImpl::GetPluginInstance()
{
  return s_PluginInstance;
}

void mitk::ChiliPluginImpl::SetPluginInstance( QcPlugin* instance )
{
  if( s_PluginInstance == 0 )
    s_PluginInstance = instance;
}

void mitk::ChiliPluginImpl::SendStudySelectedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginStudySelected() );
}

void mitk::ChiliPluginImpl::SendLightBoxCountChangedEvent()
{
  //throw ITK event (defined in mitkChiliPluginEvents.h)
  InvokeEvent( PluginLightBoxCountChanged() );
}

void mitk::ChiliPluginImpl::SetPropertyToNode( const mitk::PropertyList::Pointer property, mitk::DataTreeNode* dst )
{
  for( mitk::PropertyList::PropertyMap::const_iterator iter = property->GetMap()->begin(); iter != property->GetMap()->end(); iter++ )
  {
    dst->SetProperty( iter->first.c_str(), iter->second.first );
  }
}
