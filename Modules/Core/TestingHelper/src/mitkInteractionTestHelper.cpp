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

// MITK
#include <mitkIOUtil.h>
#include <mitkInteractionEventConst.h>
#include <mitkInteractionTestHelper.h>
#include <mitkPlaneGeometryDataMapper2D.h>
#include <mitkStandaloneDataStorage.h>

// VTK
#include <vtkCamera.h>
#include <vtkRenderWindowInteractor.h>

// us
#include <usGetModuleContext.h>

#include <tinyxml.h>

mitk::InteractionTestHelper::InteractionTestHelper(const std::string &interactionXmlFilePath)
  : m_InteractionFilePath(interactionXmlFilePath)
{
  this->Initialize(interactionXmlFilePath);
}

void mitk::InteractionTestHelper::Initialize(const std::string &interactionXmlFilePath)
{
  // TiXmlDocument document(interactionXmlPath.c_str());
  TiXmlDocument document(interactionXmlFilePath);
  bool loadOkay = document.LoadFile();
  if (loadOkay)
  {
    // get RenderingManager instance
    mitk::RenderingManager *rm = mitk::RenderingManager::GetInstance();

    // create data storage
    m_DataStorage = mitk::StandaloneDataStorage::New();

    // for each renderer found create a render window and configure
    for (TiXmlElement *element = document.FirstChildElement(mitk::InteractionEventConst::xmlTagInteractions())
                                   ->FirstChildElement(mitk::InteractionEventConst::xmlTagConfigRoot())
                                   ->FirstChildElement(mitk::InteractionEventConst::xmlTagRenderer());
         element != nullptr;
         element = element->NextSiblingElement(mitk::InteractionEventConst::xmlTagRenderer()))
    {
      // get name of renderer
      const char *rendererName =
        element->Attribute(mitk::InteractionEventConst::xmlEventPropertyRendererName().c_str());

      // get view direction
      mitk::SliceNavigationController::ViewDirection viewDirection = mitk::SliceNavigationController::Axial;
      if (element->Attribute(mitk::InteractionEventConst::xmlEventPropertyViewDirection()) != nullptr)
      {
        int viewDirectionNum =
          std::atoi(element->Attribute(mitk::InteractionEventConst::xmlEventPropertyViewDirection())->c_str());
        viewDirection = static_cast<mitk::SliceNavigationController::ViewDirection>(viewDirectionNum);
      }

      // get mapper slot id
      mitk::BaseRenderer::MapperSlotId mapperID = mitk::BaseRenderer::Standard2D;
      if (element->Attribute(mitk::InteractionEventConst::xmlEventPropertyMapperID()) != nullptr)
      {
        int mapperIDNum =
          std::atoi(element->Attribute(mitk::InteractionEventConst::xmlEventPropertyMapperID())->c_str());
        mapperID = static_cast<mitk::BaseRenderer::MapperSlotId>(mapperIDNum);
      }

      // Get Size of Render Windows
      int size[3];
      size[0] = size[1] = size[2] = 0;
      if (element->Attribute(mitk::InteractionEventConst::xmlRenderSizeX()) != nullptr)
      {
        size[0] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeX())->c_str());
      }
      if (element->Attribute(mitk::InteractionEventConst::xmlRenderSizeY()) != nullptr)
      {
        size[1] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeY())->c_str());
      }
      if (element->Attribute(mitk::InteractionEventConst::xmlRenderSizeZ()) != nullptr)
      {
        size[2] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlRenderSizeZ())->c_str());
      }

      // create renderWindow, renderer and dispatcher
      mitk::RenderWindow::Pointer rw =
        mitk::RenderWindow::New(nullptr, rendererName, rm); // VtkRenderWindow is created within constructor if nullptr

      if (size[0] != 0 && size[1] != 0)
      {
        rw->SetSize(size[0], size[1]);
        rw->GetRenderer()->Resize(size[0], size[1]);
      }

      // set storage of renderer
      rw->GetRenderer()->SetDataStorage(m_DataStorage);

      // set view direction to axial
      rw->GetSliceNavigationController()->SetDefaultViewDirection(viewDirection);

      // set renderer to render 2D
      rw->GetRenderer()->SetMapperID(mapperID);

      rw->GetRenderer()->PrepareRender();

      // Some more magic for the 3D render window case:
      // Camera view direction, position and focal point

      if (mapperID == mitk::BaseRenderer::Standard3D)
      {
        if (element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointX()) != nullptr)
        {
          double cameraFocalPoint[3];

          cameraFocalPoint[0] =
            std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointX())->c_str());
          cameraFocalPoint[1] =
            std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointY())->c_str());
          cameraFocalPoint[2] =
            std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointZ())->c_str());
          rw->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetFocalPoint(cameraFocalPoint);
        }

        if (element->Attribute(mitk::InteractionEventConst::xmlCameraPositionX()) != nullptr)
        {
          double cameraPosition[3];

          cameraPosition[0] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraPositionX())->c_str());
          cameraPosition[1] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraPositionY())->c_str());
          cameraPosition[2] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraPositionZ())->c_str());
          rw->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetPosition(cameraPosition);
        }

        if (element->Attribute(mitk::InteractionEventConst::xmlViewUpX()) != nullptr)
        {
          double viewUp[3];

          viewUp[0] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlViewUpX())->c_str());
          viewUp[1] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlViewUpY())->c_str());
          viewUp[2] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlViewUpZ())->c_str());
          rw->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetViewUp(viewUp);
        }
      }

      rw->GetVtkRenderWindow()->Render();
      rw->GetVtkRenderWindow()->WaitForCompletion();

      // connect SliceNavigationControllers to timestep changed event of TimeNavigationController
      rw->GetSliceNavigationController()->ConnectGeometryTimeEvent(rm->GetTimeNavigationController(), false);
      rm->GetTimeNavigationController()->ConnectGeometryTimeEvent(rw->GetSliceNavigationController(), false);

      // add to list of kown render windows
      m_RenderWindowList.push_back(rw);
    }

    // TODO: check the following lines taken from QmitkStdMultiWidget and adapt them to be executed in our code here.
    //    mitkWidget1->GetSliceNavigationController()
    //      ->ConnectGeometrySendEvent(mitk::BaseRenderer::GetInstance(mitkWidget4->GetRenderWindow()));

    //########### register display interactor to handle scroll events ##################
    // use MouseModeSwitcher to ensure that the statemachine of DisplayInteractor is loaded correctly
    m_MouseModeSwitcher = mitk::MouseModeSwitcher::New();
  }
  else
  {
    mitkThrow() << "Can not load interaction xml file <" << m_InteractionFilePath << ">";
  }

  // WARNING assumes a 3D window exists !!!!
  this->AddDisplayPlaneSubTree();
}

