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

#include "mitkBaseRenderer.h"
#include "mitkMapper.h"
#include "mitkResliceMethodProperty.h"

// Geometries
#include "mitkPlaneGeometry.h"
#include "mitkSlicedGeometry3D.h"

// Controllers
#include "mitkCameraController.h"
#include "mitkSliceNavigationController.h"
#include "mitkCameraRotationController.h"

#include "mitkVtkLayerController.h"

#include "mitkProperties.h"
#include "mitkWeakPointerProperty.h"
#include "mitkInteractionConst.h"
#include "mitkOverlayManager.h"

// VTK
#include <vtkLinearTransform.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>

#include <vtkProperty.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkActor.h>

mitk::BaseRenderer::BaseRendererMapType mitk::BaseRenderer::baseRendererMap;

mitk::BaseRenderer* mitk::BaseRenderer::GetInstance(vtkRenderWindow * renWin)
{
  for (BaseRendererMapType::iterator mapit = baseRendererMap.begin(); mapit != baseRendererMap.end(); ++mapit)
  {
    if ((*mapit).first == renWin)
      return (*mapit).second;
  }
  return nullptr;
}

void mitk::BaseRenderer::AddInstance(vtkRenderWindow* renWin, BaseRenderer* baseRenderer)
{
  if (renWin == nullptr || baseRenderer == nullptr)
    return;

  // ensure that no BaseRenderer is managed twice
  mitk::BaseRenderer::RemoveInstance(renWin);

  baseRendererMap.insert(BaseRendererMapType::value_type(renWin, baseRenderer));
}

void mitk::BaseRenderer::RemoveInstance(vtkRenderWindow* renWin)
{
  BaseRendererMapType::iterator mapit = baseRendererMap.find(renWin);
  if (mapit != baseRendererMap.end())
    baseRendererMap.erase(mapit);
}

mitk::BaseRenderer* mitk::BaseRenderer::GetByName(const std::string& name)
{
  for (BaseRendererMapType::iterator mapit = baseRendererMap.begin(); mapit != baseRendererMap.end(); ++mapit)
  {
    if ((*mapit).second->m_Name == name)
      return (*mapit).second;
  }
  return nullptr;
}

vtkRenderWindow* mitk::BaseRenderer::GetRenderWindowByName(const std::string& name)
{
  for (BaseRendererMapType::iterator mapit = baseRendererMap.begin(); mapit != baseRendererMap.end(); ++mapit)
  {
    if ((*mapit).second->m_Name == name)
      return (*mapit).first;
  }
  return nullptr;
}

