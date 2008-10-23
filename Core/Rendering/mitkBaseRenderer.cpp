/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkBaseRenderer.h"

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

// Events
#include "mitkEventMapper.h"
#include "mitkGlobalInteraction.h"
#include "mitkPositionEvent.h"
#include "mitkDisplayPositionEvent.h"

#include "mitkProperties.h"
#include "mitkWeakPointerProperty.h"

#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"
#include "mitkDataTreeStorage.h"
#include "mitkDataStorage.h"

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
   for(BaseRendererMapType::iterator mapit = baseRendererMap.begin(); 
      mapit != baseRendererMap.end(); mapit++)
  {
    if( (*mapit).first == renWin)
      return (*mapit).second;
  }
  return NULL;
}

void mitk::BaseRenderer::AddInstance(vtkRenderWindow* renWin, BaseRenderer* baseRenderer)
{
  if(renWin == NULL || baseRenderer == NULL)
    return;

  // ensure that no BaseRenderer is managed twice
  mitk::BaseRenderer::RemoveInstance(renWin);

  baseRendererMap.insert(BaseRendererMapType::value_type(renWin,baseRenderer));
}

void mitk::BaseRenderer::RemoveInstance(vtkRenderWindow* renWin)
{
  BaseRendererMapType::iterator mapit = baseRendererMap.find(renWin);
  if(mapit != baseRendererMap.end())
    baseRendererMap.erase(mapit);
}

mitk::BaseRenderer* mitk::BaseRenderer::GetByName( const std::string& name )
{
  for(BaseRendererMapType::iterator mapit = baseRendererMap.begin(); 
    mapit != baseRendererMap.end(); mapit++)
  {
    if( (*mapit).second->m_Name == name)
      return (*mapit).second;
  }
  return NULL;
}

vtkRenderWindow* mitk::BaseRenderer::GetRenderWindowByName( const std::string& name )
{
  for(BaseRendererMapType::iterator mapit = baseRendererMap.begin(); 
  mapit != baseRendererMap.end(); mapit++)
  {
    if( (*mapit).second->m_Name == name)
      return (*mapit).first;
  }
  return NULL;
}

mitk::BaseRenderer::BaseRenderer( const char* name, vtkRenderWindow * renWin ) :
  m_RenderWindow(NULL), m_VtkRenderer(NULL), m_MapperID(defaultMapper), m_DataTreeIterator(NULL),
  m_LastUpdateTime(0), m_CameraController(NULL), m_Focused(false),
  m_WorldGeometry(NULL), m_TimeSlicedWorldGeometry(NULL),
  m_CurrentWorldGeometry2D(NULL), m_Slice(0), m_TimeStep(0),
  m_EmptyWorldGeometry(true),
  m_NumberOfVisibleLODEnabledMappers( 0 )
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

  if(renWin != NULL)
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
  mitk::GlobalInteraction::GetInstance()->AddFocusElement(this);

  WeakPointerProperty::Pointer rendererProp = WeakPointerProperty::New((itk::Object*)this);

  m_CurrentWorldGeometry2D = mitk::PlaneGeometry::New();

  m_CurrentWorldGeometry2DData = mitk::Geometry2DData::New();
  m_CurrentWorldGeometry2DData->SetGeometry2D(m_CurrentWorldGeometry2D);
  m_CurrentWorldGeometry2DNode = mitk::DataTreeNode::New();
  m_CurrentWorldGeometry2DNode->SetData(m_CurrentWorldGeometry2DData);
  m_CurrentWorldGeometry2DNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_CurrentWorldGeometry2DNode->GetPropertyList()->SetProperty("layer", IntProperty::New(1000));
  m_CurrentWorldGeometry2DTransformTime = m_CurrentWorldGeometry2DNode->GetVtkTransform()->GetMTime();

  m_DisplayGeometry = mitk::DisplayGeometry::New();
  m_DisplayGeometry->SetWorldGeometry(m_CurrentWorldGeometry2D);
  m_DisplayGeometryData = mitk::Geometry2DData::New();
  m_DisplayGeometryData->SetGeometry2D(m_DisplayGeometry);
  m_DisplayGeometryNode = mitk::DataTreeNode::New();
  m_DisplayGeometryNode->SetData(m_DisplayGeometryData);
  m_DisplayGeometryNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_DisplayGeometryTransformTime = m_DisplayGeometryNode->GetVtkTransform()->GetMTime();


  m_SliceNavigationController = mitk::SliceNavigationController::New( "navigation" );
  m_SliceNavigationController->SetRenderer( this );
  m_SliceNavigationController->ConnectGeometrySliceEvent( this );
  m_SliceNavigationController->ConnectGeometryUpdateEvent( this );
  m_SliceNavigationController->ConnectGeometryTimeEvent( this, false );

  m_CameraRotationController = mitk::CameraRotationController::New();
  m_CameraRotationController->SetRenderWindow( m_RenderWindow );
  m_CameraRotationController->AcquireCamera();

