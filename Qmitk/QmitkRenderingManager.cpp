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

#include "mitkDataTree.h"
#include "mitkGeometry3D.h"
#include "mitkBaseRenderer.h"
#include "mitkSliceNavigationController.h"

#include <qapplication.h>
#include <qtimer.h>


QmitkRenderingManager
::QmitkRenderingManager()
{
  qApp->installEventFilter( QmitkAbortEventFilter::GetInstance() );
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


bool
QmitkRenderingManager
::InitializeViews( mitk::DataTreeIteratorBase *dataIt, unsigned int requestType )
{
  mitk::Geometry3D::Pointer geometry;
  if ( dataIt != NULL )
  {
    geometry = mitk::DataTree::ComputeVisibleBoundingGeometry3D(
      dataIt, NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      // let's see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = geometry->GetTimeBounds();
      if ( timebounds[1] < mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry = 
          mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed( 
          geometry, (unsigned int) duration );
        timegeometry->SetTimeBounds( timebounds );

        timebounds[1] = timebounds[0] + 1.0;
        geometry->SetTimeBounds( timebounds );

        geometry = timegeometry;
      }
    }
  }

  // Use geometry for initialization
  return this->InitializeViews( geometry.GetPointer(), requestType );
}

bool
QmitkRenderingManager
::InitializeViews( const mitk::Geometry3D *geometry, unsigned int requestType ) 
{
  bool boundingBoxInitialized = false;

  int warningLevel = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  if ( (geometry != NULL ) && (const_cast< mitk::BoundingBox * >(
    geometry->GetBoundingBox())->GetDiagonalLength2() > mitk::eps) )
  {
    boundingBoxInitialized = true;
  }

  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int id = mitk::BaseRenderer::GetInstance( it->first )->GetMapperID();
    if ( (requestType == REQUEST_UPDATE_ALL)
      || ((requestType == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((requestType == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      mitk::SliceNavigationController *snc = 
        mitk::BaseRenderer::GetInstance( it->first )
          ->GetSliceNavigationController();

      // Re-initialize view direction
      snc->SetViewDirectionToDefault();

      if ( boundingBoxInitialized )
      {
        // Set geometry for SNC
        snc->SetInputWorldGeometry( geometry );
        snc->Update();

        if ( id == 1 )
        {
          // For 2D SNCs, steppers are set so that the cross is centered 
          // in the image
          snc->GetSlice()->SetPos( snc->GetSlice()->GetSteps() / 2 );
        }

        // Fit the render window DisplayGeometry
        mitk::BaseRenderer::GetInstance( it->first )
          ->GetDisplayGeometry()->Fit();

        vtkRenderer *renderer = 
          mitk::BaseRenderer::GetInstance( it->first )->GetVtkRenderer();
        
        if ( renderer != NULL ) renderer->ResetCamera();
      }
      else
      {
        snc->Update();
      }
    }
  }

  if ( m_TimeNavigationController != NULL )
  {
    if ( boundingBoxInitialized )
    {
      m_TimeNavigationController->SetInputWorldGeometry( geometry );
    }
    m_TimeNavigationController->Update();
  }

  this->RequestUpdateAll( requestType );

  vtkObject::SetGlobalWarningDisplay( warningLevel );

  // Inform listeners that views have been initialized
  this->InvokeEvent( mitk::RenderingManagerViewsInitializedEvent() );


  return boundingBoxInitialized;
}


bool 
QmitkRenderingManager
::InitializeViews( unsigned int requestType )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int id = mitk::BaseRenderer::GetInstance( it->first )->GetMapperID();
    if ( (requestType == REQUEST_UPDATE_ALL)
      || ((requestType == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((requestType == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      mitk::SliceNavigationController *snc = 
        mitk::BaseRenderer::GetInstance( it->first )
          ->GetSliceNavigationController();

      // Re-initialize view direction
      snc->SetViewDirectionToDefault();

      // Update the SNC
      snc->Update();
    }
  }

  this->RequestUpdateAll( requestType );

  return true;
}

 

QmitkRenderingManagerInternal
::QmitkRenderingManagerInternal()
{
  m_Timer = new QTimer( this );
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
  m_Timer->start( 5, TRUE );
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

