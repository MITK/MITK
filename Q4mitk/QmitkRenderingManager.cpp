/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkAbortEventFilter.h"
#include "QmitkRenderingManager.h"
#include "QmitkAbortEventFilter.h"
#include <QApplication>
#include <QTimer>

#include "mitkDataTree.h"
#include "mitkGeometry3D.h"
#include "mitkBaseRenderer.h"
#include "mitkSliceNavigationController.h"


QmitkRenderingManager
::QmitkRenderingManager()
{
  QApplication::instance()->installEventFilter( QmitkAbortEventFilter::GetInstance() );
  m_QmitkRenderingManagerInternal = new QmitkRenderingManagerInternal;
  m_QmitkRenderingManagerInternal->m_QmitkRenderingManager = this;
}


void
QmitkRenderingManager
::DoMonitorRendering()
{
  QmitkAbortEventFilter::GetInstance()->ProcessEvents();
}


void
QmitkRenderingManager
::DoFinishAbortRendering()
{
  QmitkAbortEventFilter::GetInstance()->IssueQueuedEvents();
}


QmitkRenderingManager
::~QmitkRenderingManager()
{
  delete m_QmitkRenderingManagerInternal;
}


void
QmitkRenderingManager
::RestartTimer()
{
  m_QmitkRenderingManagerInternal->RestartTimer();
}


void
QmitkRenderingManager
::StopTimer()
{
  m_QmitkRenderingManagerInternal->StopTimer();
}


QmitkRenderingManagerInternal
::QmitkRenderingManagerInternal()
{
  m_Timer = new QTimer( 0 );
  m_Timer->setSingleShot(true);
  connect( m_Timer, SIGNAL( timeout() ), this, SLOT( QUpdateCallback() ) );
}


QmitkRenderingManagerInternal
::~QmitkRenderingManagerInternal()
{
  delete m_Timer;
  m_QmitkRenderingManager = NULL;
}


void
QmitkRenderingManagerInternal
::RestartTimer()
{
  m_Timer->start(5);
  //QTimer::singleShot(m_QmitkRenderingManager->GetMinimumInterval(), this, SLOT(QUpdateCallback()));
}


void
QmitkRenderingManagerInternal
::StopTimer()
{
  m_Timer->stop();
}


void
QmitkRenderingManagerInternal
::QUpdateCallback()
{
  //std::cout << "QUpdateCallback()" << std::endl;
  m_QmitkRenderingManager->UpdateCallback();
}

