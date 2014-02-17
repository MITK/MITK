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

//us
#include <usGetModuleContext.h>



mitk::InteractionTestHelper::InteractionTestHelper()
{
  this->Initialize();
}

void mitk::InteractionTestHelper::Initialize()
{
  // Global interaction must(!) be initialized
  if(! mitk::GlobalInteraction::GetInstance()->IsInitialized())
    mitk::GlobalInteraction::GetInstance()->Initialize("global");

  mitk::RenderingManager* rm = mitk::RenderingManager::GetInstance();

  //########### setup axial renderwindow ##################
  //create renderWindow, renderer and dispatcher
  m_RenderWindowAxial = mitk::RenderWindow::New(NULL, "stdmulti.widget1", rm); //VtkRenderWindow is created within constructor if NULL
  //create data storage
  m_DataStorage = mitk::StandaloneDataStorage::New();
  //set storage of renderer
  m_RenderWindowAxial->GetRenderer()->SetDataStorage(m_DataStorage);

  //set view direction to axial
  m_RenderWindowAxial->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Axial );

  //set renderer to render 2D
  m_RenderWindowAxial->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);

  //########### setup sagittal renderwindow ##################
  //create renderWindow, renderer and dispatcher
  m_RenderWindowSagittal = mitk::RenderWindow::New(NULL, "stdmulti.widget2", rm); //VtkRenderWindow is created within constructor if NULL
  //create data storage
  m_DataStorage = mitk::StandaloneDataStorage::New();
  //set storage of renderer
  m_RenderWindowSagittal->GetRenderer()->SetDataStorage(m_DataStorage);

  //set view direction to axial
  m_RenderWindowSagittal->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Sagittal );

  //set renderer to render 2D
  m_RenderWindowSagittal->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);

  //########### setup frontal renderwindow ##################
  //create renderWindow, renderer and dispatcher
  m_RenderWindowFrontal = mitk::RenderWindow::New(NULL, "stdmulti.widget3", rm); //VtkRenderWindow is created within constructor if NULL
  //create data storage
  m_DataStorage = mitk::StandaloneDataStorage::New();
  //set storage of renderer
  m_RenderWindowFrontal->GetRenderer()->SetDataStorage(m_DataStorage);

  //set view direction to axial
  m_RenderWindowFrontal->GetSliceNavigationController()->SetDefaultViewDirection( mitk::SliceNavigationController::Frontal );

  //set renderer to render 2D
  m_RenderWindowFrontal->GetRenderer()->SetMapperID(mitk::BaseRenderer::Standard2D);

  //########### register display interactor to handle scroll events ##################
  //use MouseModeSwitcher to ensure that the statemachine of DisplayInteractor is loaded correctly
  m_MouseModeSwitcher = mitk::MouseModeSwitcher::New();


  //########### connect SliceNavigationControllers to timestep changed event of TimeNavigationController #############
  m_RenderWindowAxial->GetSliceNavigationController()->ConnectGeometryTimeEvent(mitk::RenderingManager::GetInstance()->GetTimeNavigationController(), false);
  m_RenderWindowSagittal->GetSliceNavigationController()->ConnectGeometryTimeEvent(mitk::RenderingManager::GetInstance()->GetTimeNavigationController(), false);
  m_RenderWindowFrontal->GetSliceNavigationController()->ConnectGeometryTimeEvent(mitk::RenderingManager::GetInstance()->GetTimeNavigationController(), false);
}

mitk::InteractionTestHelper::~InteractionTestHelper()
{
  mitk::BaseRenderer::RemoveInstance(m_RenderWindowAxial->GetVtkRenderWindow());
  mitk::BaseRenderer::RemoveInstance(m_RenderWindowSagittal->GetVtkRenderWindow());
  mitk::BaseRenderer::RemoveInstance(m_RenderWindowFrontal->GetVtkRenderWindow());
}


mitk::DataStorage::Pointer mitk::InteractionTestHelper::GetDataStorage()
{
  return m_DataStorage;
}




void mitk::InteractionTestHelper::AddNodeToStorage(mitk::DataNode::Pointer node)
{
  this->m_DataStorage->Add(node);
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
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


void mitk::InteractionTestHelper::SetTimeStep(int newTimeStep)
{
  bool timeStepIsvalid = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetCreatedWorldGeometry()->IsValidTimeStep(newTimeStep);

  if(timeStepIsvalid)
  {
    mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->SetPos(newTimeStep);
  }
}
