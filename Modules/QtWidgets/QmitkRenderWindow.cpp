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

#include "QmitkRenderWindow.h"

#include <QCursor>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "QmitkEventAdapter.h" // TODO: INTERACTION_LEGACY
#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseDoubleClickEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkMouseWheelEvent.h"
#include "mitkInternalEvent.h"

#include "QmitkRenderWindowMenu.h"

QmitkRenderWindow::QmitkRenderWindow(QWidget *parent,
    QString name,
    mitk::VtkPropRenderer* /*renderer*/,
    mitk::RenderingManager* renderingManager,mitk::BaseRenderer::RenderingMode::Type renderingMode) :
    QVTKWidget(parent), m_ResendQtEvents(true), m_MenuWidget(NULL), m_MenuWidgetActivated(false), m_LayoutIndex(0)
{
  if(renderingMode == mitk::BaseRenderer::RenderingMode::DepthPeeling)
  {
    GetRenderWindow()->SetMultiSamples(0);
    GetRenderWindow()->SetAlphaBitPlanes(1);
  }
  else if(renderingMode == mitk::BaseRenderer::RenderingMode::MultiSampling)
  {
    GetRenderWindow()->SetMultiSamples(8);
  }
  else if(renderingMode == mitk::BaseRenderer::RenderingMode::Standard)
  {
    GetRenderWindow()->SetMultiSamples(0);
  }

  Initialize(renderingManager, name.toStdString().c_str(),renderingMode); // Initialize mitkRenderWindowBase

  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);
}

QmitkRenderWindow::~QmitkRenderWindow()
{
  Destroy(); // Destroy mitkRenderWindowBase
}

void QmitkRenderWindow::SetResendQtEvents(bool resend)
{
  m_ResendQtEvents = resend;
}

void QmitkRenderWindow::SetLayoutIndex(unsigned int layoutIndex)
{
  m_LayoutIndex = layoutIndex;
  if (m_MenuWidget)
    m_MenuWidget->SetLayoutIndex(layoutIndex);
}

unsigned int QmitkRenderWindow::GetLayoutIndex()
{
  if (m_MenuWidget)
    return m_MenuWidget->GetLayoutIndex();
  else
    return 0;
}

void QmitkRenderWindow::LayoutDesignListChanged(int layoutDesignIndex)
{
  if (m_MenuWidget)
    m_MenuWidget->UpdateLayoutDesignList(layoutDesignIndex);
}