//if TD Mouse Interaction is activated, then call TDMouseVtkCameraController instead of VtkInteractorCameraController
#ifdef MITK_USE_TD_MOUSE
    m_CameraController = mitk::TDMouseVtkCameraController::New();
#else
    m_CameraController = mitk::CameraController::New(NULL);
#endif

  m_VtkRenderer = vtkRenderer::New();

  if (mitk::VtkLayerController::GetInstance(m_RenderWindow) == NULL)
  {  
    mitk::VtkLayerController::AddInstance(m_RenderWindow,m_VtkRenderer);
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertSceneRenderer(m_VtkRenderer);
  }
  else
    mitk::VtkLayerController::GetInstance(m_RenderWindow)->InsertSceneRenderer(m_VtkRenderer);  
}


mitk::BaseRenderer::~BaseRenderer()
{
  if(m_VtkRenderer!=NULL)
  {
    m_VtkRenderer->Delete();
    m_VtkRenderer = NULL;
  }

  if(m_CameraController.IsNotNull())
    m_CameraController->SetRenderer(NULL);
  
  mitk::GlobalInteraction::GetInstance()->RemoveFocusElement(this);

  mitk::VtkLayerController::RemoveInstance(m_RenderWindow);
  
  this->InvokeEvent(mitk::BaseRenderer::RendererResetEvent());
  m_DataTreeIterator = NULL;

  if(m_RenderWindow!=NULL)
  {
    m_RenderWindow->Delete();
    m_RenderWindow = NULL;
  }
}

void mitk::BaseRenderer::SetData(const mitk::DataTreeIteratorBase* iterator)
{
  if(m_DataTreeIterator != iterator)
  {
    if (iterator != NULL)
      m_DataTreeIterator = iterator;
    else
      m_DataTreeIterator = NULL;
    Modified();
  }
}

void mitk::BaseRenderer::SetData(DataStorage* storage)
{
  if (storage != NULL)
  {
    DataTreePreOrderIterator poi( (dynamic_cast<mitk::DataTreeStorage*>(storage))->m_DataTree );
    this->SetData( &poi );
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

  if(m_CameraController)
    m_CameraController->Resize(w, h); //(formerly problematic on windows: vtkSizeBug)

  GetDisplayGeometry()->SetSizeInDisplayUnits(w, h);
}

void mitk::BaseRenderer::InitRenderer(vtkRenderWindow* renderwindow)
{
  if(m_RenderWindow != NULL)
  {
    m_RenderWindow->Delete();
  }
  m_RenderWindow = renderwindow;
  if(m_RenderWindow != NULL)
  {
    m_RenderWindow->Register(NULL);
  }
  this->InvokeEvent(mitk::BaseRenderer::RendererResetEvent());
  
  if(m_CameraController.IsNotNull())
  {
    m_CameraController->SetRenderer(this);
  }

  //BUG (#1551) added settings for depth peeling
#if ( ( VTK_MAJOR_VERSION >= 5 ) && ( VTK_MINOR_VERSION>=2)  )
  m_RenderWindow->SetAlphaBitPlanes(1);
  m_VtkRenderer->SetUseDepthPeeling(1);
  m_VtkRenderer->SetMaximumNumberOfPeels(100);
  m_VtkRenderer->SetOcclusionRatio(0.1);
#endif
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
  if(m_Slice!=slice)
  {
    m_Slice = slice;
    if(m_TimeSlicedWorldGeometry.IsNotNull())
    {
      SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_TimeSlicedWorldGeometry->GetGeometry3D(m_TimeStep));
      if(slicedWorldGeometry!=NULL)
      {
        if(m_Slice >= slicedWorldGeometry->GetSlices())
          m_Slice = slicedWorldGeometry->GetSlices()-1;
        SetCurrentWorldGeometry2D(slicedWorldGeometry->GetGeometry2D(m_Slice));
        SetCurrentWorldGeometry(slicedWorldGeometry);
      }
    }
    else
      Modified();
  }
}

