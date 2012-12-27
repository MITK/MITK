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

#ifndef QMITKSELECTABLEGLWIDGET_H
#define QMITKSELECTABLEGLWIDGET_H

#include "ui_QmitkSelectableGLWidget.h"
#include "mitkCameraRotationController.h"
#include "mitkCommon.h"
#include "mitkSliceNavigationController.h"
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
  mitk::SliceNavigationController*  GetSliceNavigationController() const;
  mitk::CameraRotationController* GetCameraRotationController() const;
  mitk::BaseController* GetController() const;
  protected:
  void wheelEvent( QWheelEvent * e );
    QmitkRenderWindow *m_RenderWindow;
    mitk::VtkPropRenderer::Pointer m_Renderer;
};

#endif
