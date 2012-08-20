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

#include "mitkLiveWireTool2D.h"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkLiveWireTool2D.xpm"

#include <mitkInteractionConst.h>
#include <mitkContourModelInteractor.h>
#include <mitkGlobalInteraction.h>


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, LiveWireTool2D, "LiveWire tool");
}

mitk::LiveWireTool2D::LiveWireTool2D()
:SegTool2D("LiveWireTool") 
{
  m_Contour = mitk::ContourModel::New();
  m_ContourModelNode = mitk::DataNode::New();
  m_ContourModelNode->SetData( m_Contour );
  m_ContourModelNode->SetProperty("name", StringProperty::New("contour node"));
  m_ContourModelNode->SetProperty("visible", BoolProperty::New(true));
  m_ContourModelNode->AddProperty( "color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );
  m_ContourModelNode->AddProperty( "selectedcolor", ColorProperty::New(0.5, 0.5, 0.1), NULL, true );

 
  
  // great magic numbers
  CONNECT_ACTION( AcINITNEWOBJECT, OnInitLiveWire );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcMOVE, OnMouseMoved );
  CONNECT_ACTION( AcCHECKPOINT, OnCheckPoint );
  CONNECT_ACTION( AcFINISH, OnFinish );
  CONNECT_ACTION( AcCLOSECONTOUR, OnCloseContour );
}


mitk::LiveWireTool2D::~LiveWireTool2D()
{
}


const char** mitk::LiveWireTool2D::GetXPM() const
{
  return mitkLiveWireTool2D_xpm;
}


const char* mitk::LiveWireTool2D::GetName() const
{
  return "LiveWire";
}


void mitk::LiveWireTool2D::Activated()
{
  Superclass::Activated();
}


void mitk::LiveWireTool2D::Deactivated()
{
  Superclass::Deactivated();
}


bool mitk::LiveWireTool2D::OnInitLiveWire (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  m_ToolManager->GetDataStorage()->Add( m_ContourModelNode );
  
  m_Contour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()), true );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::LiveWireTool2D::OnAddPoint (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  
  m_Contour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()), true );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::LiveWireTool2D::OnMouseMoved( Action* action, const StateEvent* stateEvent)
{
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  m_Contour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()) );

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}


bool mitk::LiveWireTool2D::OnCheckPoint( Action* action, const StateEvent* stateEvent)
{
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  mitk::StateEvent* newStateEvent = NULL;

  /* 
  * Check distance to first point.
  * Transition YES if click close to first control point 
  */
  mitk::Point3D click = positionEvent->GetWorldPosition();

  mitk::Point3D first = this->m_Contour->GetVertexAt(0)->Coordinates;


  if (first.EuclideanDistanceTo(click) < 1.5)
  {
    newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
  }else
  {
    newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
  }

  this->HandleEvent( newStateEvent );

  return true;
}


bool mitk::LiveWireTool2D::OnFinish( Action* action, const StateEvent* stateEvent)
{
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  
  if ( CanHandleEvent(stateEvent) < 1.0 ) return false;


  m_ContourModelNode->AddProperty( "color", ColorProperty::New(0.9, 1.0, 0.1), NULL, true );
  m_ContourModelNode->AddProperty( "selectedcolor", ColorProperty::New(1.0, 0.0, 0.1), NULL, true );

  mitk::ContourModelInteractor::Pointer interactor = mitk::ContourModelInteractor::New(m_ContourModelNode);
  //set the livewire interactor to edit control points
  m_ContourModelNode->SetInteractor(interactor);

  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);

  m_Contour = mitk::ContourModel::New();

  m_ContourModelNode = mitk::DataNode::New();
  m_ContourModelNode->SetData( m_Contour );
  m_ContourModelNode->SetProperty("name", StringProperty::New("anoter contour node"));
  m_ContourModelNode->SetProperty("visible", BoolProperty::New(true));
  m_ContourModelNode->AddProperty( "color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );

  this->Deactivated();

  return true;
}


bool mitk::LiveWireTool2D::OnCloseContour( Action* action, const StateEvent* stateEvent)
{
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  
  if ( CanHandleEvent(stateEvent) < 1.0 ) return false;

  m_Contour->Close();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}
    
