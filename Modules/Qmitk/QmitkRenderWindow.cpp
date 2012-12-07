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
#include "mitkMouseReleaseEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkMouseWheelEvent.h"

#include "QmitkRenderWindowMenu.h"

QmitkRenderWindow::QmitkRenderWindow(QWidget *parent,
    QString name,
    mitk::VtkPropRenderer* /*renderer*/,
    mitk::RenderingManager* renderingManager) :
    QVTKWidget(parent), m_ResendQtEvents(true), m_MenuWidget(NULL), m_MenuWidgetActivated(false), m_LayoutIndex(0)
{
  Initialize(renderingManager, name.toStdString().c_str()); // Initialize mitkRenderWindowBase

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

  mitk::MousePressEvent::Pointer mPressEvent = mitk::MousePressEvent::New(m_Renderer, GetMousePosition(me), GetButtonState(me),
      GetModifiers(me), GetEventButton(me));

  if (!m_Renderer->GetDispatcher()->ProcessEvent(mPressEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    this->mousePressMitkEvent(&myevent);
  }
  QVTKWidget::mousePressEvent(me);

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me)
{
  mitk::MouseReleaseEvent::Pointer mReleaseEvent = mitk::MouseReleaseEvent::New(m_Renderer, GetMousePosition(me), GetButtonState(me),
      GetModifiers(me), GetEventButton(me));

  if (!m_Renderer->GetDispatcher()->ProcessEvent(mReleaseEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    this->mouseReleaseMitkEvent(&myevent);
  }
  QVTKWidget::mouseReleaseEvent(me);

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me)
{
  this->AdjustRenderWindowMenuVisibility(me->pos());

  mitk::MouseMoveEvent::Pointer mMoveEvent = mitk::MouseMoveEvent::New(m_Renderer, GetMousePosition(me), GetButtonState(me),
      GetModifiers(me));

  if (!m_Renderer->GetDispatcher()->ProcessEvent(mMoveEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
    this->mouseReleaseMitkEvent(&myevent);
  }

  mitk::MouseEvent myevent(QmitkEventAdapter::AdaptMouseEvent(m_Renderer, me));
  this->mouseMoveMitkEvent(&myevent);

  QVTKWidget::mouseMoveEvent(me);

}

void QmitkRenderWindow::wheelEvent(QWheelEvent *we)
{
  mitk::MouseWheelEvent::Pointer mWheelEvent = mitk::MouseWheelEvent::New(m_Renderer, GetMousePosition(we), GetButtonState(we),
      GetModifiers(we), GetDelta(we));

  if (!m_Renderer->GetDispatcher()->ProcessEvent(mWheelEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    mitk::WheelEvent myevent(QmitkEventAdapter::AdaptWheelEvent(m_Renderer, we));
    this->wheelMitkEvent(&myevent);
  }
  QVTKWidget::wheelEvent(we);

  if (m_ResendQtEvents)
    we->ignore();
}

void QmitkRenderWindow::keyPressEvent(QKeyEvent *ke)
{
  mitk::ModifierKeys modifiers = GetModifiers(ke);
  char key = GetKeyLetter(ke);

  mitk::InteractionKeyEvent::Pointer keyEvent = mitk::InteractionKeyEvent::New(m_Renderer, key, modifiers);
  if (!m_Renderer->GetDispatcher()->ProcessEvent(keyEvent.GetPointer()))
  { // TODO: INTERACTION_LEGACY
    QPoint cp = mapFromGlobal(QCursor::pos());
    mitk::KeyEvent mke(QmitkEventAdapter::AdaptKeyEvent(m_Renderer, ke, cp));
    this->keyPressMitkEvent(&mke);
    ke->accept();
  }

  QVTKWidget::keyPressEvent(ke);

  if (m_ResendQtEvents)
    ke->ignore();
}

void QmitkRenderWindow::enterEvent(QEvent *e)
{
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
  MITK_DEBUG << "QmitkRenderWindow::leaveEvent";

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
      long val = listOfDataNodes[i].toLong();
      mitk::DataNode* node = static_cast<mitk::DataNode *>((void*) val);
      vectorOfDataNodePointers.push_back(node);
    }

    emit NodesDropped(this, vectorOfDataNodePointers);
  }
}

mitk::Point2D QmitkRenderWindow::GetMousePosition(QMouseEvent* me)
{

  mitk::Point2D point;
  point[0] = me->x();
  point[1] = me->y();
  return point;
}

mitk::Point2D QmitkRenderWindow::GetMousePosition(QWheelEvent* we)
{
  mitk::Point2D point;
  point[0] = we->x();
  point[1] = we->y();
  return point;
}

mitk::MouseButtons QmitkRenderWindow::GetEventButton(QMouseEvent* me)
{
  mitk::MouseButtons eventButton;
  switch (me->button())
  {
  case Qt::LeftButton:
    eventButton = mitk::LeftMouseButton;
    break;
  case Qt::RightButton:
    eventButton = mitk::RightMouseButton;
    break;
  case Qt::MiddleButton:
    eventButton = mitk::MiddleMouseButton;
    break;
  default:
    eventButton = mitk::NoButton;
    break;
  }
  return eventButton;
}

mitk::MouseButtons QmitkRenderWindow::GetButtonState(QMouseEvent* me)
{
  mitk::MouseButtons buttonState = mitk::NoButton;

  if (me->buttons() == Qt::LeftButton)
  {
    buttonState = buttonState | mitk::RightMouseButton;
  }
  if (me->buttons() == Qt::RightButton)
  {
    buttonState = buttonState | mitk::LeftMouseButton;
  }
  if (me->buttons() == Qt::MiddleButton)
  {
    buttonState = buttonState | mitk::MiddleMouseButton;
  }

  return buttonState;
}

mitk::ModifierKeys QmitkRenderWindow::GetModifiers(QMouseEvent* me)
{
  mitk::ModifierKeys modifiers = mitk::NoKey;

  if (me->modifiers() == Qt::ALT)
  {
    modifiers = modifiers | mitk::AltKey;
  }
  if (me->modifiers() == Qt::CTRL)
  {
    modifiers = modifiers | mitk::ControlKey;
  }
  if (me->modifiers() == Qt::SHIFT)
  {
    modifiers = modifiers | mitk::ShiftKey;
  }

  return modifiers;
}

mitk::MouseButtons QmitkRenderWindow::GetButtonState(QWheelEvent* we)
{
  mitk::MouseButtons buttonState = mitk::NoButton;

  if (we->buttons() == Qt::LeftButton)
  {
    buttonState = buttonState | mitk::RightMouseButton;
  }
  if (we->buttons() == Qt::RightButton)
  {
    buttonState = buttonState | mitk::LeftMouseButton;
  }
  if (we->buttons() == Qt::MiddleButton)
  {
    buttonState = buttonState | mitk::MiddleMouseButton;
  }

  return buttonState;
}

mitk::ModifierKeys QmitkRenderWindow::GetModifiers(QWheelEvent* we)
{
  mitk::ModifierKeys modifiers = mitk::NoKey;

  if (we->modifiers() == Qt::ALT)
  {
    modifiers = modifiers | mitk::AltKey;
  }
  if (we->modifiers() == Qt::CTRL)
  {
    modifiers = modifiers | mitk::ControlKey;
  }
  if (we->modifiers() == Qt::SHIFT)
  {
    modifiers = modifiers | mitk::ShiftKey;
  }

  return modifiers;
}

mitk::ModifierKeys QmitkRenderWindow::GetModifiers(QKeyEvent* ke)
{
  mitk::ModifierKeys modifiers = mitk::NoKey;

  if (ke->modifiers() && Qt::ShiftModifier)
    modifiers = modifiers | mitk::ShiftKey;
  if (ke->modifiers() && Qt::CTRL)
    modifiers = modifiers | mitk::ControlKey;
  if (ke->modifiers() && Qt::ALT)
    modifiers = modifiers | mitk::AltKey;

  return modifiers;
}

char QmitkRenderWindow::GetKeyLetter(QKeyEvent *ke)
{
  // Converting Qt Key Event to ascii element.
  // extract only standard letter keys, drop modifier keys
  char ckey = 0;
  int tkey = ke->key();
  if (tkey < 128)  // else only a modifier-key is pressed
    ckey = tkey;
  return ckey;
}

int QmitkRenderWindow::GetDelta(QWheelEvent* we)
{
  return we->delta();
}
