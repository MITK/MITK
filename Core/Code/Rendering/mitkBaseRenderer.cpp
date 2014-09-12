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
#include "mitkKeyEvent.h"

// Geometries
#include "mitkPlaneGeometry.h"
#include "mitkSlicedGeometry3D.h"

// Controllers
#include "mitkCameraController.h"
#include "mitkSliceNavigationController.h"
#include "mitkCameraRotationController.h"
#include "mitkVtkInteractorCameraController.h"

#ifdef MITK_USE_TD_MOUSE
#include "mitkTDMouseVtkCameraController.h"
#else
#include "mitkCameraController.h"
#endif

#include "mitkVtkLayerController.h"

// Events // TODO: INTERACTION_LEGACY
#include "mitkEventMapper.h"
#include "mitkGlobalInteraction.h"
#include "mitkPositionEvent.h"
#include "mitkDisplayPositionEvent.h"
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
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

mitk::BaseRenderer::BaseRendererMapType mitk::BaseRenderer::baseRendererMap;

mitk::BaseRenderer* mitk::BaseRenderer::GetInstance(vtkRenderWindow * renWin)
{
  for (BaseRendererMapType::iterator mapit = baseRendererMap.begin(); mapit != baseRendererMap.end(); mapit++)
  {
    if ((*mapit).first == renWin)
      return (*mapit).second;
  }
  return NULL;
}

void mitk::BaseRenderer::AddInstance(vtkRenderWindow* renWin, BaseRenderer* baseRenderer)
{
  if (renWin == NULL || baseRenderer == NULL)
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
  for (BaseRendererMapType::iterator mapit = baseRendererMap.begin(); mapit != baseRendererMap.end(); mapit++)
  {
    if ((*mapit).second->m_Name == name)
      return (*mapit).second;
  }
  return NULL;
}

vtkRenderWindow* mitk::BaseRenderer::GetRenderWindowByName(const std::string& name)
{
  for (BaseRendererMapType::iterator mapit = baseRendererMap.begin(); mapit != baseRendererMap.end(); mapit++)
  {
    if ((*mapit).second->m_Name == name)
      return (*mapit).first;
  }
  return NULL;
}

