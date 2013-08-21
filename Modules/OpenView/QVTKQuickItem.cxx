/*========================================================================
  OpenView -- http://openview.kitware.com

  Copyright 2012 Kitware, Inc.

  Licensed under the BSD license. See LICENSE file for details.
 ========================================================================*/
#include "QVTKQuickItem.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QQuickWindow>
#include <QThread>
#include <QSGSimpleRectNode>

#include "QVTKInteractor.h"
#include "QVTKInteractorAdapter.h"
#include "vtkGenericOpenGLRenderWindow.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkgl.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkRenderer.h"
#include "vtkRendererCollection.h"

#include "vtkCubeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"


#include <iostream>

QVTKQuickItem::QVTKQuickItem(QQuickItem* parent)
:QQuickItem(parent)
,m_InitCalledOnce(false)
{
  setFlag(ItemHasContents);
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);

  m_interactor = vtkSmartPointer<QVTKInteractor>::New();
  m_interactorAdapter = new QVTKInteractorAdapter(NULL);
  m_interactorAdapter->moveToThread(this->thread());
  m_interactorAdapter->setParent(this);
  m_connect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  //m_connect->Connect(m_interactor, vtkCommand::RenderEvent, this, SLOT(paint()));
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> win = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  this->SetRenderWindow(win);
  this->geometryChanged(QRectF(x(), y(), width(), height()), QRectF(0, 0, 100, 100));
}

QVTKQuickItem::~QVTKQuickItem()
{
  this->SetRenderWindow(0);
}

void QVTKQuickItem::SetRenderWindow(vtkGenericOpenGLRenderWindow* win)
{
  if(m_win)
    {
    m_win->SetMapped(0);
    //m_connect->Disconnect(m_win, vtkCommand::StartEvent, this, SLOT(Start()));
    //m_connect->Disconnect(m_win, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    //m_connect->Disconnect(m_win, vtkCommand::EndEvent, this, SLOT(End()));
    //m_connect->Disconnect(m_win, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
    m_connect->Disconnect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
    m_connect->Disconnect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
    m_connect->Disconnect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
    }

  m_interactor->SetRenderWindow(win);
  m_win = win;
  m_interactor->Initialize();

  if(m_win)
    {
    m_win->SetMapped(1);
    m_win->SetDoubleBuffer(0);
    m_win->SetFrontBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    m_win->SetFrontLeftBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    m_win->SetBackBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);
    m_win->SetBackLeftBuffer(vtkgl::COLOR_ATTACHMENT0_EXT);

    //m_connect->Connect(m_win, vtkCommand::StartEvent, this, SLOT(Start()));
    //m_connect->Connect(m_win, vtkCommand::WindowMakeCurrentEvent, this, SLOT(MakeCurrent()));
    //m_connect->Connect(m_win, vtkCommand::EndEvent, this, SLOT(End()));
    //m_connect->Connect(m_win, vtkCommand::WindowFrameEvent, this, SLOT(Update()));
    // Qt::DirectConnection in order to execute callback immediately.
    // This avoids an error when vtkTexture attempts to query driver features and it is unable to determine "IsCurrent"
    m_connect->Connect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
    m_connect->Connect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
    m_connect->Connect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
    }
}

vtkGenericOpenGLRenderWindow* QVTKQuickItem::GetRenderWindow() const
{
  return m_win;
}

QVTKInteractor* QVTKQuickItem::GetInteractor() const
{
  return m_interactor;
}

void QVTKQuickItem::itemChange(ItemChange change, const ItemChangeData &)
{
  // The ItemSceneChange event is sent when we are first attached to a canvas.
  if (change == ItemSceneChange) {
    QQuickWindow *c = window();
    if (!c)
      {
      return;
      }

    // Connect our the beforeRendering signal to our paint function.
    // Since this call is executed on the rendering thread it must be
    // a Qt::DirectConnection
    connect(c, SIGNAL(beforeRendering()), this, SLOT(paint()), Qt::DirectConnection);

    // If we allow QML to do the clearing, they would clear what we paint
    // and nothing would show.
    c->setClearBeforeRendering(false);
  }
}

void QVTKQuickItem::MakeCurrent()
{
  if (!this->window())
    {
    m_win->SetAbortRender(1);
    cerr << "Could not make current since there is no canvas!" << endl;
    return;
    }
  if (QThread::currentThread() != this->window()->openglContext()->thread())
    {
    m_win->SetAbortRender(1);
    cerr << "Could not make current since we are on the wrong thread!" << endl;
    return;
    }
  this->window()->openglContext()->makeCurrent(this->window());
}

void QVTKQuickItem::Start()
{
  MakeCurrent();

  if (!m_win->GetAbortRender())
    {
    m_win->PushState();
    m_win->OpenGLInitState();
    }
}

