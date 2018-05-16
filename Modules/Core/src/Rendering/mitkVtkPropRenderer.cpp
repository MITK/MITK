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

#include "mitkVtkPropRenderer.h"

// MAPPERS
#include "mitkCameraController.h"
#include "mitkImageVtkMapper2D.h"
#include "mitkMapper.h"
#include "mitkPlaneGeometryDataVtkMapper3D.h"
#include "mitkVtkMapper.h"

#include <mitkAbstractTransformGeometry.h>
#include <mitkGeometry3D.h>
#include <mitkImageSliceSelector.h>
#include <mitkLevelWindow.h>
#include <mitkNodePredicateDataType.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkSurface.h>
#include <mitkVtkInteractorStyle.h>

// VTK
#include <vtkAssemblyNode.h>
#include <vtkAssemblyPath.h>
#include <vtkCamera.h>
#include <vtkCellPicker.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLight.h>
#include <vtkLightKit.h>
#include <vtkLinearTransform.h>
#include <vtkMapper.h>
#include <vtkPointPicker.h>
#include <vtkProp.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkWorldPointPicker.h>

mitk::VtkPropRenderer::VtkPropRenderer(const char *name,
                                       vtkRenderWindow *renWin,
                                       mitk::RenderingManager *rm,
                                       mitk::BaseRenderer::RenderingMode::Type renderingMode)
  : BaseRenderer(name, renWin, rm, renderingMode), m_CameraInitializedForMapperID(0)
{
  didCount = false;

  m_WorldPointPicker = vtkWorldPointPicker::New();

  m_PointPicker = vtkPointPicker::New();
  m_PointPicker->SetTolerance(0.0025);

  m_CellPicker = vtkCellPicker::New();
  m_CellPicker->SetTolerance(0.0025);

  mitk::PlaneGeometryDataVtkMapper3D::Pointer geometryMapper = mitk::PlaneGeometryDataVtkMapper3D::New();
  m_CurrentWorldPlaneGeometryMapper = geometryMapper;
  m_CurrentWorldPlaneGeometryNode->SetMapper(2, geometryMapper);

  m_LightKit = vtkLightKit::New();
  m_LightKit->AddLightsToRenderer(m_VtkRenderer);
  m_PickingMode = WorldPointPicking;

  m_TextRenderer = vtkRenderer::New();
  m_TextRenderer->SetRenderWindow(renWin);
  m_TextRenderer->SetInteractive(0);
  m_TextRenderer->SetErase(0);
}

/*!
\brief Destructs the VtkPropRenderer.
*/
mitk::VtkPropRenderer::~VtkPropRenderer()
{
  // Workaround for GLDisplayList Bug
  {
    m_MapperID = 0;
    checkState();
  }

  if (m_LightKit != nullptr)
    m_LightKit->Delete();

  if (m_VtkRenderer != nullptr)
  {
    m_CameraController = nullptr;

    m_VtkRenderer->Delete();

    m_VtkRenderer = nullptr;
  }
  else
    m_CameraController = nullptr;

  if (m_WorldPointPicker != nullptr)
    m_WorldPointPicker->Delete();
  if (m_PointPicker != nullptr)
    m_PointPicker->Delete();
  if (m_CellPicker != nullptr)
    m_CellPicker->Delete();
  if (m_TextRenderer != nullptr)
    m_TextRenderer->Delete();
}

void mitk::VtkPropRenderer::SetDataStorage(mitk::DataStorage *storage)
{
  if (storage == nullptr || storage == m_DataStorage)
    return;

  BaseRenderer::SetDataStorage(storage);

  static_cast<mitk::PlaneGeometryDataVtkMapper3D *>(m_CurrentWorldPlaneGeometryMapper.GetPointer())
    ->SetDataStorageForTexture(m_DataStorage.GetPointer());

  // Compute the geometry from the current data tree bounds and set it as world geometry
  this->SetWorldGeometryToDataStorageBounds();
}

