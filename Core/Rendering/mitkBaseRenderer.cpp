#include "mitkBaseRenderer.h"
#include "mitkPlaneGeometry.h"
#include "mitkEventMapper.h"
#include "mitkPositionEvent.h"
#include "mitkGlobalInteraction.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkSmartPointerProperty.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkSliceNavigationController.h"
#include "mitkStatusBar.h"
#include "mitkInteractionConst.h"
#include <vtkTransform.h>

//##ModelId=3E3D2F120050
mitk::BaseRenderer::BaseRenderer() : 
  m_DataTreeIterator(NULL), m_RenderWindow(NULL), m_LastUpdateTime(0), m_MapperID(defaultMapper), m_CameraController(NULL), m_Focused(false), 
  m_WorldGeometry(NULL),  m_TimeSlicedWorldGeometry(NULL), m_CurrentWorldGeometry2D(NULL), m_Slice(0), m_TimeStep(0)
{
  m_Size[0] = 0;
  m_Size[1] = 0;

  //adding this BaseRenderer to the List of all BaseRenderer
  mitk::GlobalInteraction *globalInteraction = dynamic_cast<mitk::GlobalInteraction *>(EventMapper::GetGlobalStateMachine());
  if (globalInteraction != NULL)
	{
    globalInteraction->AddFocusElement(this);
  }

  SmartPointerProperty::Pointer rendererProp = new SmartPointerProperty((itk::Object*)this);

  m_CurrentWorldGeometry2D = mitk::PlaneGeometry::New();

  m_CurrentWorldGeometry2DData = mitk::Geometry2DData::New();
  m_CurrentWorldGeometry2DData->SetGeometry2D(m_CurrentWorldGeometry2D);
  m_CurrentWorldGeometry2DNode = mitk::DataTreeNode::New();
  m_CurrentWorldGeometry2DNode->SetData(m_CurrentWorldGeometry2DData);
  m_CurrentWorldGeometry2DNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_CurrentWorldGeometry2DTransformTime = m_CurrentWorldGeometry2DNode->GetVtkTransform()->GetMTime();

  m_DisplayGeometry = mitk::DisplayGeometry::New();
  m_DisplayGeometry->SetWorldGeometry(m_CurrentWorldGeometry2D);
  m_DisplayGeometryData = mitk::Geometry2DData::New();
  m_DisplayGeometryData->SetGeometry2D(m_DisplayGeometry);
  m_DisplayGeometryNode = mitk::DataTreeNode::New();
  m_DisplayGeometryNode->SetData(m_DisplayGeometryData);
  m_DisplayGeometryNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_DisplayGeometryTransformTime = m_DisplayGeometryNode->GetVtkTransform()->GetMTime();
}


//##ModelId=3E3D2F12008C
mitk::BaseRenderer::~BaseRenderer()
{
  delete m_DataTreeIterator;
}

//##ModelId=3D6A1791038B
void mitk::BaseRenderer::SetData(mitk::DataTreeIterator* iterator)
{
  if(m_DataTreeIterator != iterator)
  {
    delete m_DataTreeIterator;
    m_DataTreeIterator = NULL;

    if (iterator != NULL) {
      m_DataTreeIterator = iterator->clone();
    }
    Modified();
  }
}

//##ModelId=3E330C4D0395
const MapperSlotId mitk::BaseRenderer::defaultMapper = 1;

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
    m_CameraController->Resize(w, h);

  GetDisplayGeometry()->SetSizeInDisplayUnits(w, h);
  //@FIXME: die nächste Zeile ist nur da, weil der Anpassungsvorgang in SetSizeInDisplayUnits leider noch nicht richtig funktioniert.
  GetDisplayGeometry()->Fit();
}

//##ModelId=3E33163A0261
void mitk::BaseRenderer::InitRenderer(mitk::RenderWindow* renderwindow)
{
  m_RenderWindow = renderwindow;
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
    if(m_TimeSlicedWorldGeometry.IsNotNull())
    {
      if(m_TimeStep >= m_TimeSlicedWorldGeometry->GetTimeSteps())
        m_TimeStep = m_TimeSlicedWorldGeometry->GetTimeSteps()-1;
      SlicedGeometry3D* slicedWorldGeometry=dynamic_cast<SlicedGeometry3D*>(m_TimeSlicedWorldGeometry->GetGeometry3D(m_TimeStep));
      if(slicedWorldGeometry!=NULL)
        SetCurrentWorldGeometry2D(slicedWorldGeometry->GetGeometry2D(m_Slice));
    }
    else
      Modified();
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
        if(m_Slice >= slicedWorldGeometry->GetSlices())
          m_Slice = slicedWorldGeometry->GetSlices()-1;
      }
      assert(slicedWorldGeometry!=NULL); //only as long as the todo described in SetWorldGeometry is not done
      SetCurrentWorldGeometry2D(slicedWorldGeometry->GetGeometry2D(m_Slice)); // calls Modified()
    }
    else
    {
      SetCurrentWorldGeometry2D(dynamic_cast<Geometry2D*>(geometry));
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

  assert(sendEvent !=NULL);
  SetWorldGeometry(sendEvent ->GetTimeSlicedGeometry());
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
  mitk::GlobalInteraction *globalInteraction = dynamic_cast<mitk::GlobalInteraction *>(EventMapper::GetGlobalStateMachine());
  if (globalInteraction != NULL)
  {
    bool success = globalInteraction->SetFocus(this);
    if (! success) 
      mitk::StatusBar::DisplayText("Warning! from mitkBaseRenderer.cpp: Couldn't focus this BaseRenderer!");
  }

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
    GetDisplayGeometry()->DisplayToMM(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->GetType(), me->GetButton(), me->GetButtonState(), mitk::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event);
  }
  else if(m_MapperID==2)
  {
    //fix for strange offset in 3D coordinates!
    if (me->GetType() == Type_MouseButtonPress)
    {
      Point2D ptest(me->GetDisplayPosition());
      ptest[1] -= 24.5;
      me->SetDisplayPosition(ptest);
    }

    Point2D p(me->GetDisplayPosition());

    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
//    std::cout << "press event!" << std::endl;
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
    GetDisplayGeometry()->DisplayToMM(p, p_mm);
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
    GetDisplayGeometry()->DisplayToMM(p, p_mm);
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

void mitk::BaseRenderer::WheelEvent(mitk::WheelEvent *we)
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

//##ModelId=3EF1627503C4
void mitk::BaseRenderer::MakeCurrent()
{
}
