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


//MITK
#include <mitkInteractionTestHelper.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkGlobalInteraction.h>
#include <mitkIOUtil.h>



mitk::InteractionTestHelper::InteractionTestHelper()
{
  this->Initialize();
}

void mitk::InteractionTestHelper::Initialize()
{
  // Global interaction must(!) be initialized
  mitk::GlobalInteraction::GetInstance()->Initialize("global");

  //create renderWindow, renderer and dispatcher
  m_RenderWindow = mitk::RenderWindow::New();
  //create data storage
  m_DataStorage = mitk::StandaloneDataStorage::New();
  //set storage of renderer
  m_RenderWindow->GetRenderer()->SetDataStorage(m_DataStorage);
}

mitk::InteractionTestHelper::~InteractionTestHelper()
{
}


mitk::DataStorage::Pointer mitk::InteractionTestHelper::GetDataStorage()
{
  return m_DataStorage;
}

mitk::BaseRenderer* mitk::InteractionTestHelper::GetRenderer()
{
  return m_Renderer;
}


void mitk::InteractionTestHelper::AddNodeToStorage(mitk::DataNode::Pointer node)
{
  this->m_DataStorage->Add(node);
  mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
}


void mitk::InteractionTestHelper::PlaybackInteraction()
{
  for (unsigned long i=0; i < m_Events.size(); ++i)
  {
    m_Events.at(i)->GetSender()->GetDispatcher()->ProcessEvent(m_Events.at(i));
  }
}


void mitk::InteractionTestHelper::LoadInteraction(std::string interactionXmlPath)
{
  //load interaction pattern
  std::ifstream xmlStream(interactionXmlPath.c_str());
  mitk::XML2EventParser parser(xmlStream);
  m_Events = parser.GetInteractions();
  xmlStream.close();
}