mitk::BaseRenderer::BaseRenderer(const char* name, vtkRenderWindow * renWin, mitk::RenderingManager* rm, RenderingMode::Type renderingMode, bool useFXAA)
  : m_RenderWindow(nullptr)
  , m_VtkRenderer(nullptr)
  , m_MapperID(defaultMapper)
  , m_DataStorage(nullptr)
  , m_RenderingManager(rm)
  , m_LastUpdateTime(0)
  , m_CameraController(nullptr)
  , m_SliceNavigationController(nullptr)
  , m_CameraRotationController(nullptr)
  , m_WorldTimeGeometry(nullptr)
  , m_CurrentWorldGeometry(nullptr)
  , m_CurrentWorldPlaneGeometry(nullptr)
  , m_Slice(0)
  , m_TimeStep()
  , m_Component()
  , m_CurrentWorldPlaneGeometryUpdateTime()
  , m_TimeStepUpdateTime()
  , m_KeepDisplayedRegion(true)
  , m_CurrentWorldPlaneGeometryData(nullptr)
  , m_CurrentWorldPlaneGeometryNode(nullptr)
  , m_CurrentWorldPlaneGeometryTransformTime(0)
  , m_Name(name)
  , m_EmptyWorldGeometry(true)
  , m_NumberOfVisibleLODEnabledMappers(0)
  , m_StudyUID()
  , m_ConstrainZoomingAndPanning(false)
  , m_3dDrawingLock(0)
{
  m_Bounds[0] = 0;
  m_Bounds[1] = 0;
  m_Bounds[2] = 0;
  m_Bounds[3] = 0;
  m_Bounds[4] = 0;
  m_Bounds[5] = 0;

  if (name != nullptr)
  {
    m_Name = name;
  }
  else
  {
    m_Name = "unnamed renderer";
    itkWarningMacro(<< "Created unnamed renderer. Bad for serialization. Please choose a name.");
  }

  if (renWin != nullptr)
  {
    m_RenderWindow = renWin;
    m_RenderWindow->Register(nullptr);
  }
  else
  {
    itkWarningMacro(<< "Created mitkBaseRenderer without vtkRenderWindow present.");
  }
  //instances.insert( this );

  //adding this BaseRenderer to the List of all BaseRenderer

  m_BindDispatcherInteractor = new mitk::BindDispatcherInteractor(GetName());

  WeakPointerProperty::Pointer rendererProp = WeakPointerProperty::New((itk::Object*) this);

  m_CurrentWorldPlaneGeometry = mitk::PlaneGeometry::New();

  m_CurrentWorldPlaneGeometryData = mitk::PlaneGeometryData::New();
  m_CurrentWorldPlaneGeometryData->SetPlaneGeometry(m_CurrentWorldPlaneGeometry);
  m_CurrentWorldPlaneGeometryNode = mitk::DataNode::New();
  m_CurrentWorldPlaneGeometryNode->SetData(m_CurrentWorldPlaneGeometryData);
  m_CurrentWorldPlaneGeometryNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_CurrentWorldPlaneGeometryNode->GetPropertyList()->SetProperty("layer", IntProperty::New(1000));

  m_CurrentWorldPlaneGeometryNode->SetProperty("reslice.thickslices", mitk::ResliceMethodProperty::New());
  m_CurrentWorldPlaneGeometryNode->SetProperty("reslice.thickslices.num", mitk::IntProperty::New(1));

  m_CurrentWorldPlaneGeometryTransformTime = m_CurrentWorldPlaneGeometryNode->GetVtkTransform()->GetMTime();

  mitk::SliceNavigationController::Pointer sliceNavigationController = mitk::SliceNavigationController::New();
  sliceNavigationController->SetRenderer(this);
  sliceNavigationController->ConnectGeometrySliceEvent(this);
  sliceNavigationController->ConnectGeometryUpdateEvent(this);
  sliceNavigationController->ConnectGeometryTimeEvent(this, false);
  sliceNavigationController->ConnectGeometryComponentEvent(this, false);
  m_SliceNavigationController = sliceNavigationController;

  m_CameraRotationController = mitk::CameraRotationController::New();
  m_CameraRotationController->SetRenderWindow(m_RenderWindow);
  m_CameraRotationController->AcquireCamera();

  m_CameraController = mitk::CameraController::New();
  m_CameraController->SetRenderer(this);

  m_VtkRenderer = vtkRenderer::New();
  if (useFXAA) {
    m_VtkRenderer->UseFXAAOn();
  }

  if (renderingMode == RenderingMode::DepthPeeling)
  {
    m_VtkRenderer->SetUseDepthPeeling(1);
    m_VtkRenderer->SetMaximumNumberOfPeels(8);
    m_VtkRenderer->SetOcclusionRatio(0.);
  }

  if (mitk::VtkLayerController::GetInstance(m_RenderWindow) == nullptr)
  {
    mitk::VtkLayerController::AddInstance(m_RenderWindow, m_VtkRenderer);
  }
  mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertSceneRenderer(m_VtkRenderer);
}

void mitk::BaseRenderer::SetStudyUID(std::string studyUID)
{
  m_StudyUID = studyUID;
}

std::string mitk::BaseRenderer::GetStudyUID()
{
  return m_StudyUID;
}

