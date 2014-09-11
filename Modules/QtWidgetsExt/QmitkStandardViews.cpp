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

#include "QmitkStandardViews.h"

#include "mitkBaseRenderer.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <qclickablelabel.h>

QmitkStandardViews::QmitkStandardViews( QWidget* parent, Qt::WindowFlags f )
:QWidget(parent, f)
{
  QVBoxLayout *vlayout = new QVBoxLayout(this);
  QWidget *labelContainer = new QWidget(this);
  vlayout->addStretch(1);
  vlayout->addWidget(labelContainer);
  vlayout->addStretch(1);

  QHBoxLayout *hlayout = new QHBoxLayout(labelContainer);

  // find embedded picture
  QPixmap pixmap(":QmitkStandardViews.png");

  // set picture for label
  m_ClickablePicture = new QClickableLabel(labelContainer);

  // set hotspots specific for this image
  m_ClickablePicture->setPixmap( pixmap );

  m_ClickablePicture->AddHotspot( "Left", QRect(QPoint(0,64),
                                                QPoint(21, 83)) );

  m_ClickablePicture->AddHotspot( "Right", QRect(QPoint(128, 64),
                                                 QPoint(149, 83)) );

  m_ClickablePicture->AddHotspot( "Top", QRect(QPoint(66, 0),
                                               QPoint(83, 75)) );

  m_ClickablePicture->AddHotspot( "Bottom", QRect(QPoint(66, 128),
                                                  QPoint(83, 149)) );

  m_ClickablePicture->AddHotspot( "Front", QRect(QPoint(10, 102),
                                                 QPoint(29, 119)) );

  m_ClickablePicture->AddHotspot( "Back", QRect(QPoint(119, 30),
                                                QPoint(138, 48)) );

  connect( m_ClickablePicture, SIGNAL( mouseReleased(const QString&)), this, SLOT(hotspotClicked(const QString&)) );

  hlayout->addStretch(1);
  hlayout->addWidget( m_ClickablePicture );
  hlayout->addStretch(1);
}



QmitkStandardViews::~QmitkStandardViews()
{

}



void QmitkStandardViews::SetCameraController( mitk::CameraController* controller )
{
  m_CameraController = controller;
}


void QmitkStandardViews::SetCameraControllerFromRenderWindow( vtkRenderWindow* window )
{
  if ( window != NULL )
  {
    if ( mitk::BaseRenderer::GetInstance(window) != NULL )
      if ( mitk::BaseRenderer::GetInstance(window)->GetCameraController() != NULL )
        m_CameraController = mitk::BaseRenderer::GetInstance(window)->GetCameraController();

  }
  else
  {
    std::cerr << "Warning in "<<__FILE__<<", "<<__LINE__<<": render window is NULL!" << std::endl;
  }
}

void QmitkStandardViews::hotspotClicked(const QString& s)
{
  mitk::CameraController::StandardView view;
  bool good(true);

       if ( s == "Left"    ) view = mitk::CameraController::DEXTER;
  else if ( s == "Right"   ) view = mitk::CameraController::SINISTER;
  else if ( s == "Top"     ) view = mitk::CameraController::CRANIAL;
  else if ( s == "Bottom"  ) view = mitk::CameraController::CAUDAL;
  else if ( s == "Front"   ) view = mitk::CameraController::ANTERIOR;
  else if ( s == "Back"    ) view = mitk::CameraController::POSTERIOR;
  else
  {
    std::cerr << "Warning in "<<__FILE__<<", "<<__LINE__<<": unknown standard view '" << s.toStdString() << "'" << std::endl;
    view = mitk::CameraController::ANTERIOR;
    good = false;
  }

  if (good)
  {
    if ( m_CameraController.IsNotNull() )
    {
      m_CameraController->SetStandardView( view );
    }

    emit StandardViewDefined( view );
  }
}