mitk::BaseRenderer::BaseRenderer(const char* name, vtkRenderWindow * renWin, mitk::RenderingManager* rm,RenderingMode::Type renderingMode)
  : m_RenderWindow(NULL)
  , m_VtkRenderer(NULL)
  , m_MapperID(defaultMapper)
  , m_DataStorage(NULL)
  , m_RenderingManager(rm)
  , m_LastUpdateTime(0)
  , m_CameraController(NULL)
  , m_SliceNavigationController(NULL)
  , m_CameraRotationController(NULL)
  /*, m_Size()*/
  , m_Focused(false)
  , m_WorldGeometry(NULL)
  , m_WorldTimeGeometry(NULL)
  , m_CurrentWorldGeometry(NULL)
  , m_CurrentWorldPlaneGeometry(NULL)
  , m_DisplayGeometry(NULL)
  , m_Slice(0)
  , m_TimeStep()
  , m_CurrentWorldPlaneGeometryUpdateTime()
  , m_DisplayGeometryUpdateTime()
  , m_TimeStepUpdateTime()
  , m_KeepDisplayedRegion(true)
  , m_WorldGeometryData(NULL)
  , m_DisplayGeometryData(NULL)
  , m_CurrentWorldPlaneGeometryData(NULL)
  , m_WorldGeometryNode(NULL)
  , m_DisplayGeometryNode(NULL)
  , m_CurrentWorldPlaneGeometryNode(NULL)
  , m_DisplayGeometryTransformTime(0)
  , m_CurrentWorldPlaneGeometryTransformTime(0)
  , m_Name(name)
  /*, m_Bounds()*/
  , m_EmptyWorldGeometry(true)
  , m_NumberOfVisibleLODEnabledMappers(0)
{
  m_Bounds[0] = 0;
  m_Bounds[1] = 0;
  m_Bounds[2] = 0;
  m_Bounds[3] = 0;
  m_Bounds[4] = 0;
  m_Bounds[5] = 0;

  if (name != NULL)
  {
    m_Name = name;
  }
  else
  {
    m_Name = "unnamed renderer";
    itkWarningMacro(<< "Created unnamed renderer. Bad for serialization. Please choose a name.");
  }

  if (renWin != NULL)
  {
    m_RenderWindow = renWin;
    m_RenderWindow->Register(NULL);
  }
  else
  {
    itkWarningMacro(<< "Created mitkBaseRenderer without vtkRenderWindow present.");
  }

  m_Size[0] = 0;
  m_Size[1] = 0;

  //instances.insert( this );

  //adding this BaseRenderer to the List of all BaseRenderer
  // TODO: INTERACTION_LEGACY
  m_RenderingManager->GetGlobalInteraction()->AddFocusElement(this);

  m_BindDispatcherInteractor = new mitk::BindDispatcherInteractor( GetName() );

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

  m_DisplayGeometry = mitk::DisplayGeometry::New();
  m_DisplayGeometry->SetWorldGeometry(m_CurrentWorldPlaneGeometry);
  m_DisplayGeometryData = mitk::PlaneGeometryData::New();
  m_DisplayGeometryData->SetPlaneGeometry(m_DisplayGeometry);
  m_DisplayGeometryNode = mitk::DataNode::New();
  m_DisplayGeometryNode->SetData(m_DisplayGeometryData);
  m_DisplayGeometryNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_DisplayGeometryTransformTime = m_DisplayGeometryNode->GetVtkTransform()->GetMTime();

  mitk::SliceNavigationController::Pointer sliceNavigationController = mitk::SliceNavigationController::New("navigation");
  sliceNavigationController->SetRenderer(this);
  sliceNavigationController->ConnectGeometrySliceEvent(this);
  sliceNavigationController->ConnectGeometryUpdateEvent(this);
  sliceNavigationController->ConnectGeometryTimeEvent(this, false);
  m_SliceNavigationController = sliceNavigationController;

  m_CameraRotationController = mitk::CameraRotationController::New();
  m_CameraRotationController->SetRenderWindow(m_RenderWindow);
  m_CameraRotationController->AcquireCamera();

//if TD Mouse Interaction is activated, then call TDMouseVtkCameraController instead of VtkInteractorCameraController
#ifdef MITK_USE_TD_MOUSE
  m_CameraController = mitk::TDMouseVtkCameraController::New();
#else
  m_CameraController = mitk::CameraController::New(NULL);
#endif

  m_VtkRenderer = vtkRenderer::New();

  if( renderingMode == RenderingMode::DepthPeeling )
  {
    m_VtkRenderer->SetUseDepthPeeling(1);
    m_VtkRenderer->SetMaximumNumberOfPeels(8);
    m_VtkRenderer->SetOcclusionRatio(0.0);
  }

  if (mitk::VtkLayerController::GetInstance(m_RenderWindow) == NULL)
  {
    mitk::VtkLayerController::AddInstance(m_RenderWindow, m_VtkRenderer);
  }
  mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertSceneRenderer(m_VtkRenderer);
}

mitk::BaseRenderer::~BaseRenderer()
{
  if (m_OverlayManager.IsNotNull())
  {
    m_OverlayManager->RemoveBaseRenderer(this);
  }

  if (m_VtkRenderer != NULL)
  {
    m_VtkRenderer->Delete();
    m_VtkRenderer = NULL;
  }

  if (m_CameraController.IsNotNull())
    m_CameraController->SetRenderer(NULL);

  m_RenderingManager->GetGlobalInteraction()->RemoveFocusElement(this);

  mitk::VtkLayerController::RemoveInstance(m_RenderWindow);

  RemoveAllLocalStorages();

  m_DataStorage = NULL;

  if (m_BindDispatcherInteractor != NULL)
  {
    delete m_BindDispatcherInteractor;
  }

  if (m_RenderWindow != NULL)
  {
    m_RenderWindow->Delete();
    m_RenderWindow = NULL;
  }

}