bool mitk::VtkPropRenderer::SetWorldGeometryToDataStorageBounds()
{
  if (m_DataStorage.IsNull())
    return false;

  // initialize world geometry
  auto geometry = m_DataStorage->ComputeVisibleBoundingGeometry3D(nullptr, "includeInBoundingBox");

  if (geometry.IsNull())
    return false;

  this->SetWorldTimeGeometry(geometry);
  this->GetVtkRenderer()->ResetCamera();
  this->GetCameraController()->Fit();
  this->Modified();
  return true;
}

/*!
\brief

Called by the vtkMitkRenderProp in order to start MITK rendering process.
*/
int mitk::VtkPropRenderer::Render(mitk::VtkPropRenderer::RenderType type)
{
  // Do we have objects to render?
  if (this->GetEmptyWorldGeometry())
    return 0;

  if (m_DataStorage.IsNull())
    return 0;

  // Update mappers and prepare mapper queue
  if (type == VtkPropRenderer::Opaque)
    this->PrepareMapperQueue();

  // go through the generated list and let the sorted mappers paint
  for (auto it = m_MappersMap.cbegin(); it != m_MappersMap.cend(); it++)
  {
    Mapper *mapper = (*it).second;
    mapper->MitkRender(this, type);
  }

  // Render text
  if (type == VtkPropRenderer::Overlay)
  {
    if (m_TextCollection.size() > 0)
    {
      m_TextRenderer->SetViewport(this->GetVtkRenderer()->GetViewport());
      for (auto it = m_TextCollection.begin(); it != m_TextCollection.end(); ++it)
        m_TextRenderer->AddViewProp((*it).second);
      m_TextRenderer->Render();
    }
  }
  return 1;
}

/*!
\brief PrepareMapperQueue iterates the datatree

PrepareMapperQueue iterates the datatree in order to find mappers which shall be rendered. Also, it sortes the mappers
wrt to their layer.
*/
void mitk::VtkPropRenderer::PrepareMapperQueue()
{
  // variable for counting LOD-enabled mappers
  m_NumberOfVisibleLODEnabledMappers = 0;

  // Do we have to update the mappers ?
  if (m_LastUpdateTime < GetMTime() || m_LastUpdateTime < this->GetCurrentWorldPlaneGeometry()->GetMTime())
  {
    Update();
  }
  else if (m_MapperID >= 1 && m_MapperID < 6)
    Update();

  // remove all text properties before mappers will add new ones
  m_TextRenderer->RemoveAllViewProps();

  for (unsigned int i = 0; i < m_TextCollection.size(); i++)
  {
    m_TextCollection[i]->Delete();
  }
  m_TextCollection.clear();

  // clear priority_queue
  m_MappersMap.clear();

  int mapperNo = 0;

  // DataStorage
  if (m_DataStorage.IsNull())
    return;

  DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();

  for (DataStorage::SetOfObjects::ConstIterator it = allObjects->Begin(); it != allObjects->End(); ++it)
  {
    const DataNode::Pointer node = it->Value();
    if (node.IsNull())
      continue;
    const mitk::Mapper::Pointer mapper = node->GetMapper(m_MapperID);

    if (mapper.IsNull())
      continue;

    bool visible = true;
    node->GetVisibility(visible, this, "visible");

    // The information about LOD-enabled mappers is required by RenderingManager
    if (mapper->IsLODEnabled(this) && visible)
    {
      ++m_NumberOfVisibleLODEnabledMappers;
    }
    // mapper without a layer property get layer number 1
    int layer = 1;
    node->GetIntProperty("layer", layer, this);
    int nr = (layer << 16) + mapperNo;
    m_MappersMap.insert(std::pair<int, Mapper *>(nr, mapper));
    mapperNo++;
  }
}

void mitk::VtkPropRenderer::Update(mitk::DataNode *datatreenode)
{
  if (datatreenode != nullptr)
  {
    mitk::Mapper::Pointer mapper = datatreenode->GetMapper(m_MapperID);
    if (mapper.IsNotNull())
    {
      if (GetCurrentWorldPlaneGeometry()->IsValid())
      {
        mapper->Update(this);
        {
          auto *vtkmapper = dynamic_cast<VtkMapper *>(mapper.GetPointer());
          if (vtkmapper != nullptr)
          {
            vtkmapper->UpdateVtkTransform(this);
          }
        }
      }
    }
  }
}

