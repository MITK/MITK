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
#include <mitkInteractionEventConst.h>

//us
#include <usGetModuleContext.h>

#include <tinyxml.h>



mitk::InteractionTestHelper::InteractionTestHelper(const std::string &interactionXmlFilePath)
  : m_InteractionFilePath(interactionXmlFilePath)
{
  this->Initialize(interactionXmlFilePath);
}

void mitk::InteractionTestHelper::Initialize(const std::string &interactionXmlFilePath)
{
  //TiXmlDocument document(interactionXmlPath.c_str());
  TiXmlDocument document(interactionXmlFilePath);
  bool loadOkay = document.LoadFile();
  if (loadOkay)
  {
    // Global interaction must(!) be initialized
    if(! mitk::GlobalInteraction::GetInstance()->IsInitialized())
      mitk::GlobalInteraction::GetInstance()->Initialize("global");

    //get RenderingManager instance
    mitk::RenderingManager* rm = mitk::RenderingManager::GetInstance();

    //create data storage
    m_DataStorage = mitk::StandaloneDataStorage::New();

    //for each renderer found create a render window and configure
    for( TiXmlElement* element = document.FirstChildElement(mitk::InteractionEventConst::xmlTagInteractions())->FirstChildElement(mitk::InteractionEventConst::xmlTagConfigRoot())->FirstChildElement(mitk::InteractionEventConst::xmlTagRenderer());
         element != NULL;
         element = element->NextSiblingElement(mitk::InteractionEventConst::xmlTagRenderer()) )
    {
      //get name of renderer
      const char* rendererName = element->Attribute(mitk::InteractionEventConst::xmlEventPropertyRendererName().c_str());

      //get view direction
      mitk::SliceNavigationController::ViewDirection viewDirection = mitk::SliceNavigationController::Axial;
      if(element->Attribute(mitk::InteractionEventConst::xmlEventPropertyViewDirection()) != NULL)
      {
        int viewDirectionNum = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlEventPropertyViewDirection())->c_str());
        viewDirection = static_cast<mitk::SliceNavigationController::ViewDirection>(viewDirectionNum);
      }

      //get mapper slot id
      mitk::BaseRenderer::MapperSlotId mapperID = mitk::BaseRenderer::Standard2D;
      if(element->Attribute(mitk::InteractionEventConst::xmlEventPropertyMapperID()) != NULL)
      {
        int mapperIDNum = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlEventPropertyMapperID())->c_str());
        mapperID = static_cast<mitk::BaseRenderer::MapperSlotId>(mapperIDNum);
      }


      // Get Size of Render Windows
      int size[3];
      size[0]=size[1]=size[2]=0;
      if(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeX()) != NULL)
      {
        size[0] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeX())->c_str());
      }
      if(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeY()) != NULL)
      {
        size[1] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeY())->c_str());
      }
      if(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeZ()) != NULL)
      {
        size[2] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeZ())->c_str());
      }

      //create renderWindow, renderer and dispatcher
      mitk::RenderWindow::Pointer rw = mitk::RenderWindow::New(NULL, rendererName, rm); //VtkRenderWindow is created within constructor if NULL

      if (size[0] != 0 && size[1] != 0)
        rw->SetSize(size[0],size[1]);

      //set storage of renderer
      rw->GetRenderer()->SetDataStorage(m_DataStorage);

      //set view direction to axial
      rw->GetSliceNavigationController()->SetDefaultViewDirection( viewDirection );

      //set renderer to render 2D
      rw->GetRenderer()->SetMapperID(mapperID);

      //connect SliceNavigationControllers to timestep changed event of TimeNavigationController
      rw->GetSliceNavigationController()->ConnectGeometryTimeEvent(rm->GetTimeNavigationController(), false);
      rm->GetTimeNavigationController()->ConnectGeometryTimeEvent(rw->GetSliceNavigationController(), false);

      //add to list of kown render windows
      m_RenderWindowList.push_back(rw);
    }

    //TODO: check the following lines taken from QmitkStdMultiWidget and adapt them to be executed in our code here.
//    mitkWidget1->GetSliceNavigationController()
//      ->ConnectGeometrySendEvent(mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));

    //########### register display interactor to handle scroll events ##################
    //use MouseModeSwitcher to ensure that the statemachine of DisplayInteractor is loaded correctly
    m_MouseModeSwitcher = mitk::MouseModeSwitcher::New();
  }
  else
  {
    mitkThrow() << "Can not load interaction xml file <" << m_InteractionFilePath << ">";
  }
}

mitk::InteractionTestHelper::~InteractionTestHelper()
{
    mitk::RenderingManager* rm = mitk::RenderingManager::GetInstance();

    //unregister renderers
    InteractionTestHelper::RenderWindowListType::iterator it = m_RenderWindowList.begin();
    InteractionTestHelper::RenderWindowListType::iterator end = m_RenderWindowList.end();

    for(; it != end; it++)
    {
        rm->GetTimeNavigationController()->Disconnect((*it)->GetSliceNavigationController());
        (*it)->GetSliceNavigationController()->Disconnect(rm->GetTimeNavigationController());
        mitk::BaseRenderer::RemoveInstance((*it)->GetVtkRenderWindow());
    }
    rm->RemoveAllObservers();
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
  //load events if not loaded yet
  if(m_Events.empty())
    this->LoadInteraction();

  //playback all events in queue
  for (unsigned long i=0; i < m_Events.size(); ++i)
  {
    //let dispatcher of sending renderer process the event
    m_Events.at(i)->GetSender()->GetDispatcher()->ProcessEvent(m_Events.at(i));
  }
}


void mitk::InteractionTestHelper::LoadInteraction()
{
  //load interaction pattern from xml file
  std::ifstream xmlStream(m_InteractionFilePath.c_str());
  mitk::XML2EventParser parser(xmlStream);
  m_Events = parser.GetInteractions();
  xmlStream.close();
    // Avoid VTK warning: Trying to delete object with non-zero reference count.
  parser.SetReferenceCount(0);
}


void mitk::InteractionTestHelper::SetTimeStep(int newTimeStep)
{
  bool timeStepIsvalid = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetCreatedWorldGeometry()->IsValidTimeStep(newTimeStep);

  if(timeStepIsvalid)
  {
    mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->SetPos(newTimeStep);
  }
}


mitk::RenderWindow* mitk::InteractionTestHelper::GetRenderWindowByName(const std::string &name)
{
  InteractionTestHelper::RenderWindowListType::iterator it = m_RenderWindowList.begin();
  InteractionTestHelper::RenderWindowListType::iterator end = m_RenderWindowList.end();

  for(; it != end; it++)
  {
    if( name.compare( (*it)->GetRenderer()->GetName() ) == 0)
      return (*it).GetPointer();
  }

  return NULL;
}


mitk::RenderWindow* mitk::InteractionTestHelper::GetRenderWindowByDefaultViewDirection(mitk::SliceNavigationController::ViewDirection viewDirection)
{
  InteractionTestHelper::RenderWindowListType::iterator it = m_RenderWindowList.begin();
  InteractionTestHelper::RenderWindowListType::iterator end = m_RenderWindowList.end();

  for(; it != end; it++)
  {
    if( viewDirection == (*it)->GetSliceNavigationController()->GetDefaultViewDirection() )
      return (*it).GetPointer();
  }

  return NULL;
}


mitk::RenderWindow* mitk::InteractionTestHelper::GetRenderWindow(unsigned int index)
{
  if( index < m_RenderWindowList.size() )
  {
    return m_RenderWindowList.at(index).GetPointer();
  }
  else
  {
    return NULL;
  }
}
