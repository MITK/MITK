/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "mitkBaseRenderer.h"
#include <iostream>

void QmitkStandardViews::init()
{
  m_CameraController = NULL;  
}

void QmitkStandardViews::SetCameraController( mitk::CameraController * controller )
{
  m_CameraController = controller;
}

void QmitkStandardViews::SetCameraControllerFromRenderWindow( mitk::RenderWindow * window )
{
  if ( window != NULL )
  {
    if ( window->GetRenderer() != NULL )
      if ( window->GetRenderer()->GetCameraController() != NULL )
        m_CameraController = window->GetRenderer()->GetCameraController();
  }
  else
  {
    std::cout << "Warning in "<<__FILE__<<", "<<__LINE__<<": render window is NULL!" << std::endl;  
  }  
}

void QmitkStandardViews::OnStandardViewDefined( int id )
{
  mitk::CameraController::StandardView view;
  if ( m_StandardViews->find( id ) == m_Anterior )
    view = mitk::CameraController::ANTERIOR;
  else if ( m_StandardViews->find( id ) == m_Posterior )
    view = mitk::CameraController::POSTERIOR;
  else if ( m_StandardViews->find( id ) == m_Sinister )
    view = mitk::CameraController::SINISTER;
  else if ( m_StandardViews->find( id ) == m_Dexter )
    view = mitk::CameraController::DEXTER;
  else if ( m_StandardViews->find( id ) == m_Cranial )
    view = mitk::CameraController::CRANIAL;
  else if ( m_StandardViews->find( id ) == m_Caudal )
    view = mitk::CameraController::CAUDAL;
  else
  {
    std::cout << "Warning in "<<__FILE__<<", "<<__LINE__<<": unknown standard view ("<<id<<")!" << std::endl;  
    view = mitk::CameraController::ANTERIOR;
  }
  if ( m_CameraController.IsNotNull() )
  {
    m_CameraController->SetStandardView( view );
  }
  emit StandardViewDefined( view );
}
