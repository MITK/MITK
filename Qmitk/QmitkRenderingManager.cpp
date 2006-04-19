/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "QmitkRenderingManager.h"
#include <qtimer.h>

QmitkRenderingManager* QmitkRenderingManager::s_Instance;

class QmitkRenderingManagerInternal : public mitk::RenderingManager
{
public:
  mitkClassMacro(QmitkRenderingManagerInternal,mitk::RenderingManager);
  itkNewMacro(Self);
  
  friend class QmitkRenderingManager;

protected:
  virtual void RestartTimer()
  {
    m_QmitkRenderingManager->RestartTimer();
  };

  virtual void StopTimer()
  {
    m_QmitkRenderingManager->StopTimer();
  };

  QmitkRenderingManager* m_QmitkRenderingManager;
};



QmitkRenderingManager *
QmitkRenderingManager
::GetInstance()
{
  if ( !QmitkRenderingManager::s_Instance )
  {
    QmitkRenderingManager *rawPtr = new QmitkRenderingManager;
    QmitkRenderingManager::s_Instance = rawPtr;
  }

  return s_Instance;
}

QmitkRenderingManager::QmitkRenderingManager()
{
  QmitkRenderingManagerInternal::Pointer internalManager = QmitkRenderingManagerInternal::New();
  internalManager->Register();
  m_QmitkRenderingManagerInternal = internalManager;
  m_QmitkRenderingManagerInternal->m_QmitkRenderingManager = this;

  m_Timer = new QTimer( this );
  connect( m_Timer, SIGNAL( timeout() ), this, SLOT( QUpdateCallback() ) );
}

QmitkRenderingManager::~QmitkRenderingManager()
{
  delete m_Timer;
  m_QmitkRenderingManagerInternal->Delete();
}

void QmitkRenderingManager::RestartTimer()
{
  m_Timer->start( m_Interval, TRUE );
}

void QmitkRenderingManager::StopTimer()
{
  m_Timer->stop();
}

void QmitkRenderingManager::QUpdateCallback()
{
  RenderingManager::UpdateCallback();
}