mitk::BaseRenderer::~BaseRenderer()
{
  if (m_OverlayManager.IsNotNull())
  {
    m_OverlayManager->RemoveBaseRenderer(this);
  }

  if (m_VtkRenderer != nullptr)
  {
    m_VtkRenderer->Delete();
    m_VtkRenderer = nullptr;
  }

  if (m_CameraController.IsNotNull())
    m_CameraController->SetRenderer(nullptr);


  mitk::VtkLayerController::RemoveInstance(m_RenderWindow);

  RemoveAllLocalStorages();

  m_DataStorage = nullptr;

  if (m_BindDispatcherInteractor != nullptr)
  {
    delete m_BindDispatcherInteractor;
  }

  if (m_RenderWindow != nullptr)
  {
    m_RenderWindow->Delete();
    m_RenderWindow = nullptr;
  }
}

void mitk::BaseRenderer::RemoveAllLocalStorages()
{
  this->InvokeEvent(mitk::BaseRenderer::RendererResetEvent());

  std::list<mitk::BaseLocalStorageHandler*>::iterator it;
  for (it = m_RegisteredLocalStorageHandlers.begin(); it != m_RegisteredLocalStorageHandlers.end(); ++it)
    (*it)->ClearLocalStorage(this, false);
  m_RegisteredLocalStorageHandlers.clear();
}

void mitk::BaseRenderer::RegisterLocalStorageHandler(mitk::BaseLocalStorageHandler *lsh)
{
  m_RegisteredLocalStorageHandlers.push_back(lsh);
}

mitk::Dispatcher::Pointer mitk::BaseRenderer::GetDispatcher() const
{
  return m_BindDispatcherInteractor->GetDispatcher();
}

void mitk::BaseRenderer::UnregisterLocalStorageHandler(mitk::BaseLocalStorageHandler *lsh)
{
  m_RegisteredLocalStorageHandlers.remove(lsh);
}

void mitk::BaseRenderer::SetDataStorage(DataStorage* storage)
{
  if (storage != m_DataStorage && storage != nullptr )
  {
    m_DataStorage = storage;
    m_BindDispatcherInteractor->SetDataStorage(m_DataStorage);
    this->Modified();
  }
}

const mitk::BaseRenderer::MapperSlotId mitk::BaseRenderer::defaultMapper = 1;

void mitk::BaseRenderer::Paint()
{
}

void mitk::BaseRenderer::Initialize()
{
}

void mitk::BaseRenderer::Resize(int w, int h)
{
  this->m_RenderWindow->SetSize(w,h);
}

void mitk::BaseRenderer::InitRenderer(vtkRenderWindow* renderwindow)
{
  if (m_RenderWindow != renderwindow)
  {
    if (m_RenderWindow != nullptr)
    {
      m_RenderWindow->Delete();
    }
    m_RenderWindow = renderwindow;
    if (m_RenderWindow != nullptr)
    {
      m_RenderWindow->Register(nullptr);
    }
  }
  RemoveAllLocalStorages();

  if (m_CameraController.IsNotNull())
  {
    m_CameraController->SetRenderer(this);
  }
}

void mitk::BaseRenderer::InitSize(int w, int h)
{
  this->m_RenderWindow->SetSize(w,h);
}

void mitk::BaseRenderer::SetSlice(unsigned int slice)
{
  if (m_Slice != slice)
  {
    m_Slice = slice;
    if (m_WorldTimeGeometry.IsNotNull())
    {
      // get world geometry which may be rotated, for the current time step
      SlicedGeometry3D* slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(m_WorldTimeGeometry->GetGeometryForTimeStep(m_TimeStep).GetPointer());
      if (slicedWorldGeometry != nullptr)
      {
        // if slice position is part of the world geometry...
        if (m_Slice >= slicedWorldGeometry->GetSlices())
          // set the current worldplanegeomety as the selected 2D slice of the world geometry
          m_Slice = slicedWorldGeometry->GetSlices() - 1;
        SetCurrentWorldPlaneGeometry(slicedWorldGeometry->GetPlaneGeometry(m_Slice));
        SetCurrentWorldGeometry(slicedWorldGeometry);
      }
    }
    else
      Modified();
  }
}