void mitk::VtkPropRenderer::Update()
{
  if (m_DataStorage.IsNull())
    return;

  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
    Update(it->Value());

  Modified();
  m_LastUpdateTime = GetMTime();
}

/*!
\brief

This method is called from the two Constructors
*/
void mitk::VtkPropRenderer::InitRenderer(vtkRenderWindow *renderWindow)
{
  BaseRenderer::InitRenderer(renderWindow);

  vtkCallbackCommand *renderCallbackCommand = vtkCallbackCommand::New();
  renderCallbackCommand->SetCallback(VtkPropRenderer::RenderingCallback);
  renderWindow->GetInteractor()->AddObserver(vtkCommand::RenderEvent, renderCallbackCommand);
  renderCallbackCommand->Delete();

  if (renderWindow == nullptr)
  {
    m_InitNeeded = false;
    m_ResizeNeeded = false;
    return;
  }

  m_InitNeeded = true;
  m_ResizeNeeded = true;

  m_LastUpdateTime = 0;
}

void mitk::VtkPropRenderer::RenderingCallback(vtkObject *caller, unsigned long, void *, void *)
{
  auto *renderWindowInteractor = dynamic_cast<vtkRenderWindowInteractor *>(caller);
  if (!renderWindowInteractor)
    return;
  mitk::BaseRenderer *renderer = mitk::BaseRenderer::GetInstance(renderWindowInteractor->GetRenderWindow());
  if (renderer)
    renderer->RequestUpdate();
}

/*!
\brief Resize the OpenGL Window
*/
void mitk::VtkPropRenderer::Resize(int w, int h)
{
  BaseRenderer::Resize(w, h);
  m_RenderingManager->RequestUpdate(this->GetRenderWindow());
}

void mitk::VtkPropRenderer::InitSize(int w, int h)
{
  m_RenderWindow->SetSize(w, h);
  Superclass::InitSize(w, h);
  Modified();
  Update();
  if (m_VtkRenderer != nullptr)
  {
    int w = vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();
    m_VtkRenderer->ResetCamera();
    vtkObject::SetGlobalWarningDisplay(w);
  }
  this->GetCameraController()->Fit();
}

int mitk::VtkPropRenderer::WriteSimpleText(
  std::string text, double posX, double posY, double color1, double color2, double color3, float opacity)
{
  this->GetVtkRenderer()->ViewToDisplay();
  if (!text.empty())
  {
    Point2D p;
    vtkTextActor *textActor = vtkTextActor::New();

    textActor->SetDisplayPosition(posX, posY);
    textActor->SetInput(text.c_str());
    textActor->SetTextScaleModeToNone();
    textActor->GetTextProperty()->SetColor(color1, color2, color3); // TODO: Read color from node property
    textActor->GetTextProperty()->SetOpacity(opacity);
    int text_id = m_TextCollection.size();
    m_TextCollection.insert(TextMapType::value_type(text_id, textActor));
    return text_id;
  }
  else
  {
    return -1;
  }
}

void mitk::VtkPropRenderer::SetMapperID(const MapperSlotId mapperId)
{
  if (m_MapperID != mapperId)
    Superclass::SetMapperID(mapperId);

  // Workaround for GL Displaylist Bug
  checkState();
}

/*!
\brief Activates the current renderwindow.
*/
void mitk::VtkPropRenderer::MakeCurrent()
{
  if (m_RenderWindow != nullptr)
    m_RenderWindow->MakeCurrent();
}

void mitk::VtkPropRenderer::PickWorldPoint(const mitk::Point2D &displayPoint, mitk::Point3D &worldPoint) const
{
  if (this->GetRenderWindow()->GetNeverRendered() != 0)
    return; // somebody called picking before we ever rendered; cannot have enough information yet

  switch (m_PickingMode)
  {
    case (WorldPointPicking):
    {
      m_WorldPointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
      vtk2itk(m_WorldPointPicker->GetPickPosition(), worldPoint);
      break;
    }
    case (PointPicking):
    {
      m_PointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
      vtk2itk(m_PointPicker->GetPickPosition(), worldPoint);
      break;
    }
    case (CellPicking):
    {
      m_CellPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
      vtk2itk(m_CellPicker->GetPickPosition(), worldPoint);
      break;
    }
  }
  // todo: is this picking in 2D renderwindows?
  //    Superclass::PickWorldPoint(displayPoint, worldPoint);
}

