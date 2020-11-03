/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSELECTABLEGLWIDGET_H
#define QMITKSELECTABLEGLWIDGET_H

#include "mitkCameraRotationController.h"
#include "mitkCommon.h"
#include "mitkSliceNavigationController.h"
#include "mitkVtkPropRenderer.h"
#include "ui_QmitkSelectableGLWidget.h"
#include <MitkQtWidgetsExtExports.h>

#include <mitkCommon.h>

#include <QWidget>

class QmitkRenderWindow;

class MITKQTWIDGETSEXT_EXPORT QmitkSelectableGLWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkSelectableGLWidget(QWidget *parent = nullptr);
  ~QmitkSelectableGLWidget() override;
  mitk::VtkPropRenderer *GetRenderer();
  QmitkRenderWindow *GetRenderWindow() const;
  mitk::SliceNavigationController *GetSliceNavigationController() const;
  mitk::CameraRotationController *GetCameraRotationController() const;
  mitk::BaseController *GetController() const;

protected:
  void wheelEvent(QWheelEvent *e) override;
  QmitkRenderWindow *m_RenderWindow;
  mitk::VtkPropRenderer::Pointer m_Renderer;

private:
  Ui::QmitkSelectableGLWidget *m_Ui;
};

#endif
