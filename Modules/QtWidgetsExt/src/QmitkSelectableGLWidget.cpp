/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSelectableGLWidget.h"
#include "QmitkRenderWindow.h"

#include <QWheelEvent>

QmitkSelectableGLWidget::QmitkSelectableGLWidget(QWidget *parent)
  : QWidget(parent), m_Ui(new Ui::QmitkSelectableGLWidget)
{
  m_Ui->setupUi(this);
  QLayout *hlayout;
  hlayout = m_Ui->hboxLayout;
  hlayout->setMargin(3);

  QString rendererName("Renderer::");
  rendererName += objectName();

  // create Renderer
  m_Renderer = mitk::VtkPropRenderer::New(qPrintable(rendererName), nullptr);

  // create widget
  QString composedName("QSGLWt::");
  if (!objectName().isEmpty())
    composedName += objectName();
  else
    composedName += "QmitkGLWidget";
  /*
  * here is the place to define QT-Flags to enable and disable certain OpenGL elements, like StencilBuffer, RGBA and so
  * on.
  * See QGLFormat for futher informations
  * QGL::AlphaChannel: Enable Alpha in Framebuffer
  * QGL::Rgba enable use of rgba rather than color_index
  * QGL::StencilBuffer for use of stencilbuffer in OpenGL
  */
  m_RenderWindow = new QmitkRenderWindow(this, composedName, m_Renderer);

  hlayout->addWidget(m_RenderWindow);
}

QmitkSelectableGLWidget::~QmitkSelectableGLWidget()
{
  delete m_Ui;
}

mitk::VtkPropRenderer *QmitkSelectableGLWidget::GetRenderer()
{
  return m_Renderer.GetPointer();
}

QmitkRenderWindow *QmitkSelectableGLWidget::GetRenderWindow() const
{
  return m_RenderWindow;
}

void QmitkSelectableGLWidget::wheelEvent(QWheelEvent *e)
{
  if (m_RenderWindow->GetSliceNavigationController()->GetSliceLocked())
    return;

  mitk::Stepper *stepper = m_RenderWindow->GetSliceNavigationController()->GetSlice();

  if (stepper->GetSteps() <= 1)
  {
    stepper = m_RenderWindow->GetSliceNavigationController()->GetTime();
  }

  if (e->orientation() * e->delta() > 0)
  {
    stepper->Next();
  }
  else
  {
    stepper->Previous();
  }
}

mitk::SliceNavigationController *QmitkSelectableGLWidget::GetSliceNavigationController() const
{
  return m_RenderWindow->GetSliceNavigationController();
}

mitk::CameraRotationController *QmitkSelectableGLWidget::GetCameraRotationController() const
{
  return m_RenderWindow->GetCameraRotationController();
}

mitk::BaseController *QmitkSelectableGLWidget::GetController() const
{
  return m_RenderWindow->GetController();
}