mitk::DataNode *mitk::VtkPropRenderer::PickObject(const Point2D &displayPosition, Point3D &worldPosition) const
{
  m_CellPicker->InitializePickList();

  // Iterate over all DataStorage objects to determine all vtkProps intended
  // for picking
  DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();
  for (DataStorage::SetOfObjects::ConstIterator it = allObjects->Begin(); it != allObjects->End(); ++it)
  {
    const DataNode *node = it->Value();
    if (node == nullptr)
      continue;

    bool pickable = false;
    node->GetBoolProperty("pickable", pickable);
    if (!pickable)
      continue;

    auto *mapper = dynamic_cast<VtkMapper *>(node->GetMapper(m_MapperID));
    if (mapper == nullptr)
      continue;

    vtkProp *prop = mapper->GetVtkProp((mitk::BaseRenderer *)this);
    if (prop == nullptr)
      continue;

    m_CellPicker->AddPickList(prop);
  }

  // Do the picking and retrieve the picked vtkProp (if any)
  m_CellPicker->PickFromListOn();
  m_CellPicker->Pick(displayPosition[0], displayPosition[1], 0.0, m_VtkRenderer);
  m_CellPicker->PickFromListOff();

  vtk2itk(m_CellPicker->GetPickPosition(), worldPosition);
  vtkProp *prop = m_CellPicker->GetViewProp();

  if (prop == nullptr)
  {
    return nullptr;
  }

  // Iterate over all DataStorage objects to determine if the retrieved
  // vtkProp is owned by any associated mapper.
  for (DataStorage::SetOfObjects::ConstIterator it = allObjects->Begin(); it != allObjects->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if (node.IsNull())
      continue;

    mitk::Mapper *mapper = node->GetMapper(m_MapperID);
    if (mapper == nullptr)
      continue;

    auto *vtkmapper = dynamic_cast<VtkMapper *>(mapper);

    if (vtkmapper)
    {
      // if vtk-based, then ...
      if (vtkmapper->HasVtkProp(prop, const_cast<mitk::VtkPropRenderer *>(this)))
      {
        return node;
      }
    }
  }
  return nullptr;
}
// todo: is this 2D renderwindow picking?
//    return Superclass::PickObject( displayPosition, worldPosition );

vtkTextProperty *mitk::VtkPropRenderer::GetTextLabelProperty(int text_id)
{
  return this->m_TextCollection[text_id]->GetTextProperty();
}

void mitk::VtkPropRenderer::InitPathTraversal()
{
  if (m_DataStorage.IsNotNull())
  {
    this->UpdatePaths();
    this->m_Paths->InitTraversal();
  }
}

void mitk::VtkPropRenderer::UpdatePaths()
{
  if (m_DataStorage.IsNull())
  {
    return;
  }

  if (GetMTime() > m_PathTime || (m_Paths != nullptr && m_Paths->GetMTime() > m_PathTime))
  {
    // Create the list to hold all the paths
    m_Paths = vtkSmartPointer<vtkAssemblyPaths>::New();

    DataStorage::SetOfObjects::ConstPointer objects = m_DataStorage->GetAll();
    for (auto iter = objects->begin(); iter != objects->end(); ++iter)
    {
      vtkSmartPointer<vtkAssemblyPath> onePath = vtkSmartPointer<vtkAssemblyPath>::New();
      Mapper *mapper = (*iter)->GetMapper(BaseRenderer::Standard3D);
      if (mapper)
      {
        auto *vtkmapper = dynamic_cast<VtkMapper *>(mapper);

        if (nullptr != vtkmapper)
        {
          vtkProp *prop = vtkmapper->GetVtkProp(this);
          if (prop && prop->GetVisibility())
          {
            // add to assembly path
            onePath->AddNode(prop, prop->GetMatrix());
            m_Paths->AddItem(onePath);
          }
        }
      }
    }

    m_PathTime.Modified();
  }
}

