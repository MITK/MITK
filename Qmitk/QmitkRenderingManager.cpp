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

#include "mitkDataTree.h"
#include "mitkGeometry3D.h"
#include "mitkBaseRenderer.h"
#include "mitkSliceNavigationController.h"

#include <qapplication.h>


QmitkRenderingManager
::QmitkRenderingManager()
{
  qApp->installEventFilter( QmitkAbortEventFilter::GetInstance() );
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
}

void
QmitkRenderingManager
::GenerateRenderingRequestEvent()
{
  QmitkRenderingRequestEvent *event = new QmitkRenderingRequestEvent;
  QApplication::postEvent( this, event );
}


bool 
QmitkRenderingManager
::event( QEvent *event ) 
{
  std::cout << "$";
  if ( event->type() == QmitkRenderingRequestEvent::RenderingRequest )
  //QmitkRenderingRequestEvent *requestEvent = 
  //  dynamic_cast< QmitkRenderingRequestEvent * >( event );
  //if ( requestEvent != NULL )
  {
    // Directly process all pending rendering requests
    std::cout << "%";
    this->UpdateCallback();

    return true;
  }

  return false;
}