void mitk::BaseRenderer::SetTimeStep(unsigned int timeStep)
{
  if(m_TimeStep!=timeStep)
  {
    m_TimeStep = timeStep;
    m_TimeStepUpdateTime.Modified();

    if(m_TimeSlicedWorldGeometry.IsNotNull())
    {
      if(m_TimeStep >= m_TimeSlicedWorldGeometry->GetTimeSteps())
        m_TimeStep = m_TimeSlicedWorldGeometry->GetTimeSteps()-1;
      SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_TimeSlicedWorldGeometry->GetGeometry3D(m_TimeStep));
      if(slicedWorldGeometry!=NULL)
      {
        SetCurrentWorldGeometry2D(slicedWorldGeometry->GetGeometry2D(m_Slice));
        SetCurrentWorldGeometry(slicedWorldGeometry);
      }
    }
    else
      Modified();
  }
}

int mitk::BaseRenderer::GetTimeStep(const mitk::BaseData* data) const
{
  if( (data==NULL) || (data->IsInitialized()==false) )
  {
    return -1;
  }
  return data->GetTimeSlicedGeometry()->MSToTimeStep(GetTime());
}

mitk::ScalarType mitk::BaseRenderer::GetTime() const
{
  if(m_TimeSlicedWorldGeometry.IsNull())
  {
    return 0;
  }
  else
  {
    ScalarType timeInMS = m_TimeSlicedWorldGeometry->TimeStepToMS(GetTimeStep());
    if(timeInMS == ScalarTypeNumericTraits::NonpositiveMin())
      return 0;
    else
      return timeInMS;
  }
}

void mitk::BaseRenderer::SetWorldGeometry(mitk::Geometry3D* geometry)
{
  itkDebugMacro("setting WorldGeometry to " << geometry);

  if(m_WorldGeometry != geometry)
  {
    if(geometry->GetBoundingBox()->GetDiagonalLength2() == 0)
      return;
    
    m_WorldGeometry = geometry;
    m_TimeSlicedWorldGeometry=dynamic_cast<TimeSlicedGeometry*>(geometry);
    SlicedGeometry3D* slicedWorldGeometry;
    if(m_TimeSlicedWorldGeometry.IsNotNull())
    {
      itkDebugMacro("setting TimeSlicedWorldGeometry to " << m_TimeSlicedWorldGeometry);
      if(m_TimeStep >= m_TimeSlicedWorldGeometry->GetTimeSteps())
        m_TimeStep = m_TimeSlicedWorldGeometry->GetTimeSteps()-1;
      slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_TimeSlicedWorldGeometry->GetGeometry3D(m_TimeStep));
    }
    else
    {
      slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(geometry);
    }
    Geometry2D::Pointer geometry2d;
    if(slicedWorldGeometry!=NULL)
    {
      if(m_Slice >= slicedWorldGeometry->GetSlices() && (m_Slice != 0))
        m_Slice = slicedWorldGeometry->GetSlices()-1;
      geometry2d = slicedWorldGeometry->GetGeometry2D(m_Slice);
      if(geometry2d.IsNull())
      {
        PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
        plane->InitializeStandardPlane(slicedWorldGeometry);
        geometry2d = plane;
      }
      SetCurrentWorldGeometry(slicedWorldGeometry);
    }
    else
    {
      geometry2d=dynamic_cast<Geometry2D*>(geometry);
      if(geometry2d.IsNull())
      {
        PlaneGeometry::Pointer plane = PlaneGeometry::New();
        plane->InitializeStandardPlane(geometry);
        geometry2d = plane;
      }
      SetCurrentWorldGeometry(geometry);
    }
    SetCurrentWorldGeometry2D(geometry2d); // calls Modified()
  }
  if(m_CurrentWorldGeometry2D.IsNull())
    itkWarningMacro("m_CurrentWorldGeometry2D is NULL");
}

