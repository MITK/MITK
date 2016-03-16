#include "QVTKQuickItem.h"

#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QQuickWindow>
#include <QThread>
#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>

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

#include <QQuickFramebufferObject>
#include <QOpenGLFramebufferObject>

#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkObjectFactory.h>

#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include <vtkFrameBufferObject2.h>

#include "QVTKInternalOpenglRenderWindow.h"
#include "QVTKFramebufferObjectRenderer.h"

QVTKQuickItem::QVTKQuickItem(QQuickItem* parent)
: QQuickFramebufferObject(parent)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton | Qt::RightButton);
    
    m_interactor = vtkSmartPointer<QVTKInteractor>::New();
    m_interactorAdapter = new QVTKInteractorAdapter(this);
    m_connect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    
    m_win = vtkInternalOpenGLRenderWindow::New();
    m_interactor->SetRenderWindow(m_win);
    
    m_connect->Connect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
    m_connect->Connect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
    m_connect->Connect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)), NULL, 0.0, Qt::DirectConnection);
    
    connect(this, SIGNAL(textureFollowsItemSizeChanged(bool)),
            this, SLOT(onTextureFollowsItemSizeChanged(bool)));
}

QVTKQuickItem::~QVTKQuickItem()
{
    if(m_win)
    {
        m_connect->Disconnect(m_win, vtkCommand::WindowIsCurrentEvent, this, SLOT(IsCurrent(vtkObject*, unsigned long, void*, void*)));
        m_connect->Disconnect(m_win, vtkCommand::WindowIsDirectEvent, this, SLOT(IsDirect(vtkObject*, unsigned long, void*, void*)));
        m_connect->Disconnect(m_win, vtkCommand::WindowSupportsOpenGLEvent, this, SLOT(SupportsOpenGL(vtkObject*, unsigned long, void*, void*)));
        m_win->Delete();
    }
}

QSGNode* QVTKQuickItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *nodeData)
{
    if (!node) {
        node = QQuickFramebufferObject::updatePaintNode(node, nodeData);
        QSGSimpleTextureNode *n = static_cast<QSGSimpleTextureNode *>(node);
        if (n)
            n->setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
        return node;
    }
    return QQuickFramebufferObject::updatePaintNode(node, nodeData);
}

QQuickFramebufferObject::Renderer *QVTKQuickItem::createRenderer() const
{
    return new QVTKFramebufferObjectRenderer(static_cast<vtkInternalOpenGLRenderWindow*>(m_win));
}

vtkOpenGLRenderWindow* QVTKQuickItem::GetRenderWindow() const
{
    return m_win;
}

QVTKInteractor* QVTKQuickItem::GetInteractor() const
{
    return m_interactor;
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
    *ptr = 1;
}

void QVTKQuickItem::onTextureFollowsItemSizeChanged(bool follows)
{
    if (!follows)
    {
        qWarning("QVTKQuickItem: Mouse interaction is not (yet) supported when textureFollowsItemSize==false");
    }
}

void QVTKQuickItem::geometryChanged(const QRectF & newGeometry, const QRectF & oldGeometry)
{
    QQuickFramebufferObject::geometryChanged(newGeometry, oldGeometry);
    QSize oldSize(oldGeometry.width(), oldGeometry.height());
    QSize newSize(newGeometry.width(), newGeometry.height());
    QResizeEvent e(newSize, oldSize);
    if (m_interactorAdapter)
    {
        this->m_viewLock.lock();
        m_interactorAdapter->ProcessEvent(&e, m_interactor);
        this->m_viewLock.unlock();
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
    m_win->OpenGLInitContext();
    m_win->GetExtensionManager()->LoadExtension("GL_VERSION_1_4");
    m_win->GetExtensionManager()->LoadExtension("GL_VERSION_2_0");
}

bool QVTKQuickItem::prepareForRender()
{
    return true;
}

void QVTKQuickItem::cleanupAfterRender()
{
}
