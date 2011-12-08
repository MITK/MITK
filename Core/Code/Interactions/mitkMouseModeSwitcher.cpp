/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-14 14:20:26 +0100 (Thu, 14 Jan 2010) $
Version:   $Revision: 21047 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkMouseModeSwitcher.h"

#include "mitkDisplayInteractor.h"
#include "mitkDisplayVectorInteractor.h"
#include "mitkDisplayVectorInteractorLevelWindow.h"
#include "mitkDisplayVectorInteractorScroll.h"



mitk::MouseModeSwitcher::MouseModeSwitcher( mitk::GlobalInteraction* gi ) 
: m_GlobalInteraction( gi )
, m_ActiveInteractionScheme( MITK )
, m_ActiveMouseMode( Pointer )
, m_ActiveListener( NULL )
{

  this->InitializeListeners();
  this->SetInteractionScheme( m_ActiveInteractionScheme );
}


mitk::MouseModeSwitcher::~MouseModeSwitcher()
{
}


void mitk::MouseModeSwitcher::InitializeListeners()
{
  mitk::DisplayVectorInteractor::Pointer moveAndZoomInteractor = mitk::DisplayVectorInteractor::New(
    "moveNzoom", new mitk::DisplayInteractor() );
  mitk::StateMachine::Pointer listener = moveAndZoomInteractor;
  m_ListenersForMITK.push_back( listener );


  mitk::DisplayVectorInteractorScroll::Pointer scrollInteractor = mitk::DisplayVectorInteractorScroll::New( 
    "MiddleClickScroll", new mitk::DisplayInteractor() );
  listener = scrollInteractor;
  m_ListenersForPACS.push_back( listener );

  mitk::DisplayVectorInteractorLevelWindow::Pointer lwInteractor = mitk::DisplayVectorInteractorLevelWindow::New("RightClickLevelWindow");
  listener = lwInteractor;
  m_ListenersForPACS.push_back( listener );

  mitk::DisplayVectorInteractor::Pointer panInteractor = mitk::DisplayVectorInteractor::New( 
    "ShiftClickPan", new mitk::DisplayInteractor() );
  listener = panInteractor;
  m_ListenersForPACS.push_back( listener );

  mitk::DisplayVectorInteractor::Pointer crtlZoomInteractor = mitk::DisplayVectorInteractor::New( 
    "CtrlZoom", new mitk::DisplayInteractor() );
  listener = crtlZoomInteractor;
  m_ListenersForPACS.push_back( listener );


}

void mitk::MouseModeSwitcher::SetInteractionScheme( InteractionScheme scheme )
{

  switch ( scheme )
  {
  case MITK :
    {
      ListenerList::iterator iter;
      for ( iter=m_ListenersForPACS.begin(); iter!=m_ListenersForPACS.end(); iter++ )
      {
        m_GlobalInteraction->RemoveListener( (*iter) );
      }

      for ( iter=m_ListenersForMITK.begin(); iter!=m_ListenersForMITK.end(); iter++ )
      {
        m_GlobalInteraction->AddListener( (*iter) );
      }
      break;
    } // case MITK
  case PACS :
    {
      ListenerList::iterator iter;
      for ( iter=m_ListenersForMITK.begin(); iter!=m_ListenersForMITK.end(); iter++ )
      {
        m_GlobalInteraction->RemoveListener( (*iter) );
      }

      for ( iter=m_ListenersForPACS.begin(); iter!=m_ListenersForPACS.end(); iter++ )
      {
        m_GlobalInteraction->AddListener( (*iter) );
      }

      this->SelectMouseMode( Pointer );
      break;
    } // case PACS
  } // switch


  m_ActiveInteractionScheme = scheme;
}

void mitk::MouseModeSwitcher::SelectMouseMode( MouseMode mode )
{
  if ( m_ActiveInteractionScheme != PACS )
    return;

  switch ( mode )
  {
  case Pointer :
    {
      m_GlobalInteraction->RemoveListener( m_ActiveListener );
      break;
    } // case 0
  case Scroll :
    {
      m_GlobalInteraction->RemoveListener( m_ActiveListener );

      mitk::DisplayVectorInteractorScroll::Pointer scrollInteractor = mitk::DisplayVectorInteractorScroll::New( 
        "LeftClickScroll", new mitk::DisplayInteractor() );
      m_ActiveListener = scrollInteractor;

      m_GlobalInteraction->AddListener( m_ActiveListener );

      break;
    } // case 1
  case LevelWindow :
    {
      m_GlobalInteraction->RemoveListener( m_ActiveListener );

      mitk::DisplayVectorInteractorLevelWindow::Pointer lwInteractor = mitk::DisplayVectorInteractorLevelWindow::New( 
        "LeftClickLevelWindow" );
      m_ActiveListener = lwInteractor;

      m_GlobalInteraction->AddListener( m_ActiveListener );

      break;
    } // case 2
  case Zoom :
    {
      m_GlobalInteraction->RemoveListener( m_ActiveListener );

      mitk::DisplayVectorInteractor::Pointer zoomInteractor = mitk::DisplayVectorInteractor::New( 
        "Zoom", new mitk::DisplayInteractor() );
      m_ActiveListener = zoomInteractor;

      m_GlobalInteraction->AddListener( m_ActiveListener );

      break;
    } // case 3
  case Pan :
    {
      m_GlobalInteraction->RemoveListener( m_ActiveListener );

      mitk::DisplayVectorInteractor::Pointer panInteractor = mitk::DisplayVectorInteractor::New( 
        "Pan", new mitk::DisplayInteractor() );
      m_ActiveListener = panInteractor;

      m_GlobalInteraction->AddListener( m_ActiveListener );

      break;
    } // case 4

  } // switch (mode)

  this->InvokeEvent( MouseModeChangedEvent() );

}


mitk::MouseModeSwitcher::MouseMode mitk::MouseModeSwitcher::GetCurrentMouseMode() const
{
  return m_ActiveMouseMode;
}


