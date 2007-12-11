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
#include "mitkRenderWindow.h"

#include "mitkPlaneGeometry.h"
#include "mitkSlicedGeometry3D.h"

#include "mitkSliceNavigationController.h"

#include "mitkEventMapper.h"
#include "mitkGlobalInteraction.h"
#include "mitkPositionEvent.h"
#include "mitkDisplayPositionEvent.h"

#include "mitkProperties.h"
#include "mitkWeakPointerProperty.h"

#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"
#include "mitkCameraController.h"

#include <vtkLinearTransform.h>


mitk::BaseRenderer::RendererSet mitk::BaseRenderer::instances;


//##ModelId=3E3D2F120050
mitk::BaseRenderer::BaseRenderer( const char* name ) :
  m_MapperID(defaultMapper), m_DataTreeIterator(NULL), m_RenderWindow(NULL),
  m_LastUpdateTime(0), m_CameraController(NULL), m_Focused(false),
  m_WorldGeometry(NULL), m_TimeSlicedWorldGeometry(NULL),
  m_CurrentWorldGeometry2D(NULL), m_Slice(0), m_TimeStep(0)
{
  if (name != NULL)
  {
    m_Name = name;
  }
  else
  {
    m_Name = "unnamed renderer";
    itkWarningMacro(<< "Created unnamed renderer. Bad for serialization. Please choose a name.");
  }

  m_Size[0] = 0;
  m_Size[1] = 0;

  instances.insert( this );

  //adding this BaseRenderer to the List of all BaseRenderer
  mitk::GlobalInteraction::GetInstance()->AddFocusElement(this);

  WeakPointerProperty::Pointer rendererProp = new WeakPointerProperty((itk::Object*)this);

  m_CurrentWorldGeometry2D = mitk::PlaneGeometry::New();

  m_CurrentWorldGeometry2DData = mitk::Geometry2DData::New();
  m_CurrentWorldGeometry2DData->SetGeometry2D(m_CurrentWorldGeometry2D);
  m_CurrentWorldGeometry2DNode = mitk::DataTreeNode::New();
  m_CurrentWorldGeometry2DNode->SetData(m_CurrentWorldGeometry2DData);
  m_CurrentWorldGeometry2DNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_CurrentWorldGeometry2DNode->GetPropertyList()->SetProperty("layer", new IntProperty(1000));
  m_CurrentWorldGeometry2DTransformTime = m_CurrentWorldGeometry2DNode->GetVtkTransform()->GetMTime();

  m_DisplayGeometry = mitk::DisplayGeometry::New();
  m_DisplayGeometry->SetWorldGeometry(m_CurrentWorldGeometry2D);
  m_DisplayGeometryData = mitk::Geometry2DData::New();
  m_DisplayGeometryData->SetGeometry2D(m_DisplayGeometry);
  m_DisplayGeometryNode = mitk::DataTreeNode::New();
  m_DisplayGeometryNode->SetData(m_DisplayGeometryData);
  m_DisplayGeometryNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_DisplayGeometryTransformTime = m_DisplayGeometryNode->GetVtkTransform()->GetMTime();

  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();
}


//##ModelId=3E3D2F12008C
mitk::BaseRenderer::~BaseRenderer()
{
  if(m_CameraController.IsNotNull())
    m_CameraController->SetRenderer(NULL);
  this->InvokeEvent(mitk::BaseRenderer::RendererResetEvent());
  RendererSet::iterator pos = instances.find( this );
  instances.erase( pos );
  m_DataTreeIterator = NULL;
}

//##ModelId=3D6A1791038B
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

//##ModelId=3E330C4D0395
const mitk::BaseRenderer::MapperSlotId mitk::BaseRenderer::defaultMapper = 1;

//##ModelId=3E33162C00D0
void mitk::BaseRenderer::Paint()
{
}

//##ModelId=3E331632031E
void mitk::BaseRenderer::Initialize()
{
}

//##ModelId=3E33163703D9
void mitk::BaseRenderer::Resize(int w, int h)
{
  m_Size[0] = w;
  m_Size[1] = h;

  if(m_CameraController)
    m_CameraController->Resize(w, h); //(formerly problematic on windows: vtkSizeBug)

  GetDisplayGeometry()->SetSizeInDisplayUnits(w, h);
}

//##ModelId=3E33163A0261
void mitk::BaseRenderer::InitRenderer(mitk::RenderWindow* renderwindow)
{
  m_RenderWindow = renderwindow;
  this->InvokeEvent(mitk::BaseRenderer::RendererResetEvent());
}

//##ModelId=3E3799250397
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
        m_CurrentWorldGeometry = slicedWorldGeometry;
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
        m_CurrentWorldGeometry = slicedWorldGeometry;
      }
    }
    else
      Modified();
  }
}