void mitk::BaseRenderer::SetOverlayManager(itk::SmartPointer<OverlayManager> overlayManager)
{
  if (overlayManager.IsNull())
    return;

  if (this->m_OverlayManager.IsNotNull())
  {
    if (this->m_OverlayManager.GetPointer() == overlayManager.GetPointer())
    {
      return;
    }
    else
    {
      this->m_OverlayManager->RemoveBaseRenderer(this);
    }
  }
  this->m_OverlayManager = overlayManager;
  this->m_OverlayManager->AddBaseRenderer(this); //TODO
}

itk::SmartPointer<mitk::OverlayManager> mitk::BaseRenderer::GetOverlayManager()
{
  if (this->m_OverlayManager.IsNull())
  {
    m_OverlayManager = mitk::OverlayManager::New();
    m_OverlayManager->AddBaseRenderer(this);
  }
  return this->m_OverlayManager;
}

void mitk::BaseRenderer::SetTimeStep(unsigned int timeStep)
{
  if (m_TimeStep != timeStep)
  {
    auto nodes = m_DataStorage->GetAll();
    for (int i = 0; i < nodes->size(); i++) {
      nodes->at(i)->SetIntProperty("Image.Displayed Timestep", timeStep);
    }

    m_TimeStep = timeStep;
    m_TimeStepUpdateTime.Modified();

    if (m_WorldTimeGeometry.IsNotNull())
    {
      if (m_TimeStep >= m_WorldTimeGeometry->CountTimeSteps())
        m_TimeStep = m_WorldTimeGeometry->CountTimeSteps() - 1;
      SlicedGeometry3D* slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(m_WorldTimeGeometry->GetGeometryForTimeStep(m_TimeStep).GetPointer());
      if (slicedWorldGeometry != nullptr)
      {
        SetCurrentWorldPlaneGeometry(slicedWorldGeometry->GetPlaneGeometry(m_Slice));
        SetCurrentWorldGeometry(slicedWorldGeometry);
      }
    }
    else
      Modified();
  }
}

void mitk::BaseRenderer::SetComponent(unsigned int component)
{
  if (m_Component == component) {
    return;
  }

  m_Component = component;
  
  auto nodes = m_DataStorage->GetAll();
  for (int i = 0; i < nodes->size(); i++)
    nodes->at(i)->SetIntProperty("Image.Displayed Component", component);

}

int mitk::BaseRenderer::GetTimeStep(const mitk::BaseData* data) const
{
  if ((data == nullptr) || (data->IsInitialized() == false))
  {
    return -1;
  }
  return data->GetTimeGeometry()->TimePointToTimeStep(GetTime());
}

mitk::ScalarType mitk::BaseRenderer::GetTime() const
{
  if (m_WorldTimeGeometry.IsNull())
  {
    return 0;
  }
  else
  {
    ScalarType timeInMS = m_WorldTimeGeometry->TimeStepToTimePoint(GetTimeStep());
    if (timeInMS == itk::NumericTraits<mitk::ScalarType>::NonpositiveMin())
      return 0;
    else
      return timeInMS;
  }
}

void mitk::BaseRenderer::SetWorldTimeGeometry(mitk::TimeGeometry* geometry)
{
  assert(geometry != nullptr);

  itkDebugMacro("setting WorldTimeGeometry to " << geometry);
  if (m_WorldTimeGeometry != geometry)
  {
    if (geometry->GetBoundingBoxInWorld()->GetDiagonalLength2() == 0)
      return;

    m_WorldTimeGeometry = geometry;
    itkDebugMacro("setting WorldTimeGeometry to " << m_WorldTimeGeometry);

    if (m_TimeStep >= m_WorldTimeGeometry->CountTimeSteps())
      m_TimeStep = m_WorldTimeGeometry->CountTimeSteps() - 1;

    BaseGeometry* geometry3d;
    geometry3d = m_WorldTimeGeometry->GetGeometryForTimeStep(m_TimeStep);
    SetWorldGeometry3D(geometry3d);
  }
}