void QmitkRenderWindow::mousePressEvent(QMouseEvent *me)
{
  mitk::Point2D displayPos = GetMousePosition(me);
  mitk::Point3D worldPos = m_Renderer->Map2DRendererPositionTo3DWorldPosition(GetMousePosition(me));

  mitk::MousePressEvent::Pointer mPressEvent = mitk::MousePressEvent::New(m_Renderer,
                                                              displayPos,
                                                              worldPos,
                                                              GetButtonState(me),
                                                              GetModifiers(me), GetEventButton(me));

  if (!this->HandleEvent(mPressEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    this->mousePressMitkEvent(&myevent);
    QVTKWidget::mousePressEvent(me);
  }

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseDoubleClickEvent( QMouseEvent *me )
{
  mitk::Point2D displayPos = GetMousePosition(me);
  mitk::Point3D worldPos = m_Renderer->Map2DRendererPositionTo3DWorldPosition(GetMousePosition(me));

  mitk::MouseDoubleClickEvent::Pointer mPressEvent = mitk::MouseDoubleClickEvent::New(m_Renderer,displayPos, worldPos, GetButtonState(me),
    GetModifiers(me), GetEventButton(me));

  if (!this->HandleEvent(mPressEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    this->mousePressMitkEvent(&myevent);
    QVTKWidget::mousePressEvent(me);
  }

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me)
{
  mitk::Point2D displayPos = GetMousePosition(me);
  mitk::Point3D worldPos = m_Renderer->Map2DRendererPositionTo3DWorldPosition(GetMousePosition(me));

  mitk::MouseReleaseEvent::Pointer mReleaseEvent = mitk::MouseReleaseEvent::New(m_Renderer, displayPos,worldPos, GetButtonState(me),
      GetModifiers(me), GetEventButton(me));

  if (!this->HandleEvent(mReleaseEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    this->mouseReleaseMitkEvent(&myevent);
    QVTKWidget::mouseReleaseEvent(me);
  }

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me)
{
  mitk::Point2D displayPos = GetMousePosition(me);
  mitk::Point3D worldPos = m_Renderer->Map2DRendererPositionTo3DWorldPosition(GetMousePosition(me));

  this->AdjustRenderWindowMenuVisibility(me->pos());

  mitk::MouseMoveEvent::Pointer mMoveEvent = mitk::MouseMoveEvent::New(m_Renderer, displayPos, worldPos, GetButtonState(me),
      GetModifiers(me));

  if (!this->HandleEvent(mMoveEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    this->mouseMoveMitkEvent(&myevent);
    QVTKWidget::mouseMoveEvent(me);
  }
}

void QmitkRenderWindow::wheelEvent(QWheelEvent *we)
{
  mitk::Point2D displayPos = GetMousePosition(we);
  mitk::Point3D worldPos = m_Renderer->Map2DRendererPositionTo3DWorldPosition(GetMousePosition(we));

  mitk::MouseWheelEvent::Pointer mWheelEvent = mitk::MouseWheelEvent::New(m_Renderer, displayPos,worldPos, GetButtonState(we),
      GetModifiers(we), GetDelta(we));

  if (!this->HandleEvent(mWheelEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::WheelEvent myevent(QmitkEventAdapter::AdaptWheelEvent(m_Renderer, we));
    this->wheelMitkEvent(&myevent);
    QVTKWidget::wheelEvent(we);
  }

  if (m_ResendQtEvents)
    we->ignore();
}

void QmitkRenderWindow::keyPressEvent(QKeyEvent *ke)
{
  mitk::InteractionEvent::ModifierKeys modifiers = GetModifiers(ke);
  std::string key = GetKeyLetter(ke);

  mitk::InteractionKeyEvent::Pointer keyEvent = mitk::InteractionKeyEvent::New(m_Renderer, key, modifiers);
  if (!this->HandleEvent(keyEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    QPoint cp = mapFromGlobal(QCursor::pos());
    mitk::KeyEvent mke(QmitkEventAdapter::AdaptKeyEvent(m_Renderer, ke, cp));
    this->keyPressMitkEvent(&mke);
    ke->accept();
    QVTKWidget::keyPressEvent(ke);
  }

  if (m_ResendQtEvents)
    ke->ignore();
}

void QmitkRenderWindow::enterEvent(QEvent *e)
{
  // TODO implement new event
  QVTKWidget::enterEvent(e);
}

void QmitkRenderWindow::DeferredHideMenu()
{
  MITK_DEBUG << "QmitkRenderWindow::DeferredHideMenu";

  if (m_MenuWidget)
    m_MenuWidget->HideMenu();
}

void QmitkRenderWindow::leaveEvent(QEvent *e)
{
  mitk::InternalEvent::Pointer internalEvent = mitk::InternalEvent::New(this->m_Renderer, NULL, "LeaveRenderWindow");

  if (!this->HandleEvent(internalEvent.GetPointer()))

  if (m_MenuWidget)
    m_MenuWidget->smoothHide();

  QVTKWidget::leaveEvent(e);
}

void QmitkRenderWindow::paintEvent(QPaintEvent* /*event*/)
{
  //We are using our own interaction and thus have to call the rendering manually.
  this->GetRenderer()->GetRenderingManager()->RequestUpdate(GetRenderWindow());
}

void QmitkRenderWindow::resizeEvent(QResizeEvent* event)
{
  this->resizeMitkEvent(event->size().width(), event->size().height());

  QVTKWidget::resizeEvent(event);

  emit resized();
}

void QmitkRenderWindow::moveEvent(QMoveEvent* event)
{
  QVTKWidget::moveEvent(event);

  // after a move the overlays need to be positioned
  emit moved();
}

void QmitkRenderWindow::showEvent(QShowEvent* event)
{
  QVTKWidget::showEvent(event);

  // this singleshot is necessary to have the overlays positioned correctly after initial show
  // simple call of moved() is no use here!!
  QTimer::singleShot(0, this, SIGNAL( moved() ));
}

void QmitkRenderWindow::ActivateMenuWidget(bool state, QmitkStdMultiWidget* stdMultiWidget)
{
  m_MenuWidgetActivated = state;

  if (!m_MenuWidgetActivated && m_MenuWidget)
  {
    //disconnect Signal/Slot Connection
    disconnect(m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)));
    disconnect(m_MenuWidget, SIGNAL( ResetView() ), this, SIGNAL( ResetView()));
    disconnect(m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)));

    delete m_MenuWidget;
    m_MenuWidget = 0;
  }
  else if (m_MenuWidgetActivated && !m_MenuWidget)
  {
    //create render window MenuBar for split, close Window or set new setting.
    m_MenuWidget = new QmitkRenderWindowMenu(this, 0, m_Renderer, stdMultiWidget);
    m_MenuWidget->SetLayoutIndex(m_LayoutIndex);

    //create Signal/Slot Connection
    connect(m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)));
    connect(m_MenuWidget, SIGNAL( ResetView() ), this, SIGNAL( ResetView()));
    connect(m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)));
  }
}

