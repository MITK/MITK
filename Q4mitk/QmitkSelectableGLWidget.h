#ifndef QMITKSELECTABLEGLWIDGET_H
#define QMITKSELECTABLEGLWIDGET_H

#include "ui_QmitkSelectableGLWidget.h"
#include "mitkCameraRotationController.h"
#include "mitkCommon.h"
#include "mitkNavigationController.h"
#include "mitkVtkPropRenderer.h"

#include <mitkCommon.h>

#include <QWidget>

class QmitkRenderWindow;

class QmitkSelectableGLWidget : public QWidget, public Ui::QmitkSelectableGLWidget
{
  public:
	QmitkSelectableGLWidget(QWidget* parent=0);
	mitk::VtkPropRenderer* GetRenderer();
	QmitkRenderWindow* GetRenderWindow() const;
	mitk::NavigationController*  GetNavigationController() const;
	mitk::CameraRotationController* GetCameraRotationController() const;
	mitk::BaseController* GetController() const;
  protected:
	void wheelEvent( QWheelEvent * e );
    QmitkRenderWindow *m_RenderWindow;
    mitk::VtkPropRenderer::Pointer m_Renderer;
};

#endif