void mitk::BaseRenderer::SetWorldGeometry3D(mitk::BaseGeometry* geometry)
{
  itkDebugMacro("setting WorldGeometry3D to " << geometry);

  if (geometry->GetBoundingBox()->GetDiagonalLength2() == 0)
    return;
  SlicedGeometry3D* slicedWorldGeometry;
  slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(geometry);

  PlaneGeometry::Pointer geometry2d;
  if (slicedWorldGeometry != nullptr)
  {
    if (m_Slice >= slicedWorldGeometry->GetSlices() && (m_Slice != 0))
      m_Slice = slicedWorldGeometry->GetSlices() - 1;
    geometry2d = slicedWorldGeometry->GetPlaneGeometry(m_Slice);
    if (geometry2d.IsNull())
    {
      PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
      plane->InitializeStandardPlane(slicedWorldGeometry);
      geometry2d = plane;
    }
    SetCurrentWorldGeometry(slicedWorldGeometry);
  }
  else
  {
    geometry2d = dynamic_cast<PlaneGeometry*>(geometry);
    if (geometry2d.IsNull())
    {
      PlaneGeometry::Pointer plane = PlaneGeometry::New();
      plane->InitializeStandardPlane(geometry);
      geometry2d = plane;
    }
    SetCurrentWorldGeometry(geometry);
  }
  SetCurrentWorldPlaneGeometry(geometry2d); // calls Modified()

  if (m_CurrentWorldPlaneGeometry.IsNull())
    itkWarningMacro("m_CurrentWorldPlaneGeometry is nullptr");
}

void mitk::BaseRenderer::SetCurrentWorldPlaneGeometry(mitk::PlaneGeometry* geometry2d)
{
  if (m_CurrentWorldPlaneGeometry != geometry2d)
  {
    m_CurrentWorldPlaneGeometry = geometry2d;
    m_CurrentWorldPlaneGeometryData->SetPlaneGeometry(m_CurrentWorldPlaneGeometry);
    m_CurrentWorldPlaneGeometryUpdateTime.Modified();
    Modified();
  }
}

void mitk::BaseRenderer::SendUpdateSlice()
{
  m_CurrentWorldPlaneGeometryUpdateTime.Modified();
}

int *mitk::BaseRenderer::GetSize() const
{
  return this->m_RenderWindow->GetSize();
}

int *mitk::BaseRenderer::GetViewportSize() const
{
  return this->m_VtkRenderer->GetSize();
}

void mitk::BaseRenderer::SetCurrentWorldGeometry(mitk::BaseGeometry* geometry)
{
  m_CurrentWorldGeometry = geometry;
  if (geometry == nullptr)
  {
    m_Bounds[0] = 0;
    m_Bounds[1] = 0;
    m_Bounds[2] = 0;
    m_Bounds[3] = 0;
    m_Bounds[4] = 0;
    m_Bounds[5] = 0;
    m_EmptyWorldGeometry = true;
    return;
  }
  BoundingBox::Pointer boundingBox = m_CurrentWorldGeometry->CalculateBoundingBoxRelativeToTransform(nullptr);
  const BoundingBox::BoundsArrayType& worldBounds = boundingBox->GetBounds();
  m_Bounds[0] = worldBounds[0];
  m_Bounds[1] = worldBounds[1];
  m_Bounds[2] = worldBounds[2];
  m_Bounds[3] = worldBounds[3];
  m_Bounds[4] = worldBounds[4];
  m_Bounds[5] = worldBounds[5];
  if (boundingBox->GetDiagonalLength2() <= mitk::eps)
    m_EmptyWorldGeometry = true;
  else
    m_EmptyWorldGeometry = false;
}

void mitk::BaseRenderer::UpdateOverlays()
{
  if (m_OverlayManager.IsNotNull())
  {
    m_OverlayManager->UpdateOverlays(this);
  }
}

