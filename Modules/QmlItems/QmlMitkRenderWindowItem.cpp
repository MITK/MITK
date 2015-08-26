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

#include "QmlMitkRenderWindowItem.h"

#include <vtkOpenGLExtensionManager.h>

#include <QVTKInteractor.h>
#include<vtkEventQtSlotConnect.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKInteractorAdapter.h>

// MITK event types
#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseDoubleClickEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkMouseWheelEvent.h"
#include "mitkInternalEvent.h"
#include "mitkPlaneGeometryDataMapper2D.h"

#include "QmlMitkBigRenderLock.h"


QmlMitkRenderWindowItem* QmlMitkRenderWindowItem::GetInstanceForVTKRenderWindow( vtkRenderWindow* rw )
{
  return GetInstances()[rw];
}

QMap<vtkRenderWindow*, QmlMitkRenderWindowItem*>& QmlMitkRenderWindowItem::GetInstances()
{
  static QMap<vtkRenderWindow*, QmlMitkRenderWindowItem*> s_Instances;
  return s_Instances;
}


QmlMitkRenderWindowItem
::QmlMitkRenderWindowItem(QQuickItem* parent,
                          const QString& name,
                          mitk::VtkPropRenderer* /*renderer*/,
                          mitk::RenderingManager* renderingManager)
: QVTKQuickItem(parent)
{
  mitk::RenderWindowBase::Initialize( renderingManager, name.toStdString().c_str() );

  /* from QmitkRenderWindow. Required?
  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
  */

  GetInstances()[QVTKQuickItem::GetRenderWindow()] = this;
}

// called from QVTKQuickItem when window is painted for the first time!
void QmlMitkRenderWindowItem::init()
{
  QVTKQuickItem::init();

  mitk::DataStorage::Pointer m_DataStorage = mitk::RenderWindowBase::GetRenderer()->GetDataStorage();
  if (m_DataStorage.IsNotNull())
  {
    mitk::RenderingManager::GetInstance()->InitializeViews( m_DataStorage->ComputeBoundingGeometry3D(m_DataStorage->GetAll()) );
  }


  if ( mitk::RenderWindowBase::GetRenderer()->GetMapperID() == mitk::BaseRenderer::Standard2D
      && m_DataStorage.IsNotNull() )
  {

    mitk::DataNode::Pointer planeNode = mitk::RenderWindowBase::GetRenderer()->GetCurrentWorldGeometry2DNode();
    switch ( mitk::RenderWindowBase::GetRenderer()->GetSliceNavigationController()->GetDefaultViewDirection() )
    {
      case mitk::SliceNavigationController::Axial:
        planeNode->SetColor(1.0,0.0,0.0);
        break;
      case mitk::SliceNavigationController::Sagittal:
        planeNode->SetColor(0.0,1.0,0.0);
        break;
      case mitk::SliceNavigationController::Frontal:
        planeNode->SetColor(0.0,0.0,1.0);
        break;
      default:
        planeNode->SetColor(1.0,1.0,0.0);
    }
    planeNode->SetProperty("layer", mitk::IntProperty::New(1000) );
    planeNode->SetProperty("visible", mitk::BoolProperty::New(true) );
    planeNode->SetProperty("helper object", mitk::BoolProperty::New(true) );

    mitk::PlaneGeometryDataMapper2D::Pointer mapper = mitk::PlaneGeometryDataMapper2D::New();
    planeNode->SetMapper( mitk::BaseRenderer::Standard2D, mapper );

    m_DataStorage->Add( planeNode );
  }
}

void QmlMitkRenderWindowItem::InitView( mitk::BaseRenderer::MapperSlotId mapperID,
                                        mitk::SliceNavigationController::ViewDirection viewDirection )
{
  m_MapperID = mapperID;
  m_ViewDirection = viewDirection;
}


void QmlMitkRenderWindowItem::SetDataStorage(mitk::DataStorage::Pointer storage)
{
  m_DataStorage = storage;
}

mitk::Point2D QmlMitkRenderWindowItem::GetMousePosition(QMouseEvent* me) const
{

  mitk::Point2D point;
  point[0] = me->x();
  point[1] = me->y();
  return point;
}

mitk::Point2D QmlMitkRenderWindowItem::GetMousePosition(QWheelEvent* we) const
{
  mitk::Point2D point;
  point[0] = we->x();
  point[1] = we->y();
  return point;
}

mitk::InteractionEvent::MouseButtons QmlMitkRenderWindowItem::GetEventButton(QMouseEvent* me) const
{
  mitk::InteractionEvent::MouseButtons eventButton;
  switch (me->button())
  {
  case Qt::LeftButton:
    eventButton = mitk::InteractionEvent::LeftMouseButton;
    break;
  case Qt::RightButton:
    eventButton = mitk::InteractionEvent::RightMouseButton;
    break;
  case Qt::MidButton:
    eventButton = mitk::InteractionEvent::MiddleMouseButton;
    break;
  default:
    eventButton = mitk::InteractionEvent::NoButton;
    break;
  }
  return eventButton;
}