int mitk::BaseRenderer::GetTimeStep(mitk::BaseData* data) const
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

//##ModelId=3E66CC590379
void mitk::BaseRenderer::SetWorldGeometry(mitk::Geometry3D* geometry)
{
  itkDebugMacro("setting WorldGeometry to " << geometry);

  if(m_WorldGeometry != geometry)
  {
    m_WorldGeometry = geometry;
    m_TimeSlicedWorldGeometry=dynamic_cast<TimeSlicedGeometry*>(geometry);
    if(m_TimeSlicedWorldGeometry.IsNotNull())
    {
      itkDebugMacro("setting TimeSlicedWorldGeometry to " << m_TimeSlicedWorldGeometry);
      if(m_TimeStep >= m_TimeSlicedWorldGeometry->GetTimeSteps())
        m_TimeStep = m_TimeSlicedWorldGeometry->GetTimeSteps()-1;
      SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_TimeSlicedWorldGeometry->GetGeometry3D(m_TimeStep));
      if(slicedWorldGeometry!=NULL)
      {
        if(m_Slice >= slicedWorldGeometry->GetSlices() && (m_Slice != 0))
          m_Slice = slicedWorldGeometry->GetSlices()-1;
      }
      assert(slicedWorldGeometry!=NULL); //only as long as the todo described in SetWorldGeometry is not done
      Geometry2D::Pointer geometry2d = slicedWorldGeometry->GetGeometry2D(m_Slice);
      if(geometry2d.IsNull())
      {
        PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
        plane->InitializeStandardPlane(slicedWorldGeometry);
        geometry2d = plane;
      }
      SetCurrentWorldGeometry2D(geometry2d); // calls Modified()
      m_CurrentWorldGeometry = slicedWorldGeometry;
    }
    else
    {
      Geometry2D* geometry2d=dynamic_cast<Geometry2D*>(geometry);
      SetCurrentWorldGeometry2D(geometry2d);
      m_CurrentWorldGeometry = geometry2d;
      Modified();
    }
  }
  if(m_CurrentWorldGeometry2D.IsNull())
    itkWarningMacro("m_CurrentWorldGeometry2D is NULL");
}

//##ModelId=3E66CC59026B
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

//##ModelId=3E6D5DD30322
void mitk::BaseRenderer::MousePressEvent(mitk::MouseEvent *me)
{
  //set the Focus on the renderer
  bool success = mitk::GlobalInteraction::GetInstance()->SetFocus(this);
  if (! success) 
    mitk::StatusBar::GetInstance()->DisplayText("Warning! from mitkBaseRenderer.cpp: Couldn't focus this BaseRenderer!");

  if (m_CameraController)
  {
    if(me->GetButtonState()!=512) // provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
      m_CameraController->MousePressEvent(me);
  }
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

//##ModelId=3E6D5DD30372
void mitk::BaseRenderer::MouseReleaseEvent(mitk::MouseEvent *me)
{
  if (m_CameraController)
  {
    if(me->GetButtonState()!=512) // provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
      m_CameraController->MouseReleaseEvent(me);
  }

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

//##ModelId=3E6D5DD303C2
void mitk::BaseRenderer::MouseMoveEvent(mitk::MouseEvent *me)
{
  if (m_CameraController)
  {
    if((me->GetButtonState()<=512) || (me->GetButtonState()>=516))// provisorisch: Ctrl nicht durchlassen. Bald wird aus m_CameraController eine StateMachine
      m_CameraController->MouseMoveEvent(me);
  }
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

//##ModelId=3E6D5DD4002A
void mitk::BaseRenderer::KeyPressEvent(mitk::KeyEvent *ke)
{
  if (m_CameraController)
    m_CameraController->KeyPressEvent(ke);
  mitk::Event event(this, ke->type(), BS_NoButton, BS_NoButton, ke->key());
  mitk::EventMapper::MapEvent(&event);
}

void mitk::BaseRenderer::Render( bool drawOverlayOnly )
{
  // Initialize the render context and do the rendering, if the RenderWindow
  // allows for it.
  if ( m_RenderWindow && m_RenderWindow->PrepareRendering() )
  {
    this->Repaint(drawOverlayOnly );
  }
}

//##ModelId=3EF1627503C4
void mitk::BaseRenderer::MakeCurrent()
{
}


void mitk::BaseRenderer::DrawOverlayMouse(mitk::Point2D& itkNotUsed(p2d))
{
  std::cout<<"BaseRenderer::DrawOverlayMouse()- should be inconcret implementation OpenGLRenderer."<<std::endl;
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