void mitk::BaseRenderer::SetGeometry(const itk::EventObject & geometrySendEvent)
{
  const SliceNavigationController::GeometrySendEvent* sendEvent =
    dynamic_cast<const SliceNavigationController::GeometrySendEvent *>(&geometrySendEvent);

  assert(sendEvent != nullptr);
  SetWorldTimeGeometry(sendEvent->GetTimeGeometry());
}

void mitk::BaseRenderer::UpdateGeometry(const itk::EventObject & geometryUpdateEvent)
{
  const SliceNavigationController::GeometryUpdateEvent* updateEvent =
    dynamic_cast<const SliceNavigationController::GeometryUpdateEvent*>(&geometryUpdateEvent);

  if (updateEvent == nullptr)
    return;

  if (m_CurrentWorldGeometry.IsNotNull())
  {
    SlicedGeometry3D* slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(m_CurrentWorldGeometry.GetPointer());
    if (slicedWorldGeometry)
    {
      PlaneGeometry* geometry2D = slicedWorldGeometry->GetPlaneGeometry(m_Slice);

      SetCurrentWorldPlaneGeometry(geometry2D); // calls Modified()
    }
  }
}

void mitk::BaseRenderer::SetGeometrySlice(const itk::EventObject & geometrySliceEvent)
{
  const SliceNavigationController::GeometrySliceEvent* sliceEvent =
    dynamic_cast<const SliceNavigationController::GeometrySliceEvent *>(&geometrySliceEvent);

  assert(sliceEvent != nullptr);
  SetSlice(sliceEvent->GetPos());
}

void mitk::BaseRenderer::SetGeometryTime(const itk::EventObject & geometryTimeEvent)
{
  const SliceNavigationController::GeometryTimeEvent * timeEvent =
    dynamic_cast<const SliceNavigationController::GeometryTimeEvent *>(&geometryTimeEvent);

  assert(timeEvent != nullptr);
  SetTimeStep(timeEvent->GetPos());
}

const double* mitk::BaseRenderer::GetBounds() const
{
  return m_Bounds;
}

void mitk::BaseRenderer::SetGeometryComponent(const itk::EventObject& geometryComponentEvent)
{
  const SliceNavigationController::GeometryComponentEvent* componentEvent =
    dynamic_cast<const SliceNavigationController::GeometryComponentEvent*> (&geometryComponentEvent);

  assert(componentEvent != nullptr);
  SetComponent(componentEvent->GetPos());
}

void mitk::BaseRenderer::DrawOverlayMouse(mitk::Point2D& itkNotUsed(p2d))
{
  MITK_INFO << "BaseRenderer::DrawOverlayMouse()- should be inconcret implementation OpenGLRenderer." << std::endl;
}

void mitk::BaseRenderer::RequestUpdate()
{
  m_RenderingManager->RequestUpdate(this->m_RenderWindow);
}

void mitk::BaseRenderer::ForceImmediateUpdate()
{
  m_RenderingManager->ForceImmediateUpdate(this->m_RenderWindow);
}

unsigned int mitk::BaseRenderer::GetNumberOfVisibleLODEnabledMappers() const
{
  return m_NumberOfVisibleLODEnabledMappers;
}

mitk::RenderingManager* mitk::BaseRenderer::GetRenderingManager() const
{
  return m_RenderingManager.GetPointer();
}

/*!
 Sets the new Navigation controller
 */
void mitk::BaseRenderer::SetSliceNavigationController(mitk::SliceNavigationController *SlicenavigationController)
{
  if (SlicenavigationController == nullptr)
    return;

  //copy worldgeometry
  SlicenavigationController->SetInputWorldTimeGeometry(SlicenavigationController->GetCreatedWorldGeometry());
  SlicenavigationController->Update();

  //set new
  m_SliceNavigationController = SlicenavigationController;
  m_SliceNavigationController->SetRenderer(this);

  if (m_SliceNavigationController.IsNotNull())
  {
    m_SliceNavigationController->ConnectGeometrySliceEvent(this);
    m_SliceNavigationController->ConnectGeometryUpdateEvent(this);
    m_SliceNavigationController->ConnectGeometryTimeEvent(this, false);
  }
}

