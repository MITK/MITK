#include "BaseRenderer.h"
#include "PlaneGeometry.h"
#include "EventMapper.h"
#include "PositionEvent.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkSmartPointerProperty.h"
#include <vtkTransform.h>

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

//##ModelId=3E3314720003
void mitk::BaseRenderer::SetWindowId(void *id)
{
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
  GetDisplayGeometry()->SetSizeInDisplayUnits(w, h, false);
  GetDisplayGeometry()->Fit();
}

//##ModelId=3E3D2F120050
mitk::BaseRenderer::BaseRenderer() : m_DataTreeIterator(NULL), m_RenderWindow(NULL), m_LastUpdateTime(0), m_MapperID(defaultMapper), m_CameraController(NULL), m_Focused(false)
{
  SmartPointerProperty::Pointer rendererProp = new SmartPointerProperty((itk::Object*)this);

  m_WorldGeometry = mitk::PlaneGeometry::New();

  m_WorldGeometryData = mitk::Geometry2DData::New();
  m_WorldGeometryData->SetGeometry2D(m_WorldGeometry);
  m_WorldGeometryNode = mitk::DataTreeNode::New();
  m_WorldGeometryNode->SetData(m_WorldGeometryData);
  m_WorldGeometryNode->GetPropertyList()->SetProperty("renderer", rendererProp);
  m_WorldGeometryTransformTime = m_WorldGeometryNode->GetVtkTransform()->GetMTime();

  m_DisplayGeometry = mitk::DisplayGeometry::New();
  m_DisplayGeometry->SetWorldGeometry(m_WorldGeometry);
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

//##ModelId=3E66CC590379
void mitk::BaseRenderer::SetWorldGeometry(const mitk::Geometry2D* geometry2d)
{
  itkDebugMacro("setting WorldGeometry to " << geometry2d);
  if (m_WorldGeometry != geometry2d)
  {
    m_WorldGeometry = geometry2d;
    m_WorldGeometryData->SetGeometry2D(m_WorldGeometry);
    m_DisplayGeometry->SetWorldGeometry(m_WorldGeometry);
    Modified();
  }
}

//##ModelId=3E66CC59026B
void mitk::BaseRenderer::SetDisplayGeometry(mitk::DisplayGeometry* geometry2d)
{
  itkDebugMacro("setting DisplayGeometry to " << geometry2d);
  if (m_DisplayGeometry != geometry2d)
  {
    m_DisplayGeometry = geometry2d;
    m_DisplayGeometryData->SetGeometry2D(m_DisplayGeometry);
    Modified();
  }
}

//##ModelId=3E6D5DD30322
void mitk::BaseRenderer::MousePressEvent(mitk::MouseEvent *me)
{
  if (m_CameraController)
    m_CameraController->MousePressEvent(me);
  if(m_MapperID==1)
  {
    Point2D p(me->x(), me->y());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    GetDisplayGeometry()->DisplayToMM(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->type(), me->button(), me->state(), Qt::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event);
  }
  else if(m_MapperID==2)
  {
    Point2D p(me->x(), me->y());
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    std::cout << "press event!" << std::endl;
    mitk::DisplayPositionEvent event(this, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    mitk::EventMapper::MapEvent(&event);
  }
}

//##ModelId=3E6D5DD30372
void mitk::BaseRenderer::MouseReleaseEvent(mitk::MouseEvent *me)
{
  if (m_CameraController)
    m_CameraController->MouseReleaseEvent(me);

  if(m_MapperID==1)
  {
    Point2D p(me->x(), me->y());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    GetDisplayGeometry()->DisplayToMM(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->type(), me->button(), me->state(), Qt::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event);
  }
  else if(m_MapperID==2)
  {
    Point2D p(me->x(), me->y());
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    mitk::DisplayPositionEvent event(this, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    mitk::EventMapper::MapEvent(&event);
  }
}

//##ModelId=3E6D5DD303C2
void mitk::BaseRenderer::MouseMoveEvent(mitk::MouseEvent *me)
{
  if (m_CameraController)
    m_CameraController->MouseMoveEvent(me);
  if(m_MapperID==1)
  {
    Point2D p(me->x(), me->y());
    Point2D p_mm;
    Point3D position;
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    GetDisplayGeometry()->DisplayToMM(p, p_mm);
    GetDisplayGeometry()->Map(p_mm, position);
    mitk::PositionEvent event(this, me->type(), me->button(), me->state(), Qt::Key_unknown, p, position);
    mitk::EventMapper::MapEvent(&event);
  }
  else if(m_MapperID==2)
  {
    Point2D p(me->x(), me->y());
    GetDisplayGeometry()->ULDisplayToDisplay(p,p);
    mitk::DisplayPositionEvent event(this, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    mitk::EventMapper::MapEvent(&event);
  }
}

//##ModelId=3E6D5DD4002A
void mitk::BaseRenderer::KeyPressEvent(mitk::KeyEvent *ke)
{
  if (m_CameraController)
    m_CameraController->KeyPressEvent(ke);
  mitk::Event event(this, ke->type(), Qt::NoButton, Qt::NoButton, ke->key());
  mitk::EventMapper::MapEvent(&event);
}

//##ModelId=3EF1627503C4
void mitk::BaseRenderer::MakeCurrent()
{
}