void mitk::BaseRenderer::RemoveAllLocalStorages()
{
  this->InvokeEvent(mitk::BaseRenderer::RendererResetEvent());

  std::list<mitk::BaseLocalStorageHandler*>::iterator it;
  for (it = m_RegisteredLocalStorageHandlers.begin(); it != m_RegisteredLocalStorageHandlers.end(); it++)
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

mitk::Point3D mitk::BaseRenderer::Map2DRendererPositionTo3DWorldPosition(const Point2D& mousePosition) const
{
  Point2D p_mm;
  Point3D position;
  if (m_MapperID == 1)
  {
    GetDisplayGeometry()->DisplayToWorld(mousePosition, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
  }
  else if (m_MapperID == 2)
  {
    PickWorldPoint(mousePosition, position);
  }
  return position;
}

void mitk::BaseRenderer::UnregisterLocalStorageHandler(mitk::BaseLocalStorageHandler *lsh)
{
  m_RegisteredLocalStorageHandlers.remove(lsh);
}

void mitk::BaseRenderer::SetDataStorage(DataStorage* storage)
{
  if (storage != NULL)
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
  m_Size[0] = w;
  m_Size[1] = h;

  if (m_CameraController)
    m_CameraController->Resize(w, h); //(formerly problematic on windows: vtkSizeBug)

  GetDisplayGeometry()->SetSizeInDisplayUnits(w, h, m_KeepDisplayedRegion);
}

void mitk::BaseRenderer::InitRenderer(vtkRenderWindow* renderwindow)
{
  if (m_RenderWindow != renderwindow)
  {
    if (m_RenderWindow != NULL)
    {
      m_RenderWindow->Delete();
    }
    m_RenderWindow = renderwindow;
    if (m_RenderWindow != NULL)
    {
      m_RenderWindow->Register(NULL);
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
  m_Size[0] = w;
  m_Size[1] = h;
  GetDisplayGeometry()->SetSizeInDisplayUnits(w, h, false);
  GetDisplayGeometry()->Fit();
}

void mitk::BaseRenderer::SetSlice(unsigned int slice)
{
  if (m_Slice != slice)
  {
    m_Slice = slice;
    if (m_WorldTimeGeometry.IsNotNull())
    {
      SlicedGeometry3D* slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(m_WorldTimeGeometry->GetGeometryForTimeStep(m_TimeStep).GetPointer());
      if (slicedWorldGeometry != NULL)
      {
        if (m_Slice >= slicedWorldGeometry->GetSlices())
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
  if(overlayManager.IsNull())
    return;

  if(this->m_OverlayManager.IsNotNull())
  {
    if(this->m_OverlayManager.GetPointer() == overlayManager.GetPointer())
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
  if(this->m_OverlayManager.IsNull())
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
    m_TimeStep = timeStep;
    m_TimeStepUpdateTime.Modified();

    if (m_WorldTimeGeometry.IsNotNull())
    {
      if (m_TimeStep >= m_WorldTimeGeometry->CountTimeSteps())
        m_TimeStep = m_WorldTimeGeometry->CountTimeSteps() - 1;
      SlicedGeometry3D* slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(m_WorldTimeGeometry->GetGeometryForTimeStep(m_TimeStep).GetPointer());
      if (slicedWorldGeometry != NULL)
      {
        SetCurrentWorldPlaneGeometry(slicedWorldGeometry->GetPlaneGeometry(m_Slice));
        SetCurrentWorldGeometry(slicedWorldGeometry);
      }
    }
    else
      Modified();
  }
}

int mitk::BaseRenderer::GetTimeStep(const mitk::BaseData* data) const
{
  if ((data == NULL) || (data->IsInitialized() == false))
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
  assert(geometry != NULL);

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

  if (m_WorldGeometry != geometry)
  {
    if (geometry->GetBoundingBox()->GetDiagonalLength2() == 0)
      return;

    m_WorldGeometry = geometry;
    SlicedGeometry3D* slicedWorldGeometry;
    slicedWorldGeometry = dynamic_cast<SlicedGeometry3D*>(geometry);

    PlaneGeometry::Pointer geometry2d;
    if (slicedWorldGeometry != NULL)
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
  }
  if (m_CurrentWorldPlaneGeometry.IsNull())
    itkWarningMacro("m_CurrentWorldPlaneGeometry is NULL");
}

void mitk::BaseRenderer::SetDisplayGeometry(mitk::DisplayGeometry* geometry2d)
{
  itkDebugMacro("setting DisplayGeometry to " << geometry2d);
  if (m_DisplayGeometry != geometry2d)
  {
    m_DisplayGeometry = geometry2d;
    m_DisplayGeometryData->SetPlaneGeometry(m_DisplayGeometry);
    m_DisplayGeometryUpdateTime.Modified();
    Modified();
  }
}

void mitk::BaseRenderer::SetCurrentWorldPlaneGeometry(mitk::PlaneGeometry* geometry2d)
{
  if (m_CurrentWorldPlaneGeometry != geometry2d)
  {
    m_CurrentWorldPlaneGeometry = geometry2d;
    m_CurrentWorldPlaneGeometryData->SetPlaneGeometry(m_CurrentWorldPlaneGeometry);
    m_DisplayGeometry->SetWorldGeometry(m_CurrentWorldPlaneGeometry);
    m_CurrentWorldPlaneGeometryUpdateTime.Modified();
    Modified();
  }
}

void mitk::BaseRenderer::SendUpdateSlice()
{
  m_DisplayGeometryUpdateTime.Modified();
  m_CurrentWorldPlaneGeometryUpdateTime.Modified();
}

void mitk::BaseRenderer::SetCurrentWorldGeometry(mitk::BaseGeometry* geometry)
{
  m_CurrentWorldGeometry = geometry;
  if (geometry == NULL)
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
  BoundingBox::Pointer boundingBox = m_CurrentWorldGeometry->CalculateBoundingBoxRelativeToTransform(NULL);
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
  if(m_OverlayManager.IsNotNull())
  {
    m_OverlayManager->UpdateOverlays(this);
  }
}

void mitk::BaseRenderer::SetGeometry(const itk::EventObject & geometrySendEvent)
{
  const SliceNavigationController::GeometrySendEvent* sendEvent =
      dynamic_cast<const SliceNavigationController::GeometrySendEvent *>(&geometrySendEvent);

  assert(sendEvent!=NULL);
  SetWorldTimeGeometry(sendEvent->GetTimeGeometry());
}

void mitk::BaseRenderer::UpdateGeometry(const itk::EventObject & geometryUpdateEvent)
{
  const SliceNavigationController::GeometryUpdateEvent* updateEvent =
      dynamic_cast<const SliceNavigationController::GeometryUpdateEvent*>(&geometryUpdateEvent);

  if (updateEvent == NULL)
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

  assert(sliceEvent!=NULL);
  SetSlice(sliceEvent->GetPos());
}

void mitk::BaseRenderer::SetGeometryTime(const itk::EventObject & geometryTimeEvent)
{
  const SliceNavigationController::GeometryTimeEvent * timeEvent =
      dynamic_cast<const SliceNavigationController::GeometryTimeEvent *>(&geometryTimeEvent);

  assert(timeEvent!=NULL);
  SetTimeStep(timeEvent->GetPos());
}

const double* mitk::BaseRenderer::GetBounds() const
{
  return m_Bounds;
}

void mitk::BaseRenderer::MousePressEvent(mitk::MouseEvent *me)
{
  //set the Focus on the renderer
  /*bool success =*/m_RenderingManager->GetGlobalInteraction()->SetFocus(this);
  /*
   if (! success)
   mitk::StatusBar::GetInstance()->DisplayText("Warning! from mitkBaseRenderer.cpp: Couldn't focus this BaseRenderer!");
   */

  //if (m_CameraController)
  //{
  //  if(me->GetButtonState()!=512) // provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
  //    m_CameraController->MousePressEvent(me);
  //}
  if (m_MapperID == 1)
  {
    Point2D p(me->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->GetType(), me->GetButton(), me->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event, m_RenderingManager->GetGlobalInteraction());
  }
  else if (m_MapperID > 1)  //==2 for 3D and ==5 for stencil
  {
    Point2D p(me->GetDisplayPosition());

    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    me->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(me, m_RenderingManager->GetGlobalInteraction());
  }
}

void mitk::BaseRenderer::MouseReleaseEvent(mitk::MouseEvent *me)
{
  //if (m_CameraController)
  //{
  //  if(me->GetButtonState()!=512) // provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
  //    m_CameraController->MouseReleaseEvent(me);
  //}

  if (m_MapperID == 1)
  {
    Point2D p(me->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->GetType(), me->GetButton(), me->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event, m_RenderingManager->GetGlobalInteraction());
  }
  else if (m_MapperID == 2)
  {
    Point2D p(me->GetDisplayPosition());
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    me->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(me, m_RenderingManager->GetGlobalInteraction());
  }
}

void mitk::BaseRenderer::MouseMoveEvent(mitk::MouseEvent *me)
{
  //if (m_CameraController)
  //{
  //  if((me->GetButtonState()<=512) || (me->GetButtonState()>=516))// provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
  //    m_CameraController->MouseMoveEvent(me);
  //}
  if (m_MapperID == 1)
  {
    Point2D p(me->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->GetType(), me->GetButton(), me->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event, m_RenderingManager->GetGlobalInteraction());
  }
  else if (m_MapperID == 2)
  {
    Point2D p(me->GetDisplayPosition());
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    me->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(me, m_RenderingManager->GetGlobalInteraction());
  }
}

void mitk::BaseRenderer::PickWorldPoint(const mitk::Point2D& displayPoint, mitk::Point3D& worldPoint) const
{
  mitk::Point2D worldPoint2D;
  GetDisplayGeometry()->DisplayToWorld(displayPoint, worldPoint2D);
  GetDisplayGeometry()->Map(worldPoint2D, worldPoint);
}

void mitk::BaseRenderer::WheelEvent(mitk::WheelEvent * we)
{
  if (m_MapperID == 1)
  {
    Point2D p(we->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, we->GetType(), we->GetButton(), we->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(we, m_RenderingManager->GetGlobalInteraction());
    mitk::EventMapper::MapEvent(&event, m_RenderingManager->GetGlobalInteraction());
  }
  else if (m_MapperID == 2)
  {
    Point2D p(we->GetDisplayPosition());
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    we->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(we, m_RenderingManager->GetGlobalInteraction());
  }
}

void mitk::BaseRenderer::KeyPressEvent(mitk::KeyEvent *ke)
{
  if (m_MapperID == 1)
  {
    Point2D p(ke->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::KeyEvent event(this, ke->GetType(), ke->GetButton(), ke->GetButtonState(), ke->GetKey(), ke->GetText(), p);
    mitk::EventMapper::MapEvent(&event, m_RenderingManager->GetGlobalInteraction());
  }
  else if (m_MapperID == 2)
  {
    Point2D p(ke->GetDisplayPosition());
    GetDisplayGeometry()->ULDisplayToDisplay(p, p);
    ke->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(ke, m_RenderingManager->GetGlobalInteraction());
  }
}

void mitk::BaseRenderer::DrawOverlayMouse(mitk::Point2D& itkNotUsed(p2d))
{
  MITK_INFO<<"BaseRenderer::DrawOverlayMouse()- should be inconcret implementation OpenGLRenderer."<<std::endl;
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
  if (SlicenavigationController == NULL)
    return;

  //disconnect old from globalinteraction
  m_RenderingManager->GetGlobalInteraction()->RemoveListener(SlicenavigationController);

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

/*!
 Sets the new camera controller and deletes the vtkRenderWindowInteractor in case of the VTKInteractorCameraController
 */
void mitk::BaseRenderer::SetCameraController(CameraController* cameraController)
{
  mitk::VtkInteractorCameraController::Pointer vtkInteractorCameraController =
      dynamic_cast<mitk::VtkInteractorCameraController*>(cameraController);
  if (vtkInteractorCameraController.IsNotNull())
    MITK_INFO<<"!!!WARNING!!!: RenderWindow interaction events are no longer handled via CameraController (See Bug #954)."<<std::endl;
  m_CameraController->SetRenderer(NULL);
  m_CameraController = NULL;
  m_CameraController = cameraController;
  m_CameraController->SetRenderer(this);
}

void mitk::BaseRenderer::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << " MapperID: " << m_MapperID << std::endl;
  os << indent << " Slice: " << m_Slice << std::endl;
  os << indent << " TimeStep: " << m_TimeStep << std::endl;

  os << indent << " WorldGeometry: ";
  if (m_WorldGeometry.IsNull())
    os << "NULL" << std::endl;
  else
    m_WorldGeometry->Print(os, indent);

  os << indent << " CurrentWorldPlaneGeometry: ";
  if (m_CurrentWorldPlaneGeometry.IsNull())
    os << "NULL" << std::endl;
  else
    m_CurrentWorldPlaneGeometry->Print(os, indent);

  os << indent << " CurrentWorldPlaneGeometryUpdateTime: " << m_CurrentWorldPlaneGeometryUpdateTime << std::endl;
  os << indent << " CurrentWorldPlaneGeometryTransformTime: " << m_CurrentWorldPlaneGeometryTransformTime << std::endl;

  os << indent << " DisplayGeometry: ";
  if (m_DisplayGeometry.IsNull())
    os << "NULL" << std::endl;
  else
    m_DisplayGeometry->Print(os, indent);

  os << indent << " DisplayGeometryTransformTime: " << m_DisplayGeometryTransformTime << std::endl;
  Superclass::PrintSelf(os, indent);
}
