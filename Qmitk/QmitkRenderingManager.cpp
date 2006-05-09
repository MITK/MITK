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

QmitkRenderingManager::QmitkRenderingManager()
{
  m_QmitkRenderingManagerInternal = new QmitkRenderingManagerInternal;
  m_QmitkRenderingManagerInternal->m_QmitkRenderingManager = this;
}

QmitkRenderingManager::~QmitkRenderingManager()
{
  delete m_QmitkRenderingManagerInternal;
}

void QmitkRenderingManager::RestartTimer()
{
  m_QmitkRenderingManagerInternal->RestartTimer();
}

void QmitkRenderingManager::StopTimer()
{
  m_QmitkRenderingManagerInternal->StopTimer();
}




QmitkRenderingManagerInternal::QmitkRenderingManagerInternal()
{
  m_Timer = new QTimer( this );
  connect( m_Timer, SIGNAL( timeout() ), this, SLOT( QUpdateCallback() ) );
}

QmitkRenderingManagerInternal::~QmitkRenderingManagerInternal()
{
  delete m_Timer;
  m_QmitkRenderingManager = NULL;
}

void QmitkRenderingManagerInternal::RestartTimer()
{
  m_Timer->start( m_QmitkRenderingManager->GetMinimumInterval(), TRUE );
}

void QmitkRenderingManagerInternal::StopTimer()
{
  m_Timer->stop();
}

void QmitkRenderingManagerInternal::QUpdateCallback()
{
  m_QmitkRenderingManager->UpdateCallback();
}