void QmitkRenderWindow::AdjustRenderWindowMenuVisibility(const QPoint& /*pos*/)
{
  if (m_MenuWidget)
  {
    m_MenuWidget->ShowMenu();
    m_MenuWidget->MoveWidgetToCorrectPos(1.0f);
  }
}

void QmitkRenderWindow::HideRenderWindowMenu()
{
  // DEPRECATED METHOD
}

void QmitkRenderWindow::OnChangeLayoutDesign(int layoutDesignIndex)
{
  emit SignalLayoutDesignChanged(layoutDesignIndex);
}

void QmitkRenderWindow::OnWidgetPlaneModeChanged(int mode)
{
  if (m_MenuWidget)
    m_MenuWidget->NotifyNewWidgetPlanesMode(mode);
}

void QmitkRenderWindow::FullScreenMode(bool state)
{
  if (m_MenuWidget)
    m_MenuWidget->ChangeFullScreenMode(state);
}

void QmitkRenderWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("application/x-mitk-datanodes"))
  {
    event->accept();
  }
}

void QmitkRenderWindow::dropEvent(QDropEvent * event)
{
  if (event->mimeData()->hasFormat("application/x-mitk-datanodes"))
  {
    QString arg = QString(event->mimeData()->data("application/x-mitk-datanodes").data());
    QStringList listOfDataNodes = arg.split(",");
    std::vector<mitk::DataNode*> vectorOfDataNodePointers;

    for (int i = 0; i < listOfDataNodes.size(); i++)
    {
      size_t val = listOfDataNodes[i].toULongLong();
      mitk::DataNode* node = reinterpret_cast<mitk::DataNode *>(val);
      vectorOfDataNodePointers.push_back(node);
    }

    emit NodesDropped(this, vectorOfDataNodePointers);
  }
}

mitk::Point2D QmitkRenderWindow::GetMousePosition(QMouseEvent* me) const
{
  mitk::Point2D point;
  point[0] = me->x();
  point[1] = me->y();
  m_Renderer->GetDisplayGeometry()->ULDisplayToDisplay(point, point);
  return point;
}