mitk::InteractionTestHelper::~InteractionTestHelper()
{
  mitk::RenderingManager *rm = mitk::RenderingManager::GetInstance();

  // unregister renderers
  auto it = m_RenderWindowList.begin();
  auto end = m_RenderWindowList.end();

  for (; it != end; ++it)
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

  this->Set3dCameraSettings();
}

void mitk::InteractionTestHelper::PlaybackInteraction()
{
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);
  // load events if not loaded yet
  if (m_Events.empty())
    this->LoadInteraction();

  auto it = m_RenderWindowList.begin();
  auto end = m_RenderWindowList.end();
  for (; it != end; ++it)
  {
    (*it)->GetRenderer()->PrepareRender();

    (*it)->GetVtkRenderWindow()->Render();
    (*it)->GetVtkRenderWindow()->WaitForCompletion();
  }
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);

  it = m_RenderWindowList.begin();
  for (; it != end; ++it)
  {
    (*it)->GetVtkRenderWindow()->Render();
    (*it)->GetVtkRenderWindow()->WaitForCompletion();
  }

  // mitk::RenderingManager::GetInstance()->ForceImmediateUpdateAll();
  // playback all events in queue
  for (unsigned long i = 0; i < m_Events.size(); ++i)
  {
    // let dispatcher of sending renderer process the event
    m_Events.at(i)->GetSender()->GetDispatcher()->ProcessEvent(m_Events.at(i));
  }
  if (false)
  {
    it--;
    (*it)->GetVtkRenderWindow()->GetInteractor()->Start();
  }
}

void mitk::InteractionTestHelper::LoadInteraction()
{
  // load interaction pattern from xml file
  std::ifstream xmlStream(m_InteractionFilePath.c_str());
  mitk::XML2EventParser parser(xmlStream);
  m_Events = parser.GetInteractions();
  xmlStream.close();
  // Avoid VTK warning: Trying to delete object with non-zero reference count.
  parser.SetReferenceCount(0);
}

void mitk::InteractionTestHelper::SetTimeStep(int newTimeStep)
{
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(m_DataStorage);

  bool timeStepIsvalid =
    mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetCreatedWorldGeometry()->IsValidTimeStep(
      newTimeStep);

  if (timeStepIsvalid)
  {
    mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->SetPos(newTimeStep);
  }
}

mitk::RenderWindow *mitk::InteractionTestHelper::GetRenderWindowByName(const std::string &name)
{
  auto it = m_RenderWindowList.begin();
  auto end = m_RenderWindowList.end();

  for (; it != end; ++it)
  {
    if (name.compare((*it)->GetRenderer()->GetName()) == 0)
      return (*it).GetPointer();
  }

  return nullptr;
}

mitk::RenderWindow *mitk::InteractionTestHelper::GetRenderWindowByDefaultViewDirection(
  mitk::SliceNavigationController::ViewDirection viewDirection)
{
  auto it = m_RenderWindowList.begin();
  auto end = m_RenderWindowList.end();

  for (; it != end; ++it)
  {
    if (viewDirection == (*it)->GetSliceNavigationController()->GetDefaultViewDirection())
      return (*it).GetPointer();
  }

  return nullptr;
}

