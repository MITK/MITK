#include "QmitkRenderWindow.h"
#include "mitkOpenGLRenderer.h"
#include "mitkRenderWindow.h"
#include <qcursor.h>
#include "mitkDisplayPositionEvent.h"
//##ModelId=3E1EB4410304
mitk::QmitkRenderWindow::QmitkRenderWindow(mitk::BaseRenderer* renderer, QGLFormat glf, QWidget *parent, const char *name) 
  : QGLWidget(glf, parent, name), mitk::RenderWindow(name), m_Renderer(renderer)
{
  InitRenderer();
  setFocusPolicy(QWidget::StrongFocus);
  setMouseTracking(true);
}

//##ModelId=3E1EB4410318
mitk::QmitkRenderWindow::QmitkRenderWindow(mitk::BaseRenderer* renderer, QWidget *parent, const char *name)
  : QGLWidget(parent, name), mitk::RenderWindow(name), m_Renderer(renderer)
{
  InitRenderer();
  setFocusPolicy(QWidget::StrongFocus);
  setMouseTracking(true);
}

//##ModelId=3E1EB441032C
mitk::QmitkRenderWindow::~QmitkRenderWindow()
{

}

void mitk::QmitkRenderWindow::InitRenderer()
{
  m_InitNeeded = true;
  m_ResizeNeeded = true;

  if(m_Renderer.IsNull())
    m_Renderer = mitk::OpenGLRenderer::New();

  m_Renderer->InitRenderer(this);

  this->setAutoBufferSwap( false );
}

/*!
\brief Initialize the OpenGL Window
*/
//##ModelId=3E33145903C8
void mitk::QmitkRenderWindow::initializeGL() 
{
  m_Renderer->Initialize();
}

/*!
\brief Resize the OpenGL Window
*/
//##ModelId=3E33145A001C
void mitk::QmitkRenderWindow::resizeGL( int w, int h ) 
{
  if(QGLWidget::isVisible())
  {
    m_Renderer->Resize(w, h);
    updateGL();
  }
}

/*!
\brief Render the scene
*/
//##ModelId=3E3314590396
void mitk::QmitkRenderWindow::paintGL( )
{
  // Get the native window ID and pass it
  // to the m_Renderer
  // before we render for the first time...
  if(m_InitNeeded)
  {
    WId nId = winId();
    m_Renderer->SetWindowId( (void*) nId );
    m_InitNeeded = false;
  }
  else
    // rendering when not visible might cause serious problems (program may hang)
    if(QGLWidget::isVisible())
    {
      m_Renderer->Paint();
    }
}

void mitk::QmitkRenderWindow::showEvent ( QShowEvent * )
{
  // when the widget becomes visible the first time, we need to tell vtk the window size
  if(m_ResizeNeeded)
  {
    m_ResizeNeeded=false;
    m_Renderer->InitSize(width(),height());
    //m_Renderer->SetSize(width(),height());
    //    resizeGL(width(),height());
  }
}


//##ModelId=3E3D1D4A00A5
void mitk::QmitkRenderWindow::MakeCurrent()
{
  makeCurrent();
}

void mitk::QmitkRenderWindow::SwapBuffers() 
{
  swapBuffers();
};

bool mitk::QmitkRenderWindow::IsSharing () const
{
  return isSharing();
}

//##ModelId=3E6D5DD40306
void mitk::QmitkRenderWindow::mousePressEvent(QMouseEvent *me) 
{
  QGLWidget::mousePressEvent(me);
  if (m_Renderer)
  {
    Point2D p(me->x(), me->y());
    mitk::MouseEvent event(NULL, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MousePressEvent(&event);
  }
}

//##ModelId=3E6D5DD4032E
void mitk::QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me) 
{
  QGLWidget::mouseReleaseEvent(me);
  if (m_Renderer) {
    Point2D p(me->x(), me->y());
    mitk::MouseEvent event(NULL, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MouseReleaseEvent(&event);
  }
}

//##ModelId=3E6D5DD40356
void mitk::QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me) 
{
  QGLWidget::mouseMoveEvent(me);
  if (m_Renderer) {
    Point2D p(me->x(), me->y());
    mitk::MouseEvent event(NULL, me->type(), me->button(), me->state(), Qt::Key_unknown, p);
    m_Renderer->MouseMoveEvent(&event);
}
}

void mitk::QmitkRenderWindow::wheelEvent(QWheelEvent *we)
{
/*  QGLWidget::wheelEvent(we);
  if (m_Renderer)
    m_Renderer->WheelEvent(we); */
}

//##ModelId=3E6D5DD40388
void mitk::QmitkRenderWindow::keyPressEvent(QKeyEvent *ke) 
{
  if (m_Renderer)
  {
    QPoint cp = mapFromGlobal(QCursor::pos());
    mitk::KeyEvent mke(ke->type(), ke->key(), ke->ascii(), ke->state(), ke->text(), ke->isAutoRepeat(), ke->count(), cp.x(), cp.y(), QCursor::pos().x(), QCursor::pos().y()); 
    m_Renderer->KeyPressEvent(&mke);
    if(mke.isAccepted())
      ke->accept();
  }
}

//##ModelId=3EF59AD202B7
void mitk::QmitkRenderWindow::Update()
{
  update();
};

void mitk::QmitkRenderWindow::Repaint()
{
  updateGL();
};

//##ModelId=3E6D5DD403B0
void mitk::QmitkRenderWindow::focusInEvent(QFocusEvent*)  {};
//##ModelId=3E6D5DD403E2
void mitk::QmitkRenderWindow::focusOutEvent(QFocusEvent*) {}; 