void mitk::BaseRenderer::DisplayToWorld(const Point2D& displayPoint, Point3D& worldIndex) const
{
  if (m_MapperID == BaseRenderer::Standard2D)
  {
    double display[3], *world;

    //For the rigth z-position in display coordinates, take the focal point, convert it to display and use it for correct depth.
    double *displayCoord;
    double cameraFP[4];
    // Get camera focal point and position. Convert to display (screen)
    // coordinates. We need a depth value for z-buffer.
    this->GetVtkRenderer()->GetActiveCamera()->GetFocalPoint(cameraFP);
    cameraFP[3] = 0.0;
    this->GetVtkRenderer()->SetWorldPoint(cameraFP[0], cameraFP[1], cameraFP[2], cameraFP[3]);
    this->GetVtkRenderer()->WorldToDisplay();
    displayCoord = this->GetVtkRenderer()->GetDisplayPoint();

    // now convert the display point to world coordinates
    display[0] = displayPoint[0];
    display[1] = displayPoint[1];
    display[2] = displayCoord[2];

    this->GetVtkRenderer()->SetDisplayPoint(display);
    this->GetVtkRenderer()->DisplayToWorld();
    world = this->GetVtkRenderer()->GetWorldPoint();

    for (int i = 0; i < 3; i++)
    {
      worldIndex[i] = world[i] / world[3];
    }
  }
  else if (m_MapperID == BaseRenderer::Standard3D)
  {
    PickWorldPoint(displayPoint, worldIndex, PickingMode::CellPicking); //Seems to be the same code as above, but subclasses may contain different implementations.
  }

  return;
}

void mitk::BaseRenderer::DisplayToPlane(const Point2D& displayPoint, Point2D& planePointInMM) const
{
  if (m_MapperID == BaseRenderer::Standard2D)
  {
    Point3D worldPoint;
    this->DisplayToWorld(displayPoint, worldPoint);
    this->m_CurrentWorldPlaneGeometry->Map(worldPoint, planePointInMM);
  }
  else if (m_MapperID == BaseRenderer::Standard3D)
  {
    MITK_WARN << "No conversion possible with 3D mapper.";
    return;
  }

  return;
}

void mitk::BaseRenderer::WorldToDisplay(const Point3D& worldIndex, Point2D& displayPoint) const
{
  double world[4], *display;

  world[0] = worldIndex[0];
  world[1] = worldIndex[1];
  world[2] = worldIndex[2];
  world[3] = 1.0;

  this->GetVtkRenderer()->SetWorldPoint(world);
  this->GetVtkRenderer()->WorldToDisplay();
  display = this->GetVtkRenderer()->GetDisplayPoint();

  displayPoint[0] = display[0];
  displayPoint[1] = display[1];

  return;
}

void mitk::BaseRenderer::WorldToView(const mitk::Point3D& worldIndex, mitk::Point2D &viewPoint) const
{
  double world[4], *view;

  world[0] = worldIndex[0];
  world[1] = worldIndex[1];
  world[2] = worldIndex[2];
  world[3] = 1.0;

  this->GetVtkRenderer()->SetWorldPoint(world);
  this->GetVtkRenderer()->WorldToView();
  view = this->GetVtkRenderer()->GetViewPoint();
  this->GetVtkRenderer()->ViewToNormalizedViewport(view[0], view[1], view[2]);

  viewPoint[0] = view[0] * this->GetViewportSize()[0];
  viewPoint[1] = view[1] * this->GetViewportSize()[1];

  return;
}

void mitk::BaseRenderer::PlaneToDisplay(const Point2D& planePointInMM, Point2D& displayPoint) const
{
  Point3D worldPoint;
  this->m_CurrentWorldPlaneGeometry->Map(planePointInMM, worldPoint);
  this->WorldToDisplay(worldPoint, displayPoint);

  return;
}