mitk::Point2D QmitkRenderWindow::GetMousePosition(QWheelEvent* we) const
{
  mitk::Point2D point;
  point[0] = we->x();
  point[1] = we->y();
  m_Renderer->GetDisplayGeometry()->ULDisplayToDisplay(point, point);
  return point;
}

mitk::InteractionEvent::MouseButtons QmitkRenderWindow::GetEventButton(QMouseEvent* me) const
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

mitk::InteractionEvent::MouseButtons QmitkRenderWindow::GetButtonState(QMouseEvent* me) const
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

mitk::InteractionEvent::ModifierKeys QmitkRenderWindow::GetModifiers(QInputEvent* me) const
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

mitk::InteractionEvent::MouseButtons QmitkRenderWindow::GetButtonState(QWheelEvent* we) const
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

std::string QmitkRenderWindow::GetKeyLetter(QKeyEvent *ke) const
{
  // Converting Qt Key Event to string element.
  std::string key = "";
  int tkey = ke->key();
  if (tkey < 128)
  { //standard ascii letter
    key = (char) toupper(tkey);
  }
  else
  { // special keys
    switch (tkey)
    {
    case Qt::Key_Return:
      key = mitk::InteractionEvent::KeyReturn;
      break;
    case Qt::Key_Enter:
      key = mitk::InteractionEvent::KeyEnter;
      break;
    case Qt::Key_Escape:
      key = mitk::InteractionEvent::KeyEnter;
      break;
    case Qt::Key_Delete:
      key = mitk::InteractionEvent::KeyDelete;
      break;
    case Qt::Key_Up:
      key = mitk::InteractionEvent::KeyArrowUp;
      break;
    case Qt::Key_Down:
      key = mitk::InteractionEvent::KeyArrowDown;
      break;
    case Qt::Key_Left:
      key = mitk::InteractionEvent::KeyArrowLeft;
      break;
    case Qt::Key_Right:
      key = mitk::InteractionEvent::KeyArrowRight;
      break;

    case Qt::Key_F1:
      key = mitk::InteractionEvent::KeyF1;
      break;
    case Qt::Key_F2:
      key = mitk::InteractionEvent::KeyF2;
      break;
    case Qt::Key_F3:
      key = mitk::InteractionEvent::KeyF3;
      break;
    case Qt::Key_F4:
      key = mitk::InteractionEvent::KeyF4;
      break;
    case Qt::Key_F5:
      key = mitk::InteractionEvent::KeyF5;
      break;
    case Qt::Key_F6:
      key = mitk::InteractionEvent::KeyF6;
      break;
    case Qt::Key_F7:
      key = mitk::InteractionEvent::KeyF7;
      break;
    case Qt::Key_F8:
      key = mitk::InteractionEvent::KeyF8;
      break;
    case Qt::Key_F9:
      key = mitk::InteractionEvent::KeyF9;
      break;
    case Qt::Key_F10:
      key = mitk::InteractionEvent::KeyF10;
      break;
    case Qt::Key_F11:
      key = mitk::InteractionEvent::KeyF11;
      break;
    case Qt::Key_F12:
      key = mitk::InteractionEvent::KeyF12;
      break;

    case Qt::Key_End:
          key = mitk::InteractionEvent::KeyEnd;
          break;
    case Qt::Key_Home:
          key = mitk::InteractionEvent::KeyPos1;
          break;
    case Qt::Key_Insert:
          key = mitk::InteractionEvent::KeyInsert;
          break;
    case Qt::Key_PageDown:
          key = mitk::InteractionEvent::KeyPageDown;
          break;
    case Qt::Key_PageUp:
          key = mitk::InteractionEvent::KeyPageUp;
          break;
    case Qt::Key_Space:
          key = mitk::InteractionEvent::KeySpace;
          break;
    }
  }
  return key;
}

int QmitkRenderWindow::GetDelta(QWheelEvent* we) const
{
  return we->delta();
}