int mitk::VtkPropRenderer::GetNumberOfPaths()
{
  UpdatePaths();
  return m_Paths->GetNumberOfItems();
}

vtkAssemblyPath *mitk::VtkPropRenderer::GetNextPath()
{
  return m_Paths ? m_Paths->GetNextItem() : nullptr;
}

void mitk::VtkPropRenderer::ReleaseGraphicsResources(vtkWindow * /*renWin*/)
{
  if (m_DataStorage.IsNull())
    return;

  DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();
  for (auto iter = allObjects->begin(); iter != allObjects->end(); ++iter)
  {
    DataNode::Pointer node = *iter;
    if (node.IsNull())
      continue;

    Mapper *mapper = node->GetMapper(m_MapperID);

    if (mapper)
    {
      auto *vtkmapper = dynamic_cast<VtkMapper *>(mapper);

      if (vtkmapper)
        vtkmapper->ReleaseGraphicsResources(this);
    }
  }
}

const vtkWorldPointPicker *mitk::VtkPropRenderer::GetWorldPointPicker() const
{
  return m_WorldPointPicker;
}

const vtkPointPicker *mitk::VtkPropRenderer::GetPointPicker() const
{
  return m_PointPicker;
}

const vtkCellPicker *mitk::VtkPropRenderer::GetCellPicker() const
{
  return m_CellPicker;
}

mitk::VtkPropRenderer::MappersMapType mitk::VtkPropRenderer::GetMappersMap() const
{
  return m_MappersMap;
}

// Workaround for GL Displaylist bug
static int glWorkAroundGlobalCount = 0;

bool mitk::VtkPropRenderer::useImmediateModeRendering()
{
  return glWorkAroundGlobalCount > 1;
}

void mitk::VtkPropRenderer::checkState()
{
  if (m_MapperID == Standard3D)
  {
    if (!didCount)
    {
      didCount = true;
      glWorkAroundGlobalCount++;

      if (glWorkAroundGlobalCount == 2)
      {
        MITK_INFO << "Multiple 3D Renderwindows active...: turning Immediate Rendering ON for legacy mappers";
        //          vtkMapper::GlobalImmediateModeRenderingOn();
      }
    }
  }
  else
  {
    if (didCount)
    {
      didCount = false;
      glWorkAroundGlobalCount--;
      if (glWorkAroundGlobalCount == 1)
      {
        MITK_INFO << "Single 3D Renderwindow active...: turning Immediate Rendering OFF for legacy mappers";
        //        vtkMapper::GlobalImmediateModeRenderingOff();
      }
    }
  }
}

//### Contains all methods which are neceassry before each VTK Render() call
void mitk::VtkPropRenderer::PrepareRender()
{
  if (this->GetMapperID() != m_CameraInitializedForMapperID)
  {
    Initialize2DvtkCamera(); // Set parallel projection etc.
  }
  GetCameraController()->AdjustCameraToPlane();
}

bool mitk::VtkPropRenderer::Initialize2DvtkCamera()
{
  if (this->GetMapperID() == Standard3D)
  {
    // activate parallel projection for 2D
    this->GetVtkRenderer()->GetActiveCamera()->SetParallelProjection(false);
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
      vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    this->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
    this->GetRenderWindow()->GetInteractor()->EnableRenderOff();
    m_CameraInitializedForMapperID = Standard3D;
  }
  else if (this->GetMapperID() == Standard2D)
  {
    // activate parallel projection for 2D
    this->GetVtkRenderer()->GetActiveCamera()->SetParallelProjection(true);
    // turn the light out in the scene in order to render correct grey values.
    // TODO Implement a property for light in the 2D render windows (in another method)
    this->GetVtkRenderer()->RemoveAllLights();

    vtkSmartPointer<mitkVtkInteractorStyle> style = vtkSmartPointer<mitkVtkInteractorStyle>::New();
    this->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
    this->GetRenderWindow()->GetInteractor()->EnableRenderOff();
    m_CameraInitializedForMapperID = Standard2D;
  }
  return true;
}