void mitk::BaseRenderer::PlaneToView(const Point2D& planePointInMM, Point2D& viewPoint) const
{
  Point3D worldPoint;
  this->m_CurrentWorldPlaneGeometry->Map(planePointInMM, worldPoint);
  this->WorldToView(worldPoint, viewPoint);

  return; 
}

double mitk::BaseRenderer::GetScaleFactorMMPerDisplayUnit() const
{
  if (this->GetMapperID() == BaseRenderer::Standard2D)
  {
    //GetParallelScale returns half of the height of the render window in mm.
    //Divided by the half size of the Display size in pixel givest the mm per pixel.
    return this->GetVtkRenderer()->GetActiveCamera()->GetParallelScale()
      * 2.0 / GetViewportSize()[1];
  }
  else return 1.0;
}

mitk::Point2D mitk::BaseRenderer::GetDisplaySizeInMM() const
{
  Point2D dispSizeInMM;
  dispSizeInMM[0] = GetSizeX() * GetScaleFactorMMPerDisplayUnit();
  dispSizeInMM[1] = GetSizeY() * GetScaleFactorMMPerDisplayUnit();
  return dispSizeInMM;
}

mitk::Point2D mitk::BaseRenderer::GetViewportSizeInMM() const
{
  Point2D dispSizeInMM;
  dispSizeInMM[0] = GetViewportSize()[0] * GetScaleFactorMMPerDisplayUnit();
  dispSizeInMM[1] = GetViewportSize()[1] * GetScaleFactorMMPerDisplayUnit();
  return dispSizeInMM;
}

mitk::Point2D mitk::BaseRenderer::GetOriginInMM() const
{
  Point2D originPx;
  originPx[0] = m_VtkRenderer->GetOrigin()[0];
  originPx[1] = m_VtkRenderer->GetOrigin()[1];
  Point2D displayGeometryOriginInMM;
  DisplayToPlane(originPx, displayGeometryOriginInMM);//top left of the render window (Origin)
  return displayGeometryOriginInMM;
}

void mitk::BaseRenderer::SetConstrainZoomingAndPanning(bool constrain)
{
  m_ConstrainZoomingAndPanning = constrain;
  if (m_ConstrainZoomingAndPanning)
  {
    this->GetCameraController()->AdjustCameraToPlane();
    this->GetCameraRotationController()->RotateCameraToTransformationAngles();
  }
}


mitk::Point3D mitk::BaseRenderer::Map2DRendererPositionTo3DWorldPosition(const Point2D& mousePosition) const
{
  //DEPRECATED: Map2DRendererPositionTo3DWorldPosition is deprecated. use DisplayToWorldInstead
  Point3D position;
  DisplayToWorld(mousePosition, position);
  return position;
}

void mitk::BaseRenderer::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << " MapperID: " << m_MapperID << std::endl;
  os << indent << " Slice: " << m_Slice << std::endl;
  os << indent << " TimeStep: " << m_TimeStep << std::endl;

  os << indent << " CurrentWorldPlaneGeometry: ";
  if (m_CurrentWorldPlaneGeometry.IsNull())
    os << "nullptr" << std::endl;
  else
    m_CurrentWorldPlaneGeometry->Print(os, indent);

  os << indent << " CurrentWorldPlaneGeometryUpdateTime: " << m_CurrentWorldPlaneGeometryUpdateTime << std::endl;
  os << indent << " CurrentWorldPlaneGeometryTransformTime: " << m_CurrentWorldPlaneGeometryTransformTime << std::endl;

  Superclass::PrintSelf(os, indent);
}

void mitk::BaseRenderer::lock3dDrawing()
{
  m_3dDrawingLock++;
  m_CurrentWorldPlaneGeometryUpdateTime.Modified();
}

void mitk::BaseRenderer::unlock3dDrawing()
{
  if (m_3dDrawingLock > 0) {
    m_3dDrawingLock--;
    m_CurrentWorldPlaneGeometryUpdateTime.Modified();
  }
}

bool mitk::BaseRenderer::is3dDrawingLocked() const
{
  return m_3dDrawingLock > 0;
}