mitk::InteractionEvent::MouseButtons QmlMitkRenderWindowItem::GetButtonState(QMouseEvent* me) const
{
  mitk::InteractionEvent::MouseButtons buttonState = mitk::InteractionEvent::NoButton;

  if (me->buttons() & Qt::LeftButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::LeftMouseButton;
  }
  if (me->buttons() & Qt::RightButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::RightMouseButton;
  }
  if (me->buttons() & Qt::MidButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::MiddleMouseButton;
  }
  return buttonState;
}

mitk::InteractionEvent::ModifierKeys QmlMitkRenderWindowItem::GetModifiers(QInputEvent* me) const
{
  mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::NoKey;

  if (me->modifiers() & Qt::ALT)
  {
    modifiers = modifiers | mitk::InteractionEvent::AltKey;
  }
  if (me->modifiers() & Qt::CTRL)
  {
    modifiers = modifiers | mitk::InteractionEvent::ControlKey;
  }
  if (me->modifiers() & Qt::SHIFT)
  {
    modifiers = modifiers | mitk::InteractionEvent::ShiftKey;
  }
  return modifiers;
}

mitk::InteractionEvent::MouseButtons QmlMitkRenderWindowItem::GetButtonState(QWheelEvent* we) const
{
  mitk::InteractionEvent::MouseButtons buttonState = mitk::InteractionEvent::NoButton;

  if (we->buttons() & Qt::LeftButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::LeftMouseButton;
  }
  if (we->buttons() & Qt::RightButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::RightMouseButton;
  }
  if (we->buttons() & Qt::MidButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::MiddleMouseButton;
  }
  return buttonState;
}


void QmlMitkRenderWindowItem::mousePressEvent(QMouseEvent* me)
{
  mitk::Point2D mousePosition = GetMousePosition(me);
  mitk::MousePressEvent::Pointer mPressEvent =
    mitk::MousePressEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition,  GetButtonState(me), GetModifiers(me), GetEventButton(me));

  mitk::RenderWindowBase::HandleEvent(mPressEvent.GetPointer());

  QVTKQuickItem::mousePressEvent(me);

//  if (m_ResendQtEvents)
//    me->ignore();
}

void QmlMitkRenderWindowItem::mouseReleaseEvent(QMouseEvent* me)
{
  mitk::Point2D mousePosition = GetMousePosition(me);

  mitk::MouseReleaseEvent::Pointer mReleaseEvent =
    mitk::MouseReleaseEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition, GetButtonState(me), GetModifiers(me), GetEventButton(me));

  mitk::RenderWindowBase::HandleEvent(mReleaseEvent.GetPointer());


  QVTKQuickItem::mouseReleaseEvent(me);

//  if (m_ResendQtEvents)
//    me->ignore();
}

void QmlMitkRenderWindowItem::mouseMoveEvent(QMouseEvent* me)
{
  mitk::Point2D mousePosition = GetMousePosition(me);
  mitk::MouseMoveEvent::Pointer mMoveEvent =
    mitk::MouseMoveEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition, GetButtonState(me), GetModifiers(me));

  mitk::RenderWindowBase::HandleEvent(mMoveEvent.GetPointer());


  QVTKQuickItem::mouseMoveEvent(me);

// TODO: why was this not put here initially? What is special about mouse move?
//  if (m_ResendQtEvents)
//    me->ignore();
}

void QmlMitkRenderWindowItem::wheelEvent(QWheelEvent *we)
{
  mitk::Point2D mousePosition = GetMousePosition(we);
  mitk::MouseWheelEvent::Pointer mWheelEvent =
    mitk::MouseWheelEvent::New(mitk::RenderWindowBase::GetRenderer(), mousePosition, GetButtonState(we), GetModifiers(we), we->delta());

  mitk::RenderWindowBase::HandleEvent(mWheelEvent.GetPointer());


  QVTKQuickItem::wheelEvent(we);

//  if (m_ResendQtEvents)
//    we->ignore();
}


void QmlMitkRenderWindowItem::prepareForRender()
{
//  Adjust camera is kaputt wenn nicht der renderingmanager dem vtkprop bescheid sagt!
//  this is just a workaround
  QmlMitkBigRenderLock::GetMutex().lock();
  mitk::RenderWindowBase::GetRenderer()->ForceImmediateUpdate();
}

void QmlMitkRenderWindowItem::cleanupAfterRender()
{
  QmlMitkBigRenderLock::GetMutex().unlock();
}


void QmlMitkRenderWindowItem::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
  QVTKQuickItem::geometryChanged(newGeometry, oldGeometry);
}

vtkRenderWindow* QmlMitkRenderWindowItem::GetVtkRenderWindow()
{
  return QVTKQuickItem::GetRenderWindow();
}

vtkRenderWindowInteractor* QmlMitkRenderWindowItem::GetVtkRenderWindowInteractor()
{
  return QVTKQuickItem::GetInteractor();
}