mitk::RenderWindow *mitk::InteractionTestHelper::GetRenderWindow(unsigned int index)
{
  if (index < m_RenderWindowList.size())
  {
    return m_RenderWindowList.at(index).GetPointer();
  }
  else
  {
    return nullptr;
  }
}

void mitk::InteractionTestHelper::AddDisplayPlaneSubTree()
{
  // add the displayed planes of the multiwidget to a node to which the subtree
  // @a planesSubTree points ...

  mitk::PlaneGeometryDataMapper2D::Pointer mapper;
  mitk::IntProperty::Pointer layer = mitk::IntProperty::New(1000);

  mitk::DataNode::Pointer node = mitk::DataNode::New();
  node->SetProperty("name", mitk::StringProperty::New("Widgets"));
  node->SetProperty("helper object", mitk::BoolProperty::New(true));

  m_DataStorage->Add(node);

  for (auto it : m_RenderWindowList)
  {
    if (it->GetRenderer()->GetMapperID() == BaseRenderer::Standard3D)
      continue;

    // ... of widget 1
    mitk::DataNode::Pointer planeNode1 =
      (mitk::BaseRenderer::GetInstance(it->GetVtkRenderWindow()))->GetCurrentWorldPlaneGeometryNode();

    planeNode1->SetProperty("visible", mitk::BoolProperty::New(true));
    planeNode1->SetProperty("name", mitk::StringProperty::New("widget1Plane"));
    planeNode1->SetProperty("includeInBoundingBox", mitk::BoolProperty::New(false));
    planeNode1->SetProperty("helper object", mitk::BoolProperty::New(true));
    planeNode1->SetProperty("layer", layer);
    planeNode1->SetColor(1.0, 0.0, 0.0);
    mapper = mitk::PlaneGeometryDataMapper2D::New();
    planeNode1->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
    m_DataStorage->Add(planeNode1, node);
  }
}

void mitk::InteractionTestHelper::Set3dCameraSettings()
{
  TiXmlDocument document(m_InteractionFilePath);
  bool loadOkay = document.LoadFile();
  if (loadOkay)
  {
    // for each renderer found create a render window and configure
    for (TiXmlElement *element = document.FirstChildElement(mitk::InteractionEventConst::xmlTagInteractions())
                                   ->FirstChildElement(mitk::InteractionEventConst::xmlTagConfigRoot())
                                   ->FirstChildElement(mitk::InteractionEventConst::xmlTagRenderer());
         element != nullptr;
         element = element->NextSiblingElement(mitk::InteractionEventConst::xmlTagRenderer()))
    {
      // get name of renderer
      const char *rendererName =
        element->Attribute(mitk::InteractionEventConst::xmlEventPropertyRendererName().c_str());

      // get mapper slot id
      mitk::BaseRenderer::MapperSlotId mapperID = mitk::BaseRenderer::Standard2D;
      if (element->Attribute(mitk::InteractionEventConst::xmlEventPropertyMapperID()) != nullptr)
      {
        int mapperIDNum =
          std::atoi(element->Attribute(mitk::InteractionEventConst::xmlEventPropertyMapperID())->c_str());
        mapperID = static_cast<mitk::BaseRenderer::MapperSlotId>(mapperIDNum);
      }

      if (mapperID == mitk::BaseRenderer::Standard3D)
      {
        RenderWindow *namedRenderer = nullptr;

        for (auto it : m_RenderWindowList)
        {
          if (strcmp(it->GetRenderer()->GetName(), rendererName) == 0)
          {
            namedRenderer = it.GetPointer();
            break;
          }
        }

        if (namedRenderer == nullptr)
        {
          MITK_ERROR << "No match for render window was found.";
          return;
        }
        namedRenderer->GetRenderer()->PrepareRender();

        if (element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointX()) != nullptr)
        {
          double cameraFocalPoint[3];

          cameraFocalPoint[0] =
            std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointX())->c_str());
          cameraFocalPoint[1] =
            std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointY())->c_str());
          cameraFocalPoint[2] =
            std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraFocalPointZ())->c_str());
          namedRenderer->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetFocalPoint(cameraFocalPoint);
        }

        if (element->Attribute(mitk::InteractionEventConst::xmlCameraPositionX()) != nullptr)
        {
          double cameraPosition[3];

          cameraPosition[0] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraPositionX())->c_str());
          cameraPosition[1] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraPositionY())->c_str());
          cameraPosition[2] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlCameraPositionZ())->c_str());
          namedRenderer->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetPosition(cameraPosition);
        }

        if (element->Attribute(mitk::InteractionEventConst::xmlViewUpX()) != nullptr)
        {
          double viewUp[3];

          viewUp[0] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlViewUpX())->c_str());
          viewUp[1] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlViewUpY())->c_str());
          viewUp[2] = std::atoi(element->Attribute(mitk::InteractionEventConst::xmlViewUpZ())->c_str());
          namedRenderer->GetRenderer()->GetVtkRenderer()->GetActiveCamera()->SetViewUp(viewUp);
        }

        namedRenderer->GetVtkRenderWindow()->Render();
      }
    }
  }
}