void mitk::BaseRenderer::SetDisplayGeometry(mitk::DisplayGeometry* geometry2d)
{
  itkDebugMacro("setting DisplayGeometry to " << geometry2d);
  if (m_DisplayGeometry != geometry2d)
  {
    m_DisplayGeometry = geometry2d;
    m_DisplayGeometryData->SetGeometry2D(m_DisplayGeometry);
    m_DisplayGeometryUpdateTime.Modified();
    Modified();
  }
}

void mitk::BaseRenderer::SetCurrentWorldGeometry2D(mitk::Geometry2D* geometry2d)
{
  if (m_CurrentWorldGeometry2D != geometry2d)
  {
    m_CurrentWorldGeometry2D = geometry2d;
    m_CurrentWorldGeometry2DData->SetGeometry2D(m_CurrentWorldGeometry2D);
    m_DisplayGeometry->SetWorldGeometry(m_CurrentWorldGeometry2D);
    m_CurrentWorldGeometry2DUpdateTime.Modified();
    Modified();
  }
}

void mitk::BaseRenderer::SetCurrentWorldGeometry(mitk::Geometry3D* geometry)
{
  m_CurrentWorldGeometry = geometry;
  if(geometry == NULL)
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
  BoundingBox::Pointer boundingBox = 
    m_CurrentWorldGeometry->CalculateBoundingBoxRelativeToTransform(NULL);
  const BoundingBox::BoundsArrayType& worldBounds = boundingBox->GetBounds();
  m_Bounds[0] = worldBounds[0];
  m_Bounds[1] = worldBounds[1];
  m_Bounds[2] = worldBounds[2];
  m_Bounds[3] = worldBounds[3];
  m_Bounds[4] = worldBounds[4];
  m_Bounds[5] = worldBounds[5];
  if(boundingBox->GetDiagonalLength2()<=mitk::eps)
    m_EmptyWorldGeometry = true;
  else
    m_EmptyWorldGeometry = false;
}

void mitk::BaseRenderer::SetGeometry(const itk::EventObject & geometrySendEvent)
{
  const SliceNavigationController::GeometrySendEvent* sendEvent =
    dynamic_cast<const SliceNavigationController::GeometrySendEvent *>(&geometrySendEvent);

  assert(sendEvent!=NULL);
  SetWorldGeometry(sendEvent ->GetTimeSlicedGeometry());
}

