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

QmitkRenderingManager* QmitkRenderingManager::m_Instance;

QmitkRenderingManager *
QmitkRenderingManager
::GetInstance()
{
  if ( !QmitkRenderingManager::m_Instance )
  {
    QmitkRenderingManager *rawPtr = new QmitkRenderingManager;
    QmitkRenderingManager::m_Instance = rawPtr;
  }

  return m_Instance;
}

QmitkRenderingManager::QmitkRenderingManager()
{
  m_Timer = new QTimer( this );
  connect( m_Timer, SIGNAL( timeout() ), this, SLOT( QUpdateCallback() ) );
}

QmitkRenderingManager::~QmitkRenderingManager()
{
  delete m_Timer;
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