void QVTKQuickItem::End()
{
  if (!m_win->GetAbortRender())
    {
    m_win->PopState();
    }

}

void QVTKQuickItem::IsCurrent(vtkObject*, unsigned long, void*, void* call_data)
{
  bool* ptr = reinterpret_cast<bool*>(call_data);
  *ptr = QOpenGLContext::currentContext() == this->window()->openglContext();
}

void QVTKQuickItem::IsDirect(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = 1;
}

void QVTKQuickItem::SupportsOpenGL(vtkObject*, unsigned long, void*, void* call_data)
{
  int* ptr = reinterpret_cast<int*>(call_data);
  *ptr = true;
  //*ptr = QGLFormat::hasOpenGL();
}

void QVTKQuickItem::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
  QQuickItem::geometryChanged(newGeometry, oldGeometry);
  QSize oldSize(oldGeometry.width(), oldGeometry.height());
  QSize newSize(newGeometry.width(), newGeometry.height());
  QResizeEvent e(newSize, oldSize);
  if (m_interactorAdapter)
    {
    this->m_viewLock.lock();
    m_interactorAdapter->ProcessEvent(&e, m_interactor);
    this->m_viewLock.unlock();
    }
  if(m_win.GetPointer() && window())
    {
    this->m_viewLock.lock();
    m_win->SetSize(window()->width(), window()->height());
    QPointF origin = mapToScene(QPointF(0, 0));
    QPointF minPt(origin.x()/window()->width(), (window()->height() - origin.y() - height())/window()->height());
    QPointF maxPt(minPt.x() + width()/window()->width(), minPt.y() + height()/window()->height());
    if (m_win->GetRenderers()->GetFirstRenderer())
      {
      m_win->GetRenderers()->GetFirstRenderer()->SetViewport(minPt.x(), minPt.y(), maxPt.x(), maxPt.y());
      }
    this->m_viewLock.unlock();
    update();
    }
}

void QVTKQuickItem::keyPressEvent(QKeyEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::keyReleaseEvent(QKeyEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mousePressEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mouseReleaseEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mouseDoubleClickEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::mouseMoveEvent(QMouseEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::wheelEvent(QWheelEvent* e)
{
  e->accept();
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(e, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::hoverEnterEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Enter);
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::hoverLeaveEvent(QHoverEvent* e)
{
  e->accept();
  QEvent e2(QEvent::Leave);
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::hoverMoveEvent(QHoverEvent* e)
{
  e->accept();
  QMouseEvent e2(QEvent::MouseMove, e->pos(), Qt::NoButton, Qt::NoButton, e->modifiers());
  this->m_viewLock.lock();
  m_interactorAdapter->ProcessEvent(&e2, m_interactor);
  this->m_viewLock.unlock();
  update();
}

void QVTKQuickItem::init()
{
}

void QVTKQuickItem::prepareForRender()
{
}

void QVTKQuickItem::cleanupAfterRender()
{
}

QSGNode* QVTKQuickItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
  QSGSimpleRectNode *n = static_cast<QSGSimpleRectNode *>(oldNode);
  if (!n) {
    n = new QSGSimpleRectNode();
  }
  n->markDirty(QSGNode::DirtyForceUpdate);
  return n;
}


void QVTKQuickItem::paint()
{
  if (!this->isVisible())
    {
    return;
    }

    if (!this->m_InitCalledOnce)
    {
      m_win->GetExtensionManager()->LoadExtension("GL_VERSION_1_4");
      m_win->GetExtensionManager()->LoadExtension("GL_VERSION_2_0");

      init();

      this->m_InitCalledOnce = true;
    }

  this->m_viewLock.lock();

  // Let subclasses do something each render
  prepareForRender();

  // Make sure viewport is up to date.
  // This is needed because geometryChanged() is not called when parent geometry changes, so we miss when widths/heights
  // of surrounding elements change.
  m_win->SetSize(window()->width(), window()->height());
  QPointF origin = mapToScene(QPointF(0, 0));
  QPointF minPt(origin.x()/window()->width(), (window()->height() - origin.y() - height())/window()->height());
  QPointF maxPt(minPt.x() + width()/window()->width(), minPt.y() + height()/window()->height());
  if (m_win->GetRenderers()->GetFirstRenderer())
    {
    m_win->GetRenderers()->GetFirstRenderer()->SetViewport(minPt.x(), minPt.y(), maxPt.x(), maxPt.y());
    }

  // Turn off any QML shader program
  vtkgl::UseProgram(0);

  // Set blending correctly
  glEnable(GL_BLEND);
  vtkgl::BlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  m_win->Render();

  // Disable alpha test for QML
  glDisable(GL_ALPHA_TEST);

  cleanupAfterRender();

  this->m_viewLock.unlock();
}