void mitk::BaseRenderer::UpdateGeometry(const itk::EventObject & geometryUpdateEvent)
{
  const SliceNavigationController::GeometryUpdateEvent* updateEvent =
    dynamic_cast<const SliceNavigationController::GeometryUpdateEvent*>(&geometryUpdateEvent);

  if (updateEvent==NULL) return;

  if (m_CurrentWorldGeometry.IsNotNull())
  {
    SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_CurrentWorldGeometry.GetPointer());
    if (slicedWorldGeometry)
    {
      Geometry2D* geometry2D = slicedWorldGeometry->GetGeometry2D(m_Slice);
    
      SetCurrentWorldGeometry2D(geometry2D); // calls Modified()
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
  bool success = mitk::GlobalInteraction::GetInstance()->SetFocus(this);
  if (! success) 
    mitk::StatusBar::GetInstance()->DisplayText("Warning! from mitkBaseRenderer.cpp: Couldn't focus this BaseRenderer!");

  //if (m_CameraController)
  //{
  //  if(me->GetButtonState()!=512) // provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
  //    m_CameraController->MousePressEvent(me);
  //}
  if(m_MapperID==1)
  {
    Point2D p(me->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->GetType(), me->GetButton(), me->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event);
  }
  else if(m_MapperID>1)//==2 for 3D and ==5 for stencil
  {
    Point2D p(me->GetDisplayPosition());

    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    me->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(me);
  }
}

void mitk::BaseRenderer::MouseReleaseEvent(mitk::MouseEvent *me)
{

  //if (m_CameraController)
  //{
  //  if(me->GetButtonState()!=512) // provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
  //    m_CameraController->MouseReleaseEvent(me);
  //}

  if(m_MapperID==1)
  {
    Point2D p(me->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->GetType(), me->GetButton(), me->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event);
  }
  else if(m_MapperID==2)
  {
    Point2D p(me->GetDisplayPosition());
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    me->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(me);
  }
}

void mitk::BaseRenderer::MouseMoveEvent(mitk::MouseEvent *me)
{
  //if (m_CameraController)
  //{
  //  if((me->GetButtonState()<=512) || (me->GetButtonState()>=516))// provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
  //    m_CameraController->MouseMoveEvent(me);
  //}
  if(m_MapperID==1)
  {
    Point2D p(me->GetDisplayPosition());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    GetDisplayGeometry()->DisplayToWorld(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->GetType(), me->GetButton(), me->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event);
  }
  else if(m_MapperID==2)
  {
    Point2D p(me->GetDisplayPosition());
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    me->SetDisplayPosition(p);
    mitk::EventMapper::MapEvent(me);
  }
}

void mitk::BaseRenderer::PickWorldPoint(const mitk::Point2D& displayPoint, mitk::Point3D& worldPoint) const
{
  mitk::Point2D worldPoint2D;
  GetDisplayGeometry()->DisplayToWorld(displayPoint, worldPoint2D);
  GetDisplayGeometry()->Map(worldPoint2D, worldPoint);
}

void mitk::BaseRenderer::WheelEvent(mitk::WheelEvent *)
{
  //mitk::Event event(this, ke->type(), Qt::NoButton, Qt::NoButton, ke->key());
  //mitk::EventMapper::MapEvent(&event);
}

void mitk::BaseRenderer::KeyPressEvent(mitk::KeyEvent *ke)
{
  //if (m_CameraController)
  //  m_CameraController->KeyPressEvent(ke);
  mitk::Event event(this, ke->type(), BS_NoButton, BS_NoButton, ke->key());
  mitk::EventMapper::MapEvent(&event);
}

void mitk::BaseRenderer::DrawOverlayMouse(mitk::Point2D& itkNotUsed(p2d))
{
  std::cout<<"BaseRenderer::DrawOverlayMouse()- should be inconcret implementation OpenGLRenderer."<<std::endl;
}

void mitk::BaseRenderer::RequestUpdate()
{
  mitk::RenderingManager::GetInstance()->RequestUpdate(this->m_RenderWindow);
}

void mitk::BaseRenderer::ForceImmediateUpdate()
{
  mitk::RenderingManager::GetInstance()->ForceImmediateUpdate(this->m_RenderWindow);
}


unsigned int mitk::BaseRenderer::GetNumberOfVisibleLODEnabledMappers() const
{
  return m_NumberOfVisibleLODEnabledMappers;
}



/*!
Sets the new camera controller and deletes the vtkRenderWindowInteractor in case of the VTKInteractorCameraController
*/
void mitk::BaseRenderer::SetCameraController(CameraController* cameraController)
{
  mitk::VtkInteractorCameraController::Pointer vtkInteractorCameraController = dynamic_cast<mitk::VtkInteractorCameraController*>(cameraController);
  if (vtkInteractorCameraController.IsNotNull())
    std::cout<<"!!!WARNING!!!: RenderWindow interaction events are no longer handled via CameraController (See Bug #954)."<<std::endl;
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
  if(m_WorldGeometry.IsNull())
    os << "NULL" << std::endl;
  else
    m_WorldGeometry->Print(os, indent);

  os << indent << " CurrentWorldGeometry2D: ";
  if(m_CurrentWorldGeometry2D.IsNull())
    os << "NULL" << std::endl;
  else
    m_CurrentWorldGeometry2D->Print(os, indent);

  os << indent << " CurrentWorldGeometry2DUpdateTime: " << m_CurrentWorldGeometry2DUpdateTime << std::endl;
  os << indent << " CurrentWorldGeometry2DTransformTime: " << m_CurrentWorldGeometry2DTransformTime << std::endl;

  os << indent << " DisplayGeometry: ";
  if(m_DisplayGeometry.IsNull())
    os << "NULL" << std::endl;
  else
    m_DisplayGeometry->Print(os, indent);

  os << indent << " DisplayGeometryTransformTime: " << m_DisplayGeometryTransformTime << std::endl;
  Superclass::PrintSelf(os,indent);
}
