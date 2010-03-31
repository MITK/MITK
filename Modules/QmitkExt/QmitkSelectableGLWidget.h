/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